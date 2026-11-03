# WeMade 1.0.3 소스 재구현 분석

2026-11-04 기준. WeMade 1.0.3 ARMv7 실행 파일에서 Objective-C 클래스 84개, 메서드 1,232개, 인스턴스 필드 732개를 추출했다. 로컬 REA/Ghidra로 메서드 전체의 어셈블리를 저장했고, 처음에는 1,230개 메서드의 의사코드를 얻었다. 두 메서드는 Ghidra가 추론한 float 인자 저장 위치가 겹쳐 실패했다. 별도 로컬 분석 프로젝트에서 인자를 ARM 32비트 워드로 지정해 `GobBullet::setFire:Angle:X:Y:DestX:DestY:`와 `QobLine::setLineX1:Y1:X2:Y2:`도 복구했다. 현재 1,232개 모두 어셈블리와 의사코드가 있다. 원본 실행 파일은 수정하지 않았다.

실행 파일 SHA-256은 `6ec47ea094abaa31bab10058fe47bb25d46c6038b2e88a7da2629899f84af112`다. 실행 파일과 디컴파일 출력은 저장소 밖 `/Users/sokol/Documents/Codex/Builds/MachDefense/source-20261104/analysis`에 있다. `objc-layouts.json`, `export-manifest.json`, `functions/<주소>.pseudo.c`, `functions/<주소>.asm`, `evidence/`가 분석 근거다.

## 추출 도구

- [export_objc_metadata.py](../../tools/reconstruction/export_objc_metadata.py): Mach-O 클래스·메서드·필드·상속 관계를 추출한다.
- [RecoverSignatures.java](../../tools/reconstruction/RecoverSignatures.java): 겹친 인자 저장 위치를 로컬 분석 프로젝트에서 다시 지정한다. 복구 결과는 `functions/0x8bbc.abi-words.c`, `functions/0x41dc4.abi-words.c`다.
- [export_functions.py](../../tools/reconstruction/export_functions.py): 기존 REA 설정으로 로컬 프로세스를 열고 메서드별 결과와 근거 ID를 저장한다.

기존 장기 실행 MCP 연결에서 Java 프로세스 식별 스냅샷 오류가 발생했다. Java 설치 부재가 원인은 아니었다. 기존 Java 21·Ghidra 경로를 사용하는 새 로컬 CLI와 MCP 프로세스에서 분석을 완료했다. 전역 보안 설정과 도구 설정은 바꾸지 않았다.

## 원본 규칙과 소비 코드

| 원본 메서드 | 확인한 규칙 | 새 코드 |
|---|---|---|
| `QChunk::loadChunkWithFp:` 0x45c04 | size/type/dataSize, 데이터, 자식 수, 자식의 재귀 구조. 선언 size는 탐색에 쓰지 않는다. | [Chunk.cpp](../../Sources/Core/Chunk.cpp) |
| `GlobalInfo::readPartsChunk:Pos:` 0x1bdf8 | 이름 24바이트, type/tileCount/socketCount 각 1바이트. 소켓은 offset 27부터 float 쌍. | `BattleModels::parse` |
| `GobHvMach::decodeChunk:Data:Pos:` 0x14984 | 타입 4의 이름 24바이트 뒤 parentSocket/zOrder 각 1바이트. 자식 파츠 재귀 조립. | `BattleModels::parse` |
| `PartsInfo::setSocket:X:Y:` 0x1fab0 | 소켓 좌표를 정수로 절삭한 뒤 폰은 0.5배, 패드는 1배. | `PartDefinition::socket` |
| `GlobalInfo::openPartsInfo` 0x1bf7c | PartsList의 TileName과 No로 공용 시트 지정. 파츠 type은 MCB 값 유지. | `BattleModels::configureTiles` |
| `GobMachParts::setPartsInfo:` 0x20144 | 공용 시트는 TileList의 분할 수, 개별 시트는 tileCount. 그림자 길이 기본 4, type 2는 2. | `BattleModels::appendSprites` |
| `GobMachParts::refreshRotationPos` 0x20cf4 | 부모 소켓과 자기 기준 소켓으로 위치 산출. 무기와 레이더 각도는 차체와 독립. | `BattleModels::appendSprites` |
| `GobWorld::tick` 0x1616c | 표시 중이고 일시정지가 아니면 `double(0.0166f)`씩 시간 증가. | `BattleClock::tick` |
| `GobWorld::processGameRule` 0x160ec | 셀이 최대치보다 작으면 한 번 증가. 초과분을 잘라내지 않는다. 다음 시각은 `float(time + double(0.08f))`. | `BattleClock::tick` |
| `GobWorld::setStage:` 0x1627c | 최초 셀은 용량의 40%. 적 기지 HP는 40,000 + 스테이지 인덱스 × 10,000. | `BattleClock` |
| `GobEnemySpawner::initWithSpawnInfo:` 0xb9a8, `tick` 0xbb1c | x는 ±220×scale, y는 mapHalf+32×scale. 시간 조건은 strict `<`. 다음 간격은 delay의 50~100%. | `EnemySpawner` |
| `BuildItem::setItem:` 0x304d8, `tick` 0x30644 | 시작 시각 float, 기간은 원본 Bld×업그레이드 계수. 진행률이 1을 초과할 때 완료. | `ProductionJob` |
| `GlobalInfo::openMachInfo` 0x1c304 | MachList HP는 2배. 폰 속도는 0.5배, 반지름은 0.5배 후 정수 절삭. | `MachineDefinition::fromTable` |
| `GobHvMach::processMove_Vehicle` 0x12eec | 도착 허용 폭 x/y 각각 4. 회전 보간 기본 30. 원거리 가속 100, 근거리 60. 확률 1/5의 위치·크기 흔들림. | `VehicleMotion::tick` |
| `GobHvM_Enemy::processAI_Attack` 0xdb20 | 살아 있는 아군 중 사거리 안의 가장 가까운 대상 선택. 동일 거리면 먼저 찾은 대상을 유지. | `nearestLivingTarget` |
| `GobHvMach::onDamage:From:` 0x13fd0 | HP에서 피해량을 빼고 0 이하를 0으로 처리. 사망 연출과 보상은 별도 절차. | `hpAfterDamage` |

추가로 [WeaponController.cpp](../../Sources/Core/WeaponController.cpp)에 원본 `PartsController_Weapon::tick`(0x211a8), `PartsController_WeaponAI::tick`(0x22b9c), `setReload`(0x22adc)의 재장전·연사·사거리·조준·반동 계산을 옮겼다. `shotDelay > 0`이면 한 틱에 최대 한 발을 처리하고, 0이면 남은 묶음을 같은 총구와 조준점으로 발사한다. 총과 저격총은 표적에 직접 피해를 주는 분기로 표시하고 나머지는 탄환 생성에 전달한다. 총구 섬광과 탄피를 생성하는 절차는 호출 측에 남아 있다.

`StageSpawnPlan`은 80개 ESpawn 표의 시간 지정·난이도 가중 배치와 MapInfo의 고정 배치·충돌 다각형을 읽는다. `appendGeneratedWaves`는 `GobWorld::openMap:`(0x18d88)의 일곱 차례 추가 웨이브를 HP 예산으로 생성한다. `EnemyDirector`는 `processEnemyGen`(0x1596c)의 두 위상·난이도 증가·가중 선택·재생성 간격을 처리한다. 고정 배치 유닛을 먼저 생성한 뒤 추가 웨이브를 계산해야 원본의 공유 난수 소비 순서를 유지한다.

`AircraftMotion`은 `processMove_AirCraft`(0x13370)를, `EnemySteering`은 적의 초기값(0xd3f0), 차량 행동(0xd67c), 항공기 행동(0xd8cc)을 옮겼다. 항공기의 목적지는 원본처럼 기기 배율과 관계없이 x/y 각각 ±600이다. 차량은 x에 ±16을 더하고 ±110×배율 안으로 제한한다. y는 160×배율씩 전진하며 기지 앞 사거리에서 멈춘다.

`PartPose`는 조립 파츠의 위치·각도·총구 위치를 제공한다. `getMuzzlePos`(0x20b68)와 `setNextMuzzle`(0x20b20)의 소켓 1부터 순환하는 규칙을 사용한다. 소켓이 부족한 더미 파츠는 총구가 없다.

## 탄환과 동적 파츠

[Projectile.cpp](../../Sources/Core/Projectile.cpp)는 `GobBullet::setFire`(0x8bbc), `tick`(0x9558), `setCollide`(0xa568), `set2ndDamage`(0x9f80), `setSpDamage`(0x9da4)를 옮겼다.

- 최초 탄속은 원본 속도의 ±10% 난수다. 미사일은 여기서 0.05배를 가속 벡터로 쓰며 틱마다 속도에 더한다.
- 일반 이동은 속도를 0.0166으로 나눈 값을 위치에 더한다. 추적 미사일의 가속 방향은 15, 속도 방향은 20으로 나눠 목표 방향에 접근한다.
- 발사 직후 별도 이동 벡터가 있으면 각 성분을 15로 나눠 0에 접근시킨다. 절댓값이 0.5 이하면 0으로 만든다.
- 목표에 충분히 접근한 뒤 지나쳤거나 다음 이동 폭 안에 들어오면 충돌을 처리한다. 폭탄 이미지의 축소도 남은 거리의 제곱 비율을 쓴다.
- 범위 피해는 반지름 안에서 거리에 따라 감소한다. 피해 반지름 0의 원본 분기는 발사 진영과 관계없이 적 목록을 검사한다. 이 동작을 그대로 옮겼다.
- 레이저는 충돌점 주변 다른 적에게 절반 피해를 추가한다. 앞선 피해로 죽은 적은 다음 순회에서 제외한다.
- plasma·EMP의 시작·종료 시각, 주기, 피해량을 생성한다. `SpecialDamageController`가 상태 목록과 갱신 시각을 유지하고 피해·소리·입자 이벤트를 발생시킨다.

`BattleModels`에 동적으로 만든 파츠 트리를 넣는 API를 추가했다. 각 파츠는 독립 각도, 로컬 이동, 크기, 반전을 가진다. `Tile2D::drawTile`(0x4cf94)의 reverse 값 2는 세로 반전이다. 플레이어 오른쪽 무기와 총구 위치도 이 반전을 사용한다.

### 탄환 효과 생성과 남은 연결

`GobBullet::draw`(0xb290)는 effectType 5~8이면 궤적에 현재 위치를 넣는다. type 3과 4는 시간이 `effectRegenTime + effectDelay`를 넘으면 입자를 얻는다. 입자 풀이 비어 있으면 난수를 소비하지 않는다. type 3은 위치 x/y·크기·크기 변화·각도·각속도로 난수 6개, type 4는 크기·크기 변화·각도·각속도로 4개를 소비한다. 이 규칙은 `Projectile::drawEffect`에 구현했다. 생성 조건·입자 속성·풀 부족 시각·난수 소비 개수를 검사했다. `ParticlePool`과 `ParticleState`에 입자 할당·수명·이동·축소·투명도·타일 애니메이션을 구현했다. World에서 실제 그리기 시점에 이 API를 연결한다. 연속 궤적의 삼각형 스트립 표시 작업은 남아 있다.

## 검사 결과

`Tests/BattleTests.cpp`를 C++20, `-Wall -Wextra -Werror -ffp-contract=off`로 빌드하고 원본 리소스에 실행했다.

- MapInfo 80개 맵, MachInfo 126개 조립 모델, PartsInfo 463개 파츠를 읽었다.
- 세 파일 모두 선언된 루트 크기와 실제 소비 크기가 다르다. 원본처럼 자식 수를 따라 파일 끝까지 읽었다. 잘린 파일은 실패한다.
- 126개 모델의 스프라이트 명령을 생성했다. 원본 Dummy 모델 외 125개 모델이 표시할 파츠를 가지며, 시트 좌표는 실제 PNG 범위를 벗어나지 않는다.
- 소켓 절삭·비율, 고정 시간 증가·일시정지, 셀 초과, 생산 완료 경계, 스폰 위치·간격·개수, HP·속도 변환, 차량·항공기 도착·초기 가속, 표적 거리 경계를 검사했다.
- 80개 맵의 고정 배치·충돌 다각형·ESpawn 표·추가 웨이브와 첫 적응형 스폰을 생성했다. 원본 ESpawn_77의 빈 Dif, ESpawn_80의 빈 Dly는 원본 getInt처럼 0으로 읽는다.
- WeaponControllerTests에서 재장전, 묶음 발사, 0초 발사 간격, 수동 사격, 사거리 경계, 상태 효과 2의 발사 차단, 표적 추적, 반동을 검사했다.
- ProjectileTests에서 속도 난수·일시정지·직진·목표 도달·발사 벡터 감쇠·미사일 가속/추적·회전·범위 피해·레이저 연쇄·상태 효과 시각·이미지 축소를 검사했다.
- BattleTests, WeaponControllerTests, ProjectileTests는 AddressSanitizer와 UndefinedBehaviorSanitizer에서도 통과했다.

이 검사는 데이터와 계산 검증이다. 원본과 앱 화면을 같은 프레임에서 대조하는 검사는 아직 남아 있다.

## 추가 전투 규칙

`SpecialDamageController`는 원본 상태 검사 간격 0.1초, plasma 피해 간격, EMP 효과 간격, 갱신 시각의 float 절삭을 유지한다. 같은 종류를 다시 적용하면 기존 피해량·주기를 유지하고 종료 시각과 피해 시각만 갱신한다. EMP를 적용한 순간 표적을 지우고 목적지를 현재 위치로 바꾸는 이벤트를 돌려준다.

원본은 `NSDictionary allValues` 순회 중 누적한 플래그로 각 항목의 피해 분기를 결정한다. plasma와 EMP가 함께 있으면 순회 순서에 따라 EMP 항목도 피해 분기에 들어갈 수 있다. 새 코드는 순서가 있는 항목 목록을 받는다. 두 순서를 각각 검사했다. 원본 Foundation이 같은 상황에서 선택한 순서는 런타임 대조가 남아 있다.

`ParticlePool`은 원본처럼 256개다. 초기화 때 입자마다 난수 한 개를 소비하고, 재할당 때도 한 개를 소비한다. 풀이 가득 차면 추가 난수를 소비하지 않는다. `QobParticle::tick`은 수명이 있는 입자의 크기 변화를 한 틱에 두 번 적용한다. 이 동작과 수명 없는 입자의 별도 분기를 그대로 옮겼다.

[Loot.cpp](../../Sources/Core/Loot.cpp)는 원본 `GobHvM_Enemy::onCrash`, `GobItem`의 생성·수집·소멸 절차를 구현한다. 적 파괴 즉시 크레딧을 더하지 않는다. 원본 Coin 값과 확률로 광물·크레딧 아이템을 만들고, 아이템을 수집할 때 셀과 크레딧을 반영한다. 수집 대상은 범위 안에서 처음 찾은 살아 있는 type 0 아군이다. 탐색 범위는 0.5초마다 커지며, 180초가 지나도 수집 대상을 찾지 못하면 사라진다. 광물은 셀 용량까지 채운다. 아이콘과 빛 효과도 원본 시트·타일을 사용한다.

[SpecialAttack.cpp](../../Sources/Core/SpecialAttack.cpp)는 Crossfire와 AirStrike를 구현한다. Crossfire는 기체 위치에서 짝수·홀수 탄의 초기 각도를 반대 방향으로 벌리고, 터치점 주위 ±15에 추적탄을 발사한다. AirStrike는 목표에서 1,200×기기 배율만큼 떨어져 출발하며 속도가 틱마다 1% 증가한다. 원본 지연 후 0.3/발사 수 간격으로 폭탄을 떨어뜨린다. `Airplane.png`와 `Target.png`를 사용한다. 표적 효과의 크기·회전·수명도 원본 값이다.

`GobHvM_Bot`의 방어·수리 전용 AI 함수는 실행 파일에서 즉시 반환한다. 이를 근거 없이 확장하지 않았다. 같은 클래스의 5초당 HP 1 회복과 `GobHvM_Player::aiBot`의 도착 후 설치 유닛 변환 조건을 각각 구현했다.

`TerrainSegment`는 원본 충돌 선분의 방향·법선·이전 선분과의 모서리 방향을 계산한다. 반지름 16과 선분 앞뒤 경계를 사용한다. `TrainMotion`은 열차의 가속·감속을 옮겼다. 현재 MachList의 타입 문자열에는 TRAIN이 없으므로 새 타입을 임의로 추가하지 않았다.

속도 표시값은 원본 `spdPoint` 계산을 쓴다. 차량은 이동 속도를 0.0166으로 나눈 뒤 정수로 절삭한다. 보행 파츠는 한 걸음의 보간 이동을 반복하고 누적 거리/시간을 정수로 절삭한다. 이 값은 임의의 100배 표시값과 다르다.

### World 연결

`World::activateSpecial`은 저장 슬롯의 공격 이름·레벨로 SpAttackList와 ItemInfo를 찾아 사용한다. 공습은 생성 즉시 재고를 하나 차감한다. 집중사격은 살아 있는 type 0 아군 중 목표보다 뒤에 있고, 목표와의 거리 제곱이 10,000을 초과하며 무기 사거리 안인 기체만 발사한다. 발사 기체가 없으면 재고를 유지하고 0.5초 표적 효과와 원본 경고음을 낸다. 성공하면 표적 효과는 3초이며 재고 하나를 차감한다. 파일 저장은 이 함수에서 하지 않는다.

게임 HUD 위치는 `World::setUIMargin`으로 카메라 하단에 반영한다. 원본 `setStage`의 초기 178을 계속 쓰면 폰 화면에서 맵과 HUD 사이가 벌어진다. 원본 GuiGame은 시작 직후 폰 62, 패드 178로 덮어쓴다. 새 코드도 이 기본값과 HUD 이동값을 사용한다.

인게임 셀 확장은 영구 CellStorage 업그레이드와 구분했다. `upgradeCells`는 현재 셀에서 현재 용량의 60%를 차감하고 전투 중 단계만 올린다. 최대 다섯 번이며 용량은 영구 CellStorage 기본 용량 × (전투 중 단계+1)이다. 새 미션을 시작하면 단계는 0으로 돌아간다.

원본 `onClearStage`는 결과창을 생성하고 보상·저장을 처리한 다음 남은 적을 파괴한다. 이를 다음 틱의 이벤트 처리까지 미루면 정리 중 소비한 난수가 보상 추첨에도 영향을 준다. `setFinishHandler`를 추가해 World가 결과 상태를 설정한 직후 결과창 생성 절차를 동기 호출하도록 했다. 기지 자신의 파괴 수와 Coin 점수도 이 호출 뒤에 더한다. 상태 이벤트는 별도로 유지한다.

### 추가 검사

`BattleTests`, `ProjectileTests`, `LootTests`, `SpecialAttackTests`를 AddressSanitizer와 UndefinedBehaviorSanitizer로 실행해 통과했다. 충돌 선분·열차 이동·속도 표시값, 두 상태 효과의 순서, 입자 풀 소진, 원본 아이템 테이블·수집·시간 경계, Crossfire 난수 순서, AirStrike 발사·퇴장 시각을 검사했다.

## 남은 전투 대조

World 담당자가 전투 관리자에 이동·생산·표적·탄환·상태 효과·보상 API를 연결하고 있다. 각 하위 모듈의 검사 결과만으로 전체 미션의 재현을 완료했다고 판단하지 않는다. 원본과 새 앱의 같은 전투를 비교해야 한다.

남은 항목은 연속 탄환 궤적 메시, 총구 섬광·탄피·사망 폭발의 모든 난수 소비 순서, 복합 상태 효과의 원본 사전 순회 순서, 파츠·그림자·충돌·특수공격의 실제 화면 대조다. 이 항목들의 원본 의사코드와 어셈블리는 외부 분석 폴더에 확보했다.
