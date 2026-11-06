#include "../../../Modules/SaveTransfer/Validate.hpp"
#include "Core/Presentation/Menu.hpp"
#include <algorithm>
#include <cmath>
#include <ctime>
#include <limits>
#include <stdexcept>

namespace md {
namespace {
std::string readText(ResourceAccess& r,std::string_view path){auto b=r.read(path);return {b.begin(),b.end()};}
Rect centered(Vec2 c,float w,float h){return {c.x-w*.5f,480-c.y-h*.5f,w,h};}
std::uint32_t now(){return static_cast<std::uint32_t>(std::time(nullptr));}
std::string date(std::uint32_t value){
    const std::time_t t=value;const auto* tm=std::localtime(&t);char b[32]{};
    if(tm)std::strftime(b,sizeof(b),"%Y/%m/%d %H:%M:%S",tm);return b;
}
}
Menu::Menu(ResourceAccess& r,HostServices& h,SaveData& s,bool deferred):resources_(r),host_(h),save_(s),
    font_(readText(r,"Generated/TrebuchetMS-Bold-48.tsv")),maps_(DataTable::parse(readText(r,"Tables/MapList.tsv"))){
    deferredTransitions_=deferred;host_.playMusic("Intro.mp3");rebuild();
}
std::optional<Menu::Screen> Menu::takeScreenRequest(){auto value=requestedScreen_;requestedScreen_.reset();return value;}
void Menu::applyScreen(Screen screen){
    const bool managed=deferredTransitions_;deferredTransitions_=false;
    if(screen==Screen::profiles)showProfiles();
    else if(screen==Screen::missions)showMissions(slot_);
    else {closeTransfer();resetInput();screen_=Screen::title;host_.playMusic("Intro.mp3");titleTime_=0;titleVisible_=false;titleCycleSound_=false;rebuild();}
    deferredTransitions_=managed;
}
void Menu::setLayout(const FrameLayout& layout){layout_=layout;resetInput();if(transferOpen_)host_.layoutTransferField(transferBounds());rebuild();}
void Menu::resetInput(){
    for(auto& b:profiles_)b.reset();for(auto& b:stages_)b.reset();
    for(auto& b:transferActions_)b.reset();for(auto& b:transfer_)b.reset();for(auto& b:bottom_)b.reset();for(auto& b:dialogButtons_)b.reset();
    dragId_.reset();velocity_=0;
}
void Menu::showProfiles(){
    if(deferredTransitions_){requestedScreen_=Screen::profiles;return;}
    closeTransfer();resetInput();screen_=Screen::profiles;dialog_=Dialog::none;host_.playMusic("UI_BGM.mp3");
    std::uint32_t newest=0;
    for(std::size_t i=0;i<SaveData::slotCount;++i)if(save_.active(i)){
        auto stamp=save_.readWord(SaveData::slotStart+i*SaveData::slotStride+4);
        if(stamp>=newest){newest=stamp;slot_=i;}
    }
    rebuild();
}
void Menu::showMissions(std::size_t slot){
    if(!save_.active(slot))throw std::invalid_argument("Cannot select an empty profile");
    if(deferredTransitions_){slot_=slot;requestedScreen_=Screen::missions;return;}
    closeTransfer();resetInput();screen_=Screen::missions;dialog_=Dialog::none;slot_=slot;host_.playMusic("UI_BGM.mp3");
    selectedMission_=std::min(save_.stage(slot),79u);cam_=float(selectedMission_)*32;baseY_=0;rebuild();
}
bool Menu::persist(){
    const bool ok=host_.writeSave(save_.encode());if(!ok)dialog_=Dialog::saveFailed;return ok;
}
void Menu::selectProfile(std::size_t index){
    const auto before=save_;save_.selectProfile(index,now());
    if(!persist()){save_=before;return;}showMissions(index);
}
void Menu::newProfile(){
    bool empty=false;for(std::size_t i=0;i<SaveData::slotCount;++i)empty|=!save_.active(i);
    if(!empty){dialog_=Dialog::full;return;}
    namePending_=true;
    host_.requestName("",10,[this](std::optional<std::string> value){
        namePending_=false;
        if(!value){rebuild();return;}
        if(value->empty()||value->size()>10||std::any_of(value->begin(),value->end(),[](unsigned char c){return c<32||c>126;})){
            dialog_=Dialog::invalidName;rebuild();return;
        }
        const auto before=save_;const auto created=save_.createProfile(*value,now());
        if(created){slot_=*created;if(!persist())save_=before;}
        rebuild();
    });
}
void Menu::beginMission(){
    if(selectedMission_>save_.stage(slot_))return;
    save_.writeWord(SaveData::slotStart+slot_*SaveData::slotStride+0x34,selectedMission_);
    mission_=MissionRequest{slot_,selectedMission_};resetInput();
}
std::optional<MissionRequest> Menu::takeMissionRequest(){auto value=mission_;mission_.reset();return value;}
void Menu::tick(){
    if(screen_==Screen::title){
        titleTime_+=.0166f;
        const float phase=static_cast<float>(std::fmod(static_cast<double>(titleTime_),2.0));
        if(phase<.2f){
            if(!titleCycleSound_){host_.playSound("Title_TapToStart.wav");titleCycleSound_=true;}
            if(titleRandom_.next()%10<=5)titleVisible_=(titleRandom_.next()&1)==0;
        }else if(phase<1.2f)titleVisible_=true;
        else if(titleCycleSound_){titleCycleSound_=false;titleVisible_=false;}
    }
    if(screen_==Screen::missions && dialog_==Dialog::none){
        if(!dragId_){if(std::fabs(velocity_)>.1f)velocity_-=velocity_/10;else velocity_=0;cam_+=velocity_;}
        const auto current=save_.stage(slot_);const float bottom=current<5?125:float(current)*32;
        if(cam_<120)cam_=120-cam_>.1f?cam_+(120-cam_)/5:120;
        if(cam_>bottom)cam_=cam_-bottom>.1f?cam_+(bottom-cam_)/5:bottom;
        if(std::fabs(baseY_-cam_)>.1f)baseY_+=(cam_-baseY_)/5;else baseY_=cam_;
    }
    rebuild();
}
void Menu::sprite(std::string image,Vec2 center,float width,float height,unsigned tile,unsigned splitY){
    const auto sz=resources_.imageSize(image);
    Sprite s;s.image=std::move(image);s.sourcePx=Rect{0,float(tile)*sz.y/splitY,sz.x,sz.y/splitY};
    s.destination=centered(center,width,height);frame_.sprites.push_back(std::move(s));
}
void Menu::text(std::string_view value,Vec2 center,float width,float height,float size,Color color,int align){
    float x=center.x-width*.5f;if(align==1)x=center.x-font_.width(value,size)*.5f;
    if(align==2)x=center.x+width*.5f-font_.width(value,size);
    font_.draw(frame_,value,{x,480-center.y-height*.5f},size,color);
}
void Menu::number(std::string_view value,Vec2 position,bool right){
    float x=position.x-(right?float(value.size()-1)*6:0);
    for(unsigned char c:value){
        int index=-1;if(c>='0'&&c<='9')index=c-'0';
        else if(c==':')index=12;else if(c=='/')index=11;else if(c=='.')index=10;else if(c=='-')index=13;else if(c=='%')index=14;else if(c=='*')index=15;
        if(index>=0){Sprite s;s.image="NumSet01b.png";s.sourcePx=Rect{float(index*8),0,8,8};
            s.destination=centered({x,position.y},8,8);frame_.sprites.push_back(std::move(s));}
        x+=6;
    }
}
void Menu::bottom(unsigned index,std::string image,unsigned splitY){
    auto& b=bottom_[index];b.bounds=centered({80+80*float(index),30},screen_==Screen::missions?80:64,screen_==Screen::missions?40:32);
    sprite(std::move(image),{80+80*float(index),30},64,32,b.pressed()?1:0,splitY);
}
void Menu::drawDialog(){
    Sprite shade;shade.destination={0,0,320,480};shade.color={0,0,0,.7f};frame_.sprites.push_back(shade);
    sprite("MessageBox.png",{160,240},256,64);
    const char* a="",*b="";
    switch(dialog_){
    case Dialog::deleteProfile:a="Delete this profile?";b="Saved progress will be removed.";break;
    case Dialog::full:a="No empty profile slots.";b="Delete a profile to create another.";break;
    case Dialog::saveFailed:a="Could not save progress.";b="Please try again.";break;
    case Dialog::support:a="Support email is unavailable";b="in this build.";break;
    case Dialog::invalidName:a="Incorrect name.";b="Enter 1 to 10 English characters.";break;
    case Dialog::restoreConfirm:a="Replace all five profiles?";b="Your previous save will be backed up.";break;
    case Dialog::restoreError:a="Could not read recovery code.";b="Check the complete code and retry.";break;
    case Dialog::restoreInterrupted:a="Recovery needs to finish.";b="Close and reopen the app.";break;
    case Dialog::restored:a="Progress restored.";b="Choose a profile to continue.";break;
    case Dialog::none:return;
    }
    text(a,{160,250},240,16,12,{},1);text(b,{160,230},240,16,10,{},1);
    const bool confirm=dialog_==Dialog::deleteProfile||dialog_==Dialog::restoreConfirm;
    dialogButtons_[0].bounds=centered({confirm?112.f:160.f,185},64,32);
    sprite(confirm?"Common_Btn_YES.png":"Common_Btn_OK.png",{confirm?112.f:160.f,185},64,32,dialogButtons_[0].pressed()?1:0,confirm?2:4);
    if(confirm){dialogButtons_[1].bounds=centered({208,185},64,32);sprite("Common_Btn_NO.png",{208,185},64,32,dialogButtons_[1].pressed()?1:0,2);}
}
void Menu::rebuild(){
    frame_.sprites.clear();frame_.logicalWidth=320;frame_.logicalHeight=480;
    if(transferOpen_){frame_.logicalWidth=layout_.width;frame_.logicalHeight=layout_.height;drawTransfer();return;}
    if(screen_==Screen::title){
        sprite("Title.jpg",{160,240},320,480);
        if(titleVisible_)sprite("Main_TapToStart.png",{160,208},128,32);
        expandFrame();return;
    }
    sprite("iPon_SelSlot_BG@2x.jpg",{160,240},320,480);
    if(screen_==Screen::profiles){
        for(std::size_t i=0;i<SaveData::slotCount;++i){
            auto& b=profiles_[i];b.shown=save_.active(i);if(!b.shown)continue;
            const float y=340-80*float(i);b.bounds=centered({160,y},256,60);
            sprite("iPon_SelSlot_Sel@2x.png",{160,y},256,128,i==slot_||b.pressed()?0:1,2);
            text(save_.name(i),{136,y+20},128,16,10,{0,0,0,1});text(save_.name(i),{136,y+21},128,16,10,{160.f/255,1,1,1});
            const auto at=SaveData::slotStart+i*SaveData::slotStride;
            text(std::to_string(save_.readWord(at+0x2054)),{200,y+20},128,16,9,{160.f/255,1,1,1},2);
            // New profiles use the original English/us country image.
            const auto bytes=save_.plaintext();std::string country;
            for(std::size_t j=0;j<3&&bytes[at+0x30+j];++j)country+=char(bytes[at+0x30+j]);
            if(country.empty())country="us";
            sprite(country+".png",{60,y+23},16,16);
            number(date(save_.readWord(at)),{70,y-2});number(date(save_.readWord(at+4)),{70,y-20});
            number(std::to_string(save_.stage(i)+1),{234,y-1},true);number(std::to_string(save_.credits(i)),{248,y-21},true);
        }
        if(host_.supportsSaveTransfer())for(unsigned i=0;i<2;++i){
            auto& b=transfer_[i];b.bounds=centered({85.f+150*i,408},140,26);
            sprite("SysMenu_Btn.png",{85.f+150*i,408},140,26,b.pressed()?1:0,2);
            text(i?"ENTER CODE":"GET CODE",{85.f+150*i,408},132,14,12,{},1);
        }
        bottom(0,"Common_Btn_BACK.png");bottom(1,"Common_Btn_DEL.png");bottom(2,"Common_Btn_NEW.png");
    }else{
        const auto current=std::min(save_.stage(slot_),79u);const auto count=std::min(current+11,80u);
        for(unsigned i=0;i<count;++i){
            const float y=240+baseY_-32*float(i);stages_[i].bounds=centered({160,y},256,32);
            stages_[i].shown=true;
            sprite("SelStage_Btn_Stage.png",{160,y},256,32,i>current?2:stages_[i].pressed()?1:0,4);
            text("Stage "+std::to_string(i+1),{150,y},128,21,16,i>current?Color{10.f/255,10.f/255,10.f/255,1}:Color{10.f/255,1,1,1});
            const auto row=maps_.findRow(std::to_string(i+1));
            if(row){const std::string map(maps_.value(*row,"ImageFile"));sprite("t"+map+"_01.png",{198,y},48,24);sprite("t"+map+"_02.png",{246,y},48,24);}
            // GuiSelectStage::init 0x3c564 requests CompleteMark.png, which
            // is absent in WeMade 1.0.3. The port therefore has no stamp.
            // Do not substitute SelStage_CompleteMark.png at the off-screen
            // original child coordinate (160 + 160).
        }
        sprite("SelStage_Btn_Stage.png",{160,240+baseY_-32*float(selectedMission_)},256,32,3,4);
        sprite("iPon_SelStage_Fore@2x.png",{160,240},320,480);
        bottom(0,"Common_Btn_BACK.png");bottom(1,"Support.png");bottom(2,"Common_Btn_OK.png",4);
    }
    if(dialog_!=Dialog::none)drawDialog();
    expandFrame();
}
void Menu::expandFrame(){
    if(layout_.width==320&&layout_.height==480&&layout_.top==0&&layout_.bottom==0)return;
    const auto center=layout_.center();
    const Vec2 bottom{center.x,layout_.height-480-layout_.bottom};
    const Vec2 top{center.x,layout_.top};
    auto moveBounds=[](Button& b,Vec2 d){b.bounds.x+=d.x;b.bounds.y+=d.y;};
    for(auto& b:transfer_)moveBounds(b,center);
    for(auto& b:profiles_)moveBounds(b,center);
    for(auto& b:stages_)moveBounds(b,center);
    for(auto& b:bottom_)moveBounds(b,bottom);
    for(auto& b:dialogButtons_)moveBounds(b,center);
    auto original=std::move(frame_.sprites);frame_.sprites.clear();
    frame_.logicalWidth=layout_.width;frame_.logicalHeight=layout_.height;
    auto piece=[&](const Sprite& src,Rect pixels,Rect dest){auto s=src;s.sourcePx=pixels;s.destination=dest;frame_.sprites.push_back(std::move(s));};
    // Nine-slice only the existing background. Labels and buttons retain their
    // original proportions. No OS view or replacement artwork is introduced.
    auto panel=[&](const Sprite& src,bool title){
        const auto sz=resources_.imageSize(src.image);
        const float x=title?center.x:top.x,y=title?center.y:top.y;
        const float end=title?y+480:layout_.height-layout_.bottom;
        const float xs[]={0,x,x+320,layout_.width};
        if(title){
            for(int col=0;col<3;++col){if(xs[col+1]<=xs[col])continue;
                const float sx=col==2?sz.x-1:0,sw=col==1?sz.x:1;
                auto edge=src;edge.flipX=col!=1;edge.flipY=true;
                // Reflect cloud/fire edge regions, never the baked-in title lettering.
                piece(edge,{sx,0,sw,64},{xs[col],0,xs[col+1]-xs[col],y});
                edge.flipY=false;piece(edge,{sx,0,sw,sz.y},{xs[col],y,xs[col+1]-xs[col],480});
                edge.flipY=true;piece(edge,{sx,sz.y-120,sw,120},{xs[col],end,xs[col+1]-xs[col],layout_.height-end});
            }
        }else{
            // Header and footer keep their height; extend the unlabelled metal body.
            const float ys[]={0,y,y+106,end-78,end,layout_.height};
            const float sy[]={0,0,212,804,sz.y,sz.y};
            for(int col=0;col<3;++col)for(int row=0;row<5;++row){
                if(xs[col+1]<=xs[col]||ys[row+1]<=ys[row])continue;
                const float sx=col==0?0:col==1?0:sz.x-1,sw=col==1?sz.x:1;
                const float sourceY=row==0?0:row==4?sz.y-1:sy[row];
                const float sourceH=(row==0||row==4)?1:sy[row+1]-sy[row];
                piece(src,{sx,sourceY,sw,sourceH},{xs[col],ys[row],xs[col+1]-xs[col],ys[row+1]-ys[row]});
            }
        }
    };
    for(auto s:original){
        if(s.image=="Title.jpg"){panel(s,true);continue;}
        if(s.image=="iPon_SelSlot_BG@2x.jpg"||s.image=="iPon_SelStage_Fore@2x.png"){panel(s,false);continue;}
        if(s.image.empty()){s.destination={0,0,layout_.width,layout_.height};frame_.sprites.push_back(std::move(s));continue;}
        const bool bottomButton=s.image=="Common_Btn_BACK.png"||s.image=="Common_Btn_DEL.png"||s.image=="Common_Btn_NEW.png"||s.image=="Support.png"||(s.image=="Common_Btn_OK.png"&&s.destination.y==434);
        const auto d=bottomButton?bottom:center;s.destination.x+=d.x;s.destination.y+=d.y;
        frame_.sprites.push_back(std::move(s));
    }
}
Rect Menu::transferBounds()const{return {layout_.center().x+20,layout_.top+110,280,90};}
void Menu::closeTransfer(){if(transferOpen_)host_.closeTransferField();transferOpen_=false;for(auto& b:transferActions_)b.reset();}
void Menu::beginTransfer(bool exporting){
    if(screen_!=Screen::profiles||!host_.supportsSaveTransfer())return;
    resetInput();transferOpen_=true;transferExport_=exporting;
    host_.openTransferField(exporting?transfer::encode(save_):"",exporting,transferBounds());
}
void Menu::drawTransfer(){
    // Top anchored controls remain above the software keyboard. Art and hit
    // rectangles use the same final game coordinates on every platform.
    const float x=layout_.center().x,top=layout_.top;
    const auto first=frame_.sprites.size();
    text(transferExport_?"RECOVERY CODE":"ENTER RECOVERY CODE",{160,450},280,20,16,{},1);
    for(unsigned i=0;i<2;++i){const float cx=85+150*float(i);
        transferActions_[i].bounds={x+cx-65,top+55,130,32};
        sprite("SysMenu_Btn.png",{cx,409},130,32,transferActions_[i].pressed()?1:0,2);
        text(i?(transferExport_?"CLOSE":"CANCEL"):(transferExport_?"COPY":"CHECK"),{cx,409},124,16,13,{},1);
    }
    text(transferExport_?"Keep this code to restore all five profiles.":"Paste the complete MD1 code below.",{160,382},286,16,10,{},1);
    for(auto i=first;i<frame_.sprites.size();++i){frame_.sprites[i].destination.x+=x;frame_.sprites[i].destination.y+=top;}
}
void Menu::touch(const Touch& t){
    if(namePending_)return;
    if(transferOpen_){
        for(unsigned i=0;i<2;++i)if(transferActions_[i].touch(t).action==ButtonAction::pop){
            host_.playSound("UI_Btn_OK.wav");
            if(i==1)closeTransfer();
            else if(transferExport_)host_.copyTransferCode(transfer::encode(save_));
            else {
                const auto code=host_.transferFieldText();closeTransfer();
                try{restoreCandidate_=transfer::decode(code);transfer::validate(*restoreCandidate_,resources_);dialog_=Dialog::restoreConfirm;}
                catch(const std::exception& e){transferError_=e.what();dialog_=Dialog::restoreError;}
            }
            rebuild();return;
        }
        rebuild();return;
    }
    if(dialog_!=Dialog::none){
        const auto count=(dialog_==Dialog::deleteProfile||dialog_==Dialog::restoreConfirm)?2u:1u;
        for(unsigned i=0;i<count;++i)if(dialogButtons_[i].touch(t).action==ButtonAction::pop){
            if(i==0&&dialog_==Dialog::deleteProfile){const auto before=save_;save_.deleteProfile(slot_);dialog_=Dialog::none;if(!persist())save_=before;}
            else if(i==0&&dialog_==Dialog::restoreConfirm&&restoreCandidate_){
                try { if(host_.restoreSave(restoreCandidate_->encode())){save_=*restoreCandidate_;dialog_=Dialog::restored;}else dialog_=Dialog::saveFailed; } catch(const std::exception&) { dialog_=Dialog::restoreInterrupted; }
                restoreCandidate_.reset();
            }else {dialog_=Dialog::none;restoreCandidate_.reset();}
            for(auto& b:dialogButtons_)b.reset();rebuild();return;
        }
        rebuild();return;
    }
    if(screen_==Screen::title){if(t.phase==TouchPhase::ended){showProfiles();}return;}
    if(screen_==Screen::profiles&&host_.supportsSaveTransfer())for(unsigned i=0;i<2;++i){
        if(transfer_[i].touch(t).action==ButtonAction::pop){host_.playSound("UI_Btn_OK.wav");beginTransfer(i==0);rebuild();return;}
    }
    for(unsigned i=0;i<3;++i){
        const auto result=bottom_[i].touch(t);
        if(result.action==ButtonAction::pop){
            host_.playSound("UI_Btn_OK.wav");
            if(screen_==Screen::profiles){
                if(i==0){if(deferredTransitions_)requestedScreen_=Screen::title;else applyScreen(Screen::title);}
                else if(i==1&&save_.active(slot_))dialog_=Dialog::deleteProfile;
                else if(i==2)newProfile();
            }else{
                if(i==0)showProfiles();else if(i==1)dialog_=Dialog::support;else beginMission();
            }
            rebuild();return;
        }
        if(result.consumed){rebuild();return;}
    }
    if(screen_==Screen::profiles){
        for(std::size_t i=0;i<SaveData::slotCount;++i)if(profiles_[i].touch(t).action==ButtonAction::push){
            host_.playSound("UI_Click.wav");if(i==slot_)selectProfile(i);else slot_=i;break;
        }
    }else{
        if(t.phase==TouchPhase::began){
            const auto count=std::min(save_.stage(slot_)+11,80u);
            for(unsigned i=0;i<count;++i)if(stages_[i].contains(t.position)){
                dragId_=t.id;lastDragY_=t.position.y;velocity_=0;break;
            }
        }
        if(dragId_&&*dragId_==t.id&&t.phase==TouchPhase::moved){
            velocity_=lastDragY_-t.position.y;cam_+=velocity_;lastDragY_=t.position.y;
        }
        const auto count=std::min(save_.stage(slot_)+11,80u);
        for(unsigned i=0;i<count;++i){const auto result=stages_[i].touch(t);
            if(result.action==ButtonAction::pop&&i<=save_.stage(slot_)){
                host_.playSound("UI_Btn_OK.wav");if(i==selectedMission_)beginMission();else selectedMission_=i;break;
            }
        }
        if(dragId_&&*dragId_==t.id&&(t.phase==TouchPhase::ended||t.phase==TouchPhase::cancelled))dragId_.reset();
    }
    rebuild();
}
}
