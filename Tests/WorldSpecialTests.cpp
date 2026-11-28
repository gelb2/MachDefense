#include "PNGTestSize.hpp"
#include "Core/Simulation/World.hpp"
#include "NuclearDamageSweep.hpp"
#include "BaseUpgradeEffectSweep.hpp"
#include "DamageAudioSweep.hpp"
#include <cassert>
#include <algorithm>
#include <limits>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
struct Resources:md::ResourceAccess{
    std::filesystem::path root;std::unordered_map<std::string,md::Vec2> sizes;
    md::Bytes read(std::string_view path)override{std::ifstream f(root/path,std::ios::binary);if(!f)throw std::runtime_error(std::string(path));return {std::istreambuf_iterator<char>(f),{}};}
    md::Vec2 imageSize(std::string_view name)override{auto it=sizes.find(std::string(name));if(it!=sizes.end())return it->second;if(!std::filesystem::exists(root/"Images"/name))return {};auto b=read("Images/"+std::string(name));if(b.size()>24&&b[0]==137){return sizes[std::string(name)]=mdtest::pngSize(b);}return {};}
};
struct Host:md::HostServices{int writes=0,sounds=0;unsigned groundExplosions=0,airStrikeStarts=0,raidLaserShots=0;std::optional<md::Bytes>readSave()override{return{};}bool writeSave(std::span<const std::uint8_t>)override{++writes;return true;}void playSound(std::string_view name,float,float)override{++sounds;groundExplosions+=name=="SFX_ExplGround.wav";airStrikeStarts+=name=="SFX_AirStrike.wav";raidLaserShots+=name=="Plasma02.wav";}void playMusic(std::string_view,float)override{}void stopMusic()override{}void requestName(std::string,std::size_t,std::function<void(std::optional<std::string>)>)override{}};

struct RaidWave { double at; unsigned count; };

std::vector<RaidWave> raidWaves(const md::DataTable& table,unsigned stageNumber) {
    const auto row=table.findRow(std::to_string(stageNumber));
    assert(row);
    std::vector<RaidWave> waves;
    const auto& columns=table.columns();
    for (std::size_t i=2;i<columns.size();i+=2) {
        const auto seconds=table.integer(*row,columns[i]);
        const auto count=table.integer(*row,columns[i+1]);
        assert(seconds && count && *count>=1 && *count<=3);
        waves.push_back({static_cast<double>(*seconds),static_cast<unsigned>(*count)});
    }
    return waves;
}

int checkEnemyRaidDamage(Resources& resources, md::BattleGeometry geometry) {
    using namespace md;
    auto readTable = [&](std::string_view name) {
        const auto bytes = resources.read("Tables/" + std::string(name) + ".tsv");
        return DataTable::parse(std::string_view(reinterpret_cast<const char*>(bytes.data()), bytes.size()));
    };
    const auto parts = readTable("MachBuild_Parts");
    const auto items = readTable("ItemInfo");
    const auto stages = readTable("MapList").rowCount();
    const auto builds = BuildCatalog::load(parts,readTable("MachBuild_Name"));
    const auto weapons = WeaponCatalog::load(readTable("WeaponInfo"),geometry.deviceScale==.5f);
    const auto* bomb = weapons.find("AS_BOMB_01");
    assert(bomb && bomb->damageRange > 0 && bomb->shotCount > 0);

    std::vector<ProjectileTarget> fullHealth;
    int minimum = std::numeric_limits<int>::max();
    std::string weakest;
    for (std::size_t row=0; row<parts.rowCount(); ++row) {
        const auto name = parts.value(row,"MachName");
        const auto* build = builds.find(name);
        assert(build && build->setType==0);
        const auto loadout = PlayerLoadout::fromBuild(*build,items,geometry);
        fullHealth.push_back({{0,0},loadout.hp,loadout.radius});
        if (loadout.hp < minimum) {
            minimum = loadout.hp;
            weakest = name;
        }
    }
    assert(builds.minimumMechHP(items,geometry)==minimum);
    // The catalog-wide minimum is in a starter build, usable from the first
    // stage, still valid at level 1 at the final mission or on a replay.
    auto save = SaveData::empty(1);
    save.createProfile("STARTER",1);
    const auto starter = save.builds(0);
    assert(std::any_of(starter.begin(),starter.end(),[&](const auto& b) {
        const auto* build=builds.find(b.name+"_01");
        return build && build->setType==0 && PlayerLoadout::fromBuild(*build,items,geometry).hp==minimum;
    }));
    const int cap = minimum * 95 / 100;
    int previous = 0;
    unsigned activeStages = 0;
    for (unsigned stage=0; stage<stages; ++stage) {
        const auto raid = EnemyAirRaid::forStage(stage,stages,minimum);
        assert(raid.has_value()==(stage>=10));
        if (!raid) continue;
        ++activeStages;
        const unsigned span=stages-11;
        assert(raid->maximumDamage==cap*(span+stage-10)/(2*span));
        if (previous) assert(raid->maximumDamage>=previous);
        previous=raid->maximumDamage;
        auto players=fullHealth;
        std::vector<ProjectileTarget> enemies{{{0,0},minimum,1}};
        int total=0;
        for (unsigned drop=0; drop<bomb->shotCount; ++drop) {
            const auto perDrop=raid->damageForDrop(drop,bomb->shotCount);
            const auto hits=projectileCollide(*bomb,{0,0},true,60,enemies,players,perDrop);
            assert(hits.size()==players.size());
            for (const auto& hit : hits) assert(hit.player && hit.damage==perDrop && !hit.special);
            total+=perDrop;
        }
        for (std::size_t i=0; i<players.size(); ++i) {
            assert(players[i].hp==fullHealth[i].hp-total);
            assert(20*players[i].hp>=fullHealth[i].hp);
        }
        assert(total==raid->maximumDamage && total<=cap && enemies[0].hp==minimum);
    }
    assert(activeStages==stages-10 && previous==cap);
    std::cout << "Enemy raid: " << stages << " stages, " << activeStages << " active, "
              << fullHealth.size() << " mech builds, minimum " << minimum << " (" << weakest
              << "), final-stage cap " << previous << "\n";
    return minimum;
}

double checkEnemyRaidInWorld(Resources& resources, Host& host, bool upgraded, unsigned stage, unsigned aircraftCount, int minimumMechHPForTest, bool testFinish=false) {
    using namespace md;
    auto save = SaveData::empty(1);
    save.createProfile(upgraded?"UPGRADED":"STARTER",1);
    save.writeWord(SaveData::slotStart+0x38,79);
    if (testFinish) save.setClearCount(0,stage,1);
    auto builds=save.builds(0);
    if (upgraded) {
        for (auto& b:builds) if (b.name=="MachineGun") b.level=9;
        save.setBuilds(0,builds);
    }
    World world(resources,host,save,0,BattleGeometry::phone(),781);
    world.start(stage);
    world.setValidationCells(10000);
    assert(world.enqueueBuild("MachineGun"));
    WorldActor* mech=nullptr;
    unsigned drops=0;
    constexpr unsigned shotsPerAircraft=10;
    std::vector<int> aircraftDamage(aircraftCount);
    bool detected=false,mechHit=false;
    const unsigned initialExplosions=host.groundExplosions;
    double warningTime=0,lastImpact=0;
    std::optional<std::uint64_t> lastRaidProjectile;
    const bool expected=aircraftCount>0;
    const auto isRaidReticle=[](const Sprite& sprite) {
        return sprite.image==enemyRaidReticleImage;
    };
    for (int tick=0; tick<4250; ++tick) {
        for (const auto& a:world.actors()) {
            a->weapons.clear();
            if (!a->player && !a->base) a->removed=true;
            if (a->player && !a->base && a->type==0) {
                mech=a.get();
                a->position=a->bipod.position={0,-330};
                a->bipod.setDestination(a->position);
                // Keep a target alive to measure each aircraft in multi-plane waves.
                if (aircraftCount>1) a->hp=a->maxHP;
            }
        }
        const unsigned before=host.groundExplosions;
        world.tick();
        if (mech && mech->hp<mech->maxHP) mechHit=true;
        if (host.groundExplosions!=before) lastImpact=world.time();
        if (!detected && world.time()-world.startTime()>=60) {
            Frame frame;
            world.appendSprites(frame);
            bool plane=false, warning=false;
            for (const auto& sprite : frame.sprites) {
                if (sprite.image==enemyRaidImage) {
                    plane=true;
                    const auto size=resources.imageSize(enemyRaidImage);
                    const float previousScale=1.3f*BattleGeometry::phone().deviceScale;
                    assert(std::abs(sprite.destination.width-size.x*previousScale*.5f)<.001f);
                    assert(std::abs(sprite.destination.height-size.y*previousScale*.5f)<.001f);
                }
                if (!isRaidReticle(sprite)) continue;
                assert(mech && !warning);
                const auto center=world.worldToScreen(mech->position);
                assert(std::abs(sprite.destination.x+sprite.destination.width*.5f-center.x)<.01f);
                assert(std::abs(sprite.destination.y+sprite.destination.height*.5f-center.y)<.01f);
                assert(sprite.sourcePx && sprite.sourcePx->x==0 && sprite.sourcePx->width==32);
                assert(sprite.blend==Blend::alpha && sprite.color.a>0 && !lastRaidProjectile);
                warning=true;
            }
            assert(plane==expected && warning==expected);
            detected=true;
            warningTime=world.time();
            world.setPaused(true);
            for (int i=0;i<60;++i) world.tick();
            assert(world.time()==warningTime);
            world.setPaused(false);
        }
        for (const auto& p:world.projectiles()) {
            if (!p.damageOverride || (lastRaidProjectile && p.renderOrder<=*lastRaidProjectile)) continue;
            assert(p.motion.drop && p.motion.toPlayer);
            assert(drops<aircraftCount*shotsPerAircraft);
            aircraftDamage[drops/shotsPerAircraft]+=*p.damageOverride;
            ++drops;
            lastRaidProjectile=p.renderOrder;
        }
        if (testFinish && drops>0) {
            for (const auto& a:world.actors()) if (a->base&&!a->player) {
                world.damage(a->id,a->hp);
                break;
            }
            assert(world.state()==WorldState::cleared);
            Frame frame;
            world.appendSprites(frame);
            for (const auto& sprite:frame.sprites) {
                assert(sprite.image!=enemyRaidImage && sprite.image!="Blt_Bullet.png");
                assert(!isRaidReticle(sprite));
            }
            const unsigned starts=host.airStrikeStarts;
            for (int i=0;i<180;++i) world.tick();
            assert(host.airStrikeStarts==starts);
            assert(world.projectiles().empty() && host.groundExplosions==initialExplosions);
            return 0;
        }
    }
    assert(detected && mech && !testFinish);
    Frame settledFrame;
    world.appendSprites(settledFrame);
    assert(std::none_of(settledFrame.sprites.begin(),settledFrame.sprites.end(),isRaidReticle));
    if (expected) {
        assert(drops==aircraftCount*shotsPerAircraft && host.groundExplosions-initialExplosions==drops);
        assert(lastImpact-warningTime<5+(aircraftCount-1)*1.5);
        const auto budget=EnemyAirRaid::forStage(stage,80,minimumMechHPForTest);
        assert(budget);
        for (const int total:aircraftDamage) assert(total==budget->maximumDamage);
        if (aircraftCount==1) assert(20*mech->hp>=mech->maxHP);
        if (stage==10||stage==79) assert(mechHit);
        return lastImpact-warningTime;
    }
    assert(!drops && host.groundExplosions==initialExplosions && mech->hp==mech->maxHP);
    return 0;
}

void checkPrecomputedRaids(const md::DataTable& table) {
    using md::EnemyAirRaidSchedule;
    assert(table.rowCount()==70);
    std::array<unsigned,3> counts{};
    for (unsigned stage=11;stage<=80;++stage) {
        auto schedule=EnemyAirRaidSchedule::fromTable(table,stage);
        const auto waves=raidWaves(table,stage);
        const auto row=*table.findRow(std::to_string(stage));
        const double period=static_cast<double>(*table.integer(row,"CycleSeconds"));
        assert(waves.front().at==60 && waves.size()==32);
        double previous=-1;
        for (int cycle=0;cycle<3;++cycle) {
            for (const auto wave:waves) {
                const double first=wave.at+period*cycle;
                if (previous>=0) assert(first-previous>=20 && first-previous<=45);
                if (cycle==0) ++counts[wave.count-1];
                for (unsigned aircraft=0;aircraft<wave.count;++aircraft) {
                    const double at=first+aircraft*1.5;
                    assert(!schedule.takeDue(at-.01));
                    assert(schedule.takeDue(at));
                    assert(!schedule.takeDue(at));
                }
                previous=first;
            }
        }
    }
    assert(std::all_of(counts.begin(),counts.end(),[](unsigned count) { return count>0; }));
    for (const auto row:{"11\t65\t59\t1\t85\t2", "11\t65\t60\t1\t79\t3",
                         "11\t70\t60\t1\t106\t2", "11\t39\t60\t1\t80\t3",
                         "11\t66\t60\t1\t80\t2", "11\t60\t60\t1\tmissing\t2",
                         "11\t60\t60\t0\t80\t2", "11\t60\t60\t1\t80\t4",
                         "11\t60\t60\t1\t80\tmissing"}) {
        const auto malformed=md::DataTable::parse(std::string("Stage\tCycleSeconds\tAt01\tCount01\tAt02\tCount02\n")+row);
        bool rejected=false;
        try {
            EnemyAirRaidSchedule::fromTable(malformed,11);
        } catch (const std::runtime_error&) {
            rejected=true;
        }
        assert(rejected);
    }
}

void checkWorldRaidSchedule(Resources& resources,Host& host,const md::DataTable& table) {
    using namespace md;
    constexpr unsigned stage=10;
    auto waves=raidWaves(table,stage+1);
    const auto row=*table.findRow(std::to_string(stage+1));
    waves.push_back({waves.front().at+static_cast<double>(*table.integer(row,"CycleSeconds")),waves.front().count});

    auto save=SaveData::empty(1);
    save.createProfile("RAID SCHEDULE",1);
    World world(resources,host,save,0,BattleGeometry::phone(),43);
    for (const bool skipFirst:{false,true}) {
        world.start(stage);
        world.setValidationCells(10000);
        bool queued=false;
        const auto elapsed=[&] {return world.time()-world.startTime();};
        const std::size_t end=skipFirst?3:waves.size();
        std::size_t next=skipFirst?1:0;
        unsigned aircraft=0;
        const auto last=waves[end-1];
        while (elapsed()<last.at+(last.count-1)*1.5+6) {
            if (!queued && (!skipFirst || elapsed()>=65)) {
                assert(world.enqueueBuild("MachineGun"));
                queued=true;
            }
            for (const auto& actor:world.actors()) {
                actor->weapons.clear();
                if (!actor->player && !actor->base) actor->removed=true;
                if (actor->player && !actor->base && actor->type==0) {
                    actor->position=actor->bipod.position={0,-330};
                    actor->bipod.setDestination(actor->position);
                    actor->hp=actor->maxHP;
                }
            }
            const unsigned starts=host.airStrikeStarts;
            world.tick();
            if (host.airStrikeStarts==starts) continue;
            assert(host.airStrikeStarts==starts+1 && next<end);
            const auto wave=waves[next];
            const auto scheduled=wave.at+aircraft*1.5;
            assert(elapsed()>=scheduled && elapsed()-scheduled<.017);
            Frame frame;
            world.appendSprites(frame);
            assert(std::none_of(frame.sprites.begin(),frame.sprites.end(),[](const auto& sprite) {
                return sprite.image==md::enemySu33Image;
            }));
            const auto planes=std::count_if(frame.sprites.begin(),frame.sprites.end(),[](const auto& sprite) {
                return sprite.image==enemyRaidImage;
            });
            const auto warnings=std::count_if(frame.sprites.begin(),frame.sprites.end(),[](const auto& sprite) {
                return sprite.image==enemyRaidReticleImage;
            });
            assert(planes==aircraft+1 && warnings==aircraft+1);
            if (++aircraft==wave.count) {
                aircraft=0;
                ++next;
            }
        }
        assert(next==end && aircraft==0);
    }
    std::cout<<"Stage 11: "<<waves.size()<<" waves through wrap, 1.5s between aircraft; no retiming on retry\n";
}

void checkSu33Campaign(Resources& resources, md::BattleGeometry geometry, unsigned stage, int expectedDamage) {
    using namespace md;
    Host host;
    auto save=SaveData::empty(1);
    save.createProfile("SU33",1);
    World world(resources,host,save,0,geometry,781);
    world.start(stage);
    const auto targetId=world.validationSpawn("AircraftHeavy_01",true,{0,0});
    auto* helicopter=world.actor(targetId);
    assert(helicopter && helicopter->flightTactics);
    helicopter->flightTactics->speed=0;
    helicopter->flightTactics->dodgeReady=1000;
    const auto initial=helicopter->hp;
    const auto stock=md::DataTable::load(resources.root/"Tables"/"EnemyAirRaidSchedule.tsv");
    const unsigned planes=expectedDamage>0 ? raidWaves(stock,stage+1).front().count : 0;
    bool sawPlane=false,sawBeam=false,sawReticle=false;
    for(unsigned tick=0;tick<4200;++tick) {
        for(const auto& unit:world.actors()) {
            unit->weapons.clear();
            if(!unit->player&&!unit->base)unit->removed=true;
        }
        world.tick();
        if(tick<3600)assert(host.airStrikeStarts==0);
        if(world.time()>=60 && !sawBeam) {
            Frame frame;
            world.appendSprites(frame);
            for(const auto& sprite:frame.sprites) {
                sawPlane|=sprite.image==enemySu33Image;
                sawBeam|=sprite.image=="Efx_Ion03.png";
                sawReticle|=sprite.image==enemyRaidReticleImage;
            }
        }
    }
    assert(host.airStrikeStarts==planes && host.raidLaserShots==14*planes);
    assert(sawPlane==(planes>0) && sawBeam==(planes>0) && sawReticle==(planes>0));
    assert(helicopter->hp==initial-expectedDamage*int(planes));
    assert(host.writes==0);
    std::cout<<"Su-33 campaign stage "<<stage+1<<": "<<planes<<" sorties, "<<expectedDamage<<" damage per aircraft\n";
}

int main(int argc,char**argv){assert(argc==2);Resources resources;resources.root=argv[1];Host host;
    const auto table=md::DataTable::load(resources.root/"Tables"/"EnemyAirRaidSchedule.tsv");
    checkPrecomputedRaids(table);
    checkWorldRaidSchedule(resources,host,table);
    for(const auto geometry:{md::BattleGeometry::phone(),md::BattleGeometry::pad()}) {
        checkSu33Campaign(resources,geometry,9,0);
        checkSu33Campaign(resources,geometry,10,168);
        checkSu33Campaign(resources,geometry,79,336);
    }
    const auto minimum=checkEnemyRaidDamage(resources,md::BattleGeometry::phone());
    assert(checkEnemyRaidDamage(resources,md::BattleGeometry::pad())==minimum);
    double latestImpact=0;
    for (unsigned stage=0;stage<80;++stage) {
        const auto aircraft=stage>=10 ? raidWaves(table,stage+1).front().count : 0;
        latestImpact=std::max(latestImpact,checkEnemyRaidInWorld(resources,host,false,stage,aircraft,minimum));
    }
    latestImpact=std::max(latestImpact,checkEnemyRaidInWorld(resources,host,true,79,raidWaves(table,80).front().count,minimum));
    checkEnemyRaidInWorld(resources,host,false,11,raidWaves(table,12).front().count,minimum,true);
    std::cout << "All world stages checked; latest raid impact " << latestImpact
              << "s after warning, stage finish on replay and profile independence checked\n";
    assert(host.writes==0);
    std::cout<<md::validation::runNuclearDamageSweep(resources,host);assert(host.writes==0);
    std::cout<<md::validation::runBaseUpgradeEffectSweep(resources,host);assert(host.writes==0);
    std::cout<<md::validation::runDamageAudioSweep(resources,host);assert(host.writes==0);
    for(auto geometry:{md::BattleGeometry::phone(),md::BattleGeometry::pad()}){
        auto save=md::SaveData::empty(1);save.createProfile("PLAYER",1);
        md::World world(resources,host,save,0,geometry,123);world.start(0);
        const float h=geometry.deviceScale==.5f?480.f:1024.f,margin=geometry.deviceScale==.5f?62.f:178.f;
        assert(world.cameraTargetY()==h*.5f-margin-geometry.halfLength());
        world.setUIMargin(margin+20);world.tick();assert(world.cameraY()==h*.5f-margin-20-geometry.halfLength());
        world.setUIMargin(margin);world.tick();assert(world.cameraY()==h*.5f-margin-geometry.halfLength());
        auto attacks=save.attacks(0);const auto initial=attacks[0].count;
        assert(world.activateSpecial(attacks[0].name,{0,0}));assert(save.attacks(0)[0].count==initial-1);
        md::Frame frame;world.appendSprites(frame);bool plane=false,marker=false;
        for(const auto& s:frame.sprites){
            plane|=s.image=="Airplane.png";
            if(s.image!="Target.png")continue;
            assert(s.sourcePx && s.sourcePx->x==0);
            assert(s.color.r==1 && s.color.g==1 && s.color.b==1);
            assert(s.blend==md::Blend::additive);
            marker=true;
        }
        assert(plane&&marker);
        bool drop=false;for(int i=0;i<120;++i){world.tick();for(const auto& b:world.projectiles())drop|=b.motion.drop;}assert(drop);
        world.start(0);attacks=save.attacks(0);attacks.push_back({"Crossfire-Missile",0,3,true});save.setAttacks(0,attacks);
        assert(!world.activateSpecial("Crossfire-Missile",{0,0}));assert(save.attacks(0).back().count==3);
        const auto builds=save.builds(0);assert(world.enqueueBuild(builds[0].name));
        md::WorldActor* unit=nullptr;
        for(int i=0;i<900&&!unit;++i){world.tick();for(const auto& a:world.actors())if(a->player&&!a->base&&a->type==0){unit=a.get();break;}}
        assert(unit);unit->position={0,-300};
        assert(!world.activateSpecial("Crossfire-Missile",{0,-301}));assert(save.attacks(0).back().count==3);
        assert(!world.activateSpecial("Crossfire-Missile",{0,-200}));assert(save.attacks(0).back().count==3);
        assert(world.activateSpecial("Crossfire-Missile",{0,-190}));assert(save.attacks(0).back().count==2);
        bool lock=false;for(const auto& p:world.projectiles())lock|=p.motion.lockOn;assert(lock);
        world.setPaused(true);assert(!world.activateSpecial(attacks[0].name,{0,0}));assert(host.writes==0);
        world.start(0);assert(world.cellLevel()==0&&world.cellCapacity()==500&&world.cellUpgradeCost()==300);
        assert(!world.upgradeCells());const auto permanent=save.baseUpgrade(0,5);const auto credits=save.credits(0);
        for(unsigned level=0;level<5;++level){
            int guard=50000;
            while(world.cells()<float(world.cellUpgradeCost())&&--guard){for(const auto& a:world.actors())if(!a->player)a->hp=0;world.tick();}
            assert(guard>0);const float beforeCells=world.cells();const int cost=world.cellUpgradeCost();
            assert(world.upgradeCells()&&world.cellLevel()==level+1&&world.cells()==beforeCells-float(cost));
            assert(world.cellCapacity()==500.f*float(level+2));assert(save.baseUpgrade(0,5)==permanent&&save.credits(0)==credits);
        }
        assert(!world.upgradeCells());world.start(0);assert(world.cellLevel()==0&&world.cellCapacity()==500);
        md::ActorId base=0;for(const auto& a:world.actors())if(a->base&&!a->player){base=a->id;a->coin=7;}
        const auto scoreOffset=md::SaveData::slotStart+0x2054;
        const auto scoreBefore=save.readWord(scoreOffset);bool callback=false;
        world.setFinishHandler([&](md::WorldState state){assert(state==md::WorldState::cleared);callback=true;assert(world.statistics().enemyDestroyed==0);assert(save.readWord(scoreOffset)==scoreBefore);});
        world.damage(base,10000000);assert(callback&&world.statistics().enemyDestroyed>=1&&save.readWord(scoreOffset)>=scoreBefore+7);

    }
    std::cout<<"World special attack inventory, qualification, projectile, sprite and UI margin tests passed\n";
}
