# MachDefense 공용 모듈 진단과 플랫폼 확장 계획

이 문서는 분리 전 진단이다. 실제 변경과 검사 결과는 [모듈화 작업 기록](evidence/module-work-20261105.md)에 있다.

2026-11-05 · 조사 기준: `feat/source-reimplementation`, `eb8b334cc9d5f69784175abe0e782a586b436771`. 이후 미커밋 변경은 문서다. 이번 작업은 코드·빌드 구성·HeavyMach 이력의 정적 조사이며, 게임 코드나 설치 앱을 변경하지 않았다.

## 1. 판단과 최종 목표

**HeavyMach와 같은 방식으로 공용 코어·자원·플랫폼 연결을 나눌 수 있다. MachDefense에는 이미 그 바탕이 있다.** 현재 C++20 코어 26개 구현 파일과 32개 헤더가 하나의 정적 라이브러리로 빌드되고, 이미지·음원·게임 표는 별도 자원 번들에 들어간다. 이 코어에는 UIKit·OpenGLES·OpenAL 헤더를 직접 포함하는 파일이 없다. 화면과 소리는 Apple 연결 코드가 출력한다.

필요한 작업은 기존 코어 내부의 책임 분리다. `World.cpp`와 `Battle.cpp`에 여러 게임 규칙과 화면 구성이 섞여 있어, 한 부분을 수정할 때 함께 확인해야 할 범위가 크다. 이 내부 책임을 나누고 플랫폼 연결에서 원작과 달라진 구현을 바로잡는 것이 이번 모듈화의 목적이다.

최종 목표는 **iPhone·iPad 네이티브 → Android 네이티브 → 웹·PWA → Tauri Mac 앱**이다. 웹은 HeavyMach PR의 방식처럼 같은 C++ 코어를 WebAssembly로 빌드한다. PWA와 Tauri도 이 웹 게임 산출물을 사용한다. MachDefense 게임 규칙을 Kotlin·JavaScript·Rust로 다시 작성하지 않는다. Tauri Mac과 AppKit 기반 네이티브 Mac은 다른 실행 방식이다. 이번 최종 목표에 명시된 Mac은 Tauri이며, AppKit 프로젝트를 먼저 만들어야 하는 의존성은 없다.

원작 비교가 끝나지 않은 지형·이동·표적 버그는 별도 수정 이력을 둔다. 모듈 분리가 해당 버그를 고쳤다고 간주하지 않는다. OpenAL 교체는 같은 작업 단계에 포함하되 독립 커밋과 검사로 진행한다.

## 2. HeavyMach에서 실제로 한 일

| 이력 | 확인한 내용 | MachDefense 적용 |
|---|---|---|
| `58ade7ae`의 부모 `172ab045` | 이미 `Sources/Core`, `HeavyMachCore` CMake 대상과 기능별 검사가 있었다 | 새로 코어를 만드는 일과 기존 코어를 플랫폼들이 공유하도록 묶는 일을 구별한다 |
| `feat/apple-workspace-modules`, `58ade7ae` | 공용 Apple 모듈, AppKit Mac 앱, 키보드 조작을 추가했다. 82개 파일 변경 | workspace 아래 앱 프로젝트가 공용 코어와 자원을 참조하는 방식을 따른다 |
| `Modules/project.yml` | 일반/치트 코어 정적 framework, AppleRuntime, Assets.bundle을 구분했다 | 현재 MachDefenseCore 정적 library와 Assets.bundle을 유지하며 필요한 연결만 추가한다. 모든 논리 모듈을 framework로 만들 필요는 없다 |
| `63afa4f5` | 부모 `172ab045`, `6404c999`를 갖는 develop 병합. `58ade7ae`가 조상이다 | 기능 브랜치 → develop 규칙을 따른다. 이 병합에는 VTOL 후속 작업도 포함돼 있어 모듈화만의 결과로 집계하지 않는다 |
| `feat/android-app` 및 Android CMake | 루트 코어를 `add_subdirectory`로 빌드하고 JNI·Activity·EGL·오디오·저장 연결을 Android에 둔다 | 게임 규칙 복제 없이 같은 코어 target을 NDK에서 빌드한다 |
| fastutlego의 `porting/wasm`, `00dc422f`, PR #24 병합 `757523e1` | C++ 코어를 Emscripten으로 빌드하고 WebGL·웹 음향·입력·저장 연결을 추가했다 | 웹 브리지와 렌더 변환 방식을 참고한다. 게임 이름·미션·슬롯 형식까지 복사하지 않는다 |
| HeavyMach `Web/CMakeLists.txt` | 코어 파일 목록을 별도로 열거하고 `Sources/Web`과 함께 빌드한다 | 동일 소스를 사용한다는 원칙은 유지하되, MachDefense는 공용 소스 목록을 한 곳에서 가져와 누락을 방지한다 |
| HeavyMach `Desktop/Tauri` | 웹 산출물을 감싸는 실행기와 파일 저장·종료 처리가 있다 | Tauri는 창·종료·저장 파일을 맡고 전투 계산은 웹 게임이 수행한다 |

HeavyMach의 AppleRuntime 일부 구현은 역사적으로 `Sources/iOS`에 남아 있고 Android·웹에는 별도 메뉴/HUD 연결이 있다. MachDefense는 현재부터 메뉴·HUD·입력을 C++로 작성했으므로 이 코드를 플랫폼마다 복제할 이유가 없다. HeavyMach의 **공유 방식과 검증 순서**를 가져오고, 폴더 이름만 iOS인 공용 파일까지 재현하지 않는다.

원본 확인 경로: HeavyMach `docs/apple-platforms.md`, `Modules/project.yml`, `Android/cmake/CMakeLists.txt`, `Web/CMakeLists.txt`, `Desktop/Tauri/`; 현재 원격 추적 `origin/main`은 `53cf9b5a`였다. 다른 작업 브랜치가 checkout돼 있어 최신 플랫폼 파일은 `git show origin/main:<경로>`로 읽었다. HeavyMach 파일·브랜치를 변경하지 않았다.

## 3. 현재 기술 스택은 어디까지 같은가

| 분야 | HeavyMach | MachDefense 현재 | 판정 |
|---|---|---|---|
| 게임 규칙 | C++20 | C++20 | 같은 언어·빌드 계열. 서로 다른 게임의 규칙과 수치를 공유한다는 뜻은 아니다 |
| Apple 연결 | Objective-C++ | Objective-C++ | 동일 |
| iPhone/iPad 렌더 | OpenGL ES 1.1 | `kEAGLRenderingAPIOpenGLES1`, fixed-function 경로 | 같은 API 계열. 렌더 소스 자체가 동일하지는 않다 |
| 텍스처 해석 | stb 기반 | `Modules/ThirdParty/stb`, PNG/JPEG | 같은 계열. 필터·알파·크롭·업로드 조건은 따로 대조해야 한다 |
| 음향 출력 | OpenAL Soft 1.22.2 | Apple `OpenAL.framework` | **다르다. MD-AUDIO-BACKEND-01 수정 대상** |
| 음원 해석 | HeavyMach WAV 형식용 처리 | AudioToolbox `ExtAudioFile`로 WAV/MP3를 PCM 변환 | 다르다. MachDefense 배경음 MP3를 포함한 실제 자원을 기준으로 분리해야 한다 |
| 공용 빌드 | CMake + XcodeGen, static framework와 번들 | CMake + XcodeGen, static library와 번들 | 같은 관리 방식. 포장 형식은 다르다 |
| UI | 원본 에셋·게임 렌더, 플랫폼 연결 일부 | 메뉴·HUD·업그레이드·도움말·치트 UI가 C++ Frame을 생성 | MachDefense의 공용 C++ UI를 그대로 여러 플랫폼에서 사용 가능 |
| 플랫폼 범위 | Apple·Android·웹·PWA·Tauri 구현 이력 | 이번 소스판은 iPhone/iPad 구현 | Android·웹·PWA·Tauri는 계획 단계 |

현재 소스판은 새 C++ 구현이 준비된 `Resources`를 읽는다. IPA의 ARM 명령을 실행하는 포팅판과 다른 앱이다. 이미 재구현했다는 사실이 원작의 모든 동작을 맞췄다는 뜻은 아니다. 지형 정체·음향 구현 차이 등 확인된 차이를 아래 작업에서 다룬다.

## 4. 공용으로 묶을 범위와 영향을 받는 파일

아래 경로는 별도 표기가 없으면 `Sources/Core` 기준이다. ‘분리’는 먼저 책임과 파일을 나누는 뜻이다. 처음부터 수십 개 링크 라이브러리를 만들지 않고 기존 `MachDefenseCore`를 계속 사용한다.

| 모듈·역할 | 현재 파일·함수 | 영향을 받는 요소 / 분리 방법 | 왜 필요한가 |
|---|---|---|---|
| 기본 타입·시간·난수 | `RenderTypes.hpp`의 Vec2/Rect, `DarwinRandom.hpp`, `Battle.hpp/.cpp`의 BattleGeometry/BattleClock | 좌표형·원작 틱·난수는 하위 공용 타입으로 둔다. 화면용 Sprite와 수학형을 분리하되 계산식·정밀도 유지 | 이동·피해 계산이 렌더 헤더 전체에 의존하지 않도록 한다 |
| 표·원본 데이터 | `DataTable.*`, `Chunk.*`, BattleModels/WeaponCatalog/BuildCatalog 등 | MCB·게임 표 파싱, 정의 데이터는 C++ 유지. 파일을 읽는 일은 ResourceAccess로 전달 | 모든 플랫폼이 같은 미션·무장·파츠 정의를 읽게 한다 |
| 피해·특수 피해·사망 | `Combat.*`, `Projectile.*`, `World::damage/updateProjectiles`, `WorldEffects.cpp` | 기본 피해·범위 피해·상태이상 계산과 HP 적용·소멸·보상 연결을 분리. 적용 순서와 ActorId 유지 | 핵폭탄 살상 반경, 진영 판정, 연속 피격과 보상 처리가 플랫폼마다 갈라지는 것을 막는다 |
| 메크·차량·항공기 이동 | `Bipod.*`, Battle의 VehicleMotion/AircraftMotion/TrainMotion, `World::updateActor/setDestination` | 이동 상태와 목표 좌표 계산을 구분. 메크 발·몸체 포즈 출력 유지 | 아군 메크와 적 이동을 독립 검사하면서 원작의 덜컹거림·기울기를 보존한다 |
| 적 AI·아군 명령 | Battle의 EnemySteering, Bipod의 PlayerSteering/checkPlayerMoveAhead, `Formation.*`, World의 명령/AI 호출 | 후퇴·정지·목표 갱신·아군 장애물 입력을 명시한다 | 길막힘 문제를 모션·명령·충돌 중 어디에서 일어나는지 분리해 검사한다 |
| 지형·유닛 충돌 | TerrainSegment, separateGroundBodies, `World::buildTerrain/collideTerrain/collideGroundBodies` | 맵 선분·공간 셀 등록·충돌 판정·위치 보정 분리. 원본 셀 크기·순회 순서 유지 | 미션 4·10·11, 밀집 터렛, 구멍 위 설치 문제의 재현 범위를 좁힌다 |
| 표적·사거리·발사 | `World::updateActor/updateWeapons`, `WeaponController.*`, `Combat.*` | 표적 후보/사거리/진영 → 표적 유지/갱신 → 포신 추적 → 발사 조건의 경계를 만든다 | 화면 밖 적에 대한 사격 실패를 카메라 표시와 구별해 검사한다 |
| 탄환·필살기 | `Projectile.*`, `SpecialAttack.*`, World의 addProjectile/activateSpecial/airStrikes | 탄도·도착·피해 적용과 효과 출력 분리. 직접 피해 총탄과 비행 탄환 구분 유지 | 총탄을 모두 새 투사체로 바꾸거나 특수기술 계산을 재해석하지 않는다 |
| 스폰·생산 | Battle의 EnemySpawner/EnemyDirector/StageSpawnPlan, `Production.*`, World의 start/enqueueBuild/tick | 적 출현 표·추가 웨이브·생산 대기열·완료 위치·운반봇 변환 | 80미션 출현과 생산 비용/완료 시점이 동일 코드를 사용하게 한다 |
| 경제·강화·보상 | `Economy.*`, `PlayerParts.*`, `Settlement.*`, `Loot.*`, World의 refreshBaseUpgrades/damage | 자금·셀·수집·강화 효과·해금과 외형 파츠 연결 보존 | 구매 기록만 남고 실제 효과가 빠지는 오류를 막을 검사 경계를 만든다 |
| 부품 조립·포구 | BattleModels/AssemblyPart/PartPose, PlayerParts, World의 configureWeapons/updateWeapons | 소켓·계층 변환·반전·포구 위치 계산은 공용. 스프라이트 제출만 표현부에 둔다 | **포구 좌표는 탄도 입력이다.** 조립을 단순 그림 코드로 취급하면 사격이 달라진다 |
| 게임 상태·저장 | `GameSession.*`, `SaveData.*`, Menu, Settlement, ValidationTools | 슬롯 형식과 상태 전이는 공용. 저장 매체만 플랫폼별 구현. 화면·전투·강화 모달의 pause 판단 보존 | 플랫폼 간 저장값과 진행 규칙을 유지한다 |
| 카메라·효과·프레임 구성 | World의 camera/touch/prepareDraw/appendSprites, `WorldEffects.*`, `Trail.*`, ParticlePool, `FrameLayout.hpp` | 논리 월드↔표시 좌표, Sprite/Mesh 생성, 원작 그리기 순서 분리. 난수를 소비하는 효과 갱신은 원래 실행 순서에 둔다 | 화면 확장과 렌더 API 교체가 이동/탄도에 영향을 주지 않도록 한다 |
| 메뉴·HUD·글꼴 | `Menu.*`, `GameHUD.*`, `Button.hpp`, `UpgradeDialog.*`, `SummaryDialog.*`, `Transition.*`, `Help.hpp`, `SpriteFont.hpp` | 원본 에셋 조합과 히트박스, 버튼 이벤트를 C++에서 함께 관리 | iOS·Android·웹에서 같은 버튼과 터치 판정을 사용한다 |
| 치트 검사 | `ValidationTools.*`, `ValidationPanel.*`, GameSession·World의 validation 진입점 | 일반 게임 규칙을 그대로 호출하며 검사 조건만 설정. 공개 일반 앱에서는 진입 차단 | 검증용 전투 로직을 따로 만들어 정상 앱의 버그를 가리지 않는다 |

## 5. 크게 나누기 전에 보존해야 할 연결

### 한 프레임의 처리 순서

`World::tick`은 시간·카메라 → 적 스폰·생산 완료 → 기존 액터 갱신 → 공습 → 트레일 → 탄환 → 파티클 → 수집물 순으로 처리한다. 액터 갱신 시작 시 크기를 고정해, 그 도중 생성된 자식 유닛은 다음 갱신에서 처리한다. 이 순서와 생성/제거 시점을 바꾸면 같은 난수를 넣어도 결과가 달라진다.

`World::updateActor` 안에는 표적 정리·탐색, AI, 이동, 지형/유닛 충돌, 목표 재지정, 연기·상태이상, 무장 갱신이 이어져 있다. 분리 후에도 World가 이 순서를 소유한다. 모듈별 스레드·각자의 타이머·각자의 난수 생성기를 추가하지 않는다.

### 그리기와 난수

`World::prepareDraw`는 읽기 전용 함수가 아니다. 탄환 효과·트레일을 만들면서 `random_`과 파티클 풀을 바꾼다. 이를 렌더러나 별도 GPU 스레드로 이동하면 전투 난수 순서가 달라질 수 있다. 호출 횟수·시점을 보존한 채 효과 갱신을 명시하고, 생성된 `Frame`만 렌더러에 넘긴다. `appendSprites`의 레이어·메시 삽입 순서도 그대로 유지한다.

### 충돌·표적·부품

미션 10·11 정체는 현재 코어에서 재현됐지만 포팅판과의 최종 원인 대조가 끝나지 않았다. 원본 지형 파일은 동일하며 충돌 셀 등록의 네 번째 검사 좌표에 차이가 있었다. 그 좌표만 바꿔도 미션 10 정체는 풀리지 않았다. 이 실패를 모듈화 전후 비교 사례로 보존한다.

표적 선택은 현재 `World::updateActor`가 담당하고 `WeaponController::tick`이 실제 사거리·발사 시점을 다시 판단한다. ‘적 사거리’를 한 숫자 접근자로 옮기는 것만으로 분리가 끝나지 않는다. 무장 회전 준비, 상태이상, 목표 유지, 경계 조건까지 함께 검사해야 한다.

`BattleModels::pose`와 `PartPose::muzzle`은 화면과 사격 양쪽에서 소비한다. 공용 조립 계산에 남기고 플랫폼 렌더러가 임의의 이미지 크기나 회전으로 포구를 다시 계산하지 않게 한다.

### 저장 완료와 비동기 플랫폼

현재 `HostServices::writeSave`는 bool을 즉시 반환한다. Apple은 `NSDataWritingAtomic` 결과를 반환한다. 웹의 IndexedDB 동기화와 Tauri 명령은 완료가 나중에 도착할 수 있다. 메모리에 썼다는 이유로 저장 성공을 먼저 반환하면 종료 직전 데이터가 유실될 수 있다.

플랫폼 확장 전에 저장 요청 ID·진행 중·완료·실패를 표현하는 작은 공용 계약을 설계한다. 바이트 포맷과 슬롯 구조는 그대로 두고, Apple도 같은 완료 응답을 보내도록 한다. 세션이 바뀐 뒤 늦게 도착한 완료를 새 세션에 적용하지 않는 검사도 필요하다. 이 작업은 **아직 구현되지 않았다.** 원본 저장 시점은 유지하고 발사·피격마다 저장하지 않는다. 웹 IDBFS의 명시적 동기화 방식은 [Emscripten 파일 시스템 문서](https://emscripten.org/docs/api_reference/Filesystem-API.html#FS.syncfs)를 따른다.

## 6. 오디오 교체와 모듈화

**원본 IPA의 API는 Apple OpenAL, 포팅판의 실제 구현은 OpenAL Soft, 현재 새 소스판은 Apple OpenAL 직접 연결이다.** API 이름이 같아도 구현과 컨텍스트 동작은 같지 않다. 포팅판에서 검증한 연결을 재사용하라는 지시에 따라 새 소스판도 OpenAL Soft를 연결하는 과제를 진행한다.

| 작업 | 영향을 받는 파일 | 보존·검사 |
|---|---|---|
| 라이브러리 연결 교체 | `MDAudio.mm/.h`, `Apple/iOS/project.yml`, `Modules/project.yml`, generated pbxproj, 새 OpenAL Soft 소스/빌드 명세 | Apple OpenAL 중복 링크 없음, arm64 기기/시뮬레이터 빌드, 실제 링크 목록·해시 |
| 재생 상태 공용화 | MDAudio의 buffer/source 생성·재사용·stop/play·music 상태 | 원작 음원 이름·소스 매핑·위치/볼륨·루프·재생 순서. 기존 source 15→Missile06.wav 대응도 검사 |
| PCM 입력 분리 | MDAudio의 `decode`, `Resources/Audio` | WAV/MP3 채널·주파수·샘플 수·길이 비교. 현재 Apple AudioToolbox 디코더를 Android/Web 공용이라 부르지 않는다 |
| OS 중단·복귀 | `MDApplication.mm`, MDAudio, `HostServices` | AVAudioSession·앱 활성 상태·미디어 재설정. 게임 입력을 오디오 플래그로 막지 않음 |
| 검사 | `Apple/Tests/AudioTests.mm`, `RendererTests.mm`, `MenuRenderTests.mm`, 무장 sweep | 실제 출력음·첫/반복/동시 발사·지연·전체 무음·다시 진입·종료 |

포팅판의 OpenAL Soft 버전·컴파일 옵션·출력 설정을 먼저 고정하고 재사용한다. HeavyMach의 31개 음원이나 mono WAV 전용 가정을 MachDefense에 넣지 않는다. MachDefense는 MP3 음악도 사용한다. 현재 코드의 `AL_NONE`, relative source, 위치값 제한이 원작과 일치하는지도 대조 대상이다. 이번 조사에서는 그 일치 여부를 확정하지 않았다.

Apple→OpenAL Soft 교체를 먼저 검증·커밋하고, 그다음 재생 로직과 플랫폼 디코더/출력 연결의 경계를 정리한다. 일반 사격 때 컨텍스트 해제·파일 읽기·디코딩·버퍼 생성이 없는지 검사한다. 수정 전 음원/상태/프레임 기록을 기준으로 비교한다.

네이티브 Apple·Android는 OpenAL Soft를 각 SDK로 빌드할 수 있는 구조를 따른다. 웹은 같은 라이브러리 바이너리를 가져가지 않는다. HeavyMach 웹처럼 OpenAL 호출을 Emscripten의 Web Audio 구현에 연결하는 후보를 검증한다. 이 차이는 브라우저 출력 어댑터에 둔다. [Emscripten의 음향 지원](https://emscripten.org/docs/porting/Audio.html).

## 7. 파일과 빌드 관리안

기존 저장소 하나를 유지한다. 아래는 **예정 구조**이며 아직 이동하지 않았다. 플랫폼별 게임 규칙 사본은 만들지 않는다.

```text
MachDefense/
  Sources/
    Core/
      Contracts/       # Vec2, 시간·난수, Game/Host/Resource/Frame 계약
      Data/            # 표·MCB·정의 데이터
      Simulation/      # 피해·이동·지형·표적·탄환·스폰·생산·경제·부품
      Session/         # 슬롯·게임 상태 전이·저장 포맷
      Presentation/    # 카메라·프레임·메뉴·HUD·버튼·글꼴
      Validation/      # 같은 게임 코드에 조건을 넣는 검사 기능
    Audio/             # 공용 재생 정책·PCM 계약·OpenAL 연결
    Platform/Apple/    # UIKit, AVAudioSession, Apple 파일/디코딩/GLES 컨텍스트
  Modules/
    project.yml        # Xcode 공용 대상
    core-sources.cmake # 공용 소스 목록(추가 예정)
    ThirdParty/        # stb, 검증할 OpenAL Soft와 출처/라이선스
  Resources/           # 원본 파일명·해시를 유지한 자원 한 벌
  Apple/
    MachDefense.xcworkspace
    iOS/               # iPhone·iPad 일반/치트 프로젝트
    Tests/
  Android/             # Gradle·NDK·JNI·Activity·입력·저장·EGL
  Web/                 # Emscripten·WebGL·음향·브라우저 입출력
  Distribution/PWA/    # 설치 안내·manifest·service worker·캐시 버전
  Desktop/Tauri/       # 웹 산출물 포장·Rust 저장·창·종료
  Tests/               # 공용 회귀·기준 입력·상태 비교
  docs/                # 요구·설계·원본 대조·검사·보고서
```

- `MachDefenseCore`는 한 공용 C++ 라이브러리로 유지한다. 우선 내부 폴더와 함수 책임을 정리한다. 별도 링크 대상은 오디오처럼 외부 라이브러리·SDK 의존성이 다른 경계에 둔다.
- 루트 CMake·Android·Web은 같은 `core-sources.cmake`를 소비한다. XcodeGen의 소스 범위와 이 목록을 대조해 누락·중복 컴파일을 검사한다. `#include`는 `Core/Simulation/...`처럼 책임이 보이게 바꾸며, 변경 시 테스트와 모든 소비자를 함께 고친다.
- 파일 이동 커밋은 `git mv`와 include/build 경로 수정에 한정한다. 계산식 변경·버그 수정·플랫폼 추가는 각각 별도 커밋이다. 이동 후에도 blame/log로 원래 코드를 추적할 수 있게 대규모 재포맷을 섞지 않는다.
- XcodeGen `.yml`을 수정한 뒤 생성된 `.xcodeproj`도 함께 갱신한다. framework/SPM으로 다시 포장하는 일은 현재 목표의 필수 조건이 아니다.
- `Resources`는 플랫폼별로 복사 관리하지 않는다. 빌드 시 번들·APK assets·웹 패키지로 포장하고 파일 목록·바이트 해시를 비교한다. IPA·추출 원본 실행 파일은 계속 저장소 밖에 둔다.
- 기본·치트의 bundle/application ID와 저장 경로를 분리한다. 현재 코어는 같은 library를 쓰며 `createGame(..., validation)`으로 기능을 구분한다. 모듈화 중 이를 조용히 다른 전투 코드로 갈라놓지 않는다.
- 빌드 산출물·DerivedData·에뮬레이터 저장·개인 시험 저장·원본 분석 파일은 `.local` 또는 외부 보관소에 둔다. 의존성 원본과 버전 잠금·빌드 스크립트는 추적한다.

## 8. 플랫폼별 연결 범위

| 플랫폼 | 공유하는 부분 | 플랫폼 안에 둘 코드 | 먼저 확인할 조건 |
|---|---|---|---|
| iPhone/iPad | 전체 게임 코어·C++ UI·자원·저장 포맷 | UIKit 창/터치/이름 입력, CADisplayLink, AVAudioSession, GLES 컨텍스트, 파일 저장 | 지금 동작·저장을 유지. 인게임 버튼을 UIKit으로 옮기지 않음 |
| Android 네이티브 | 같은 C++ 코어를 NDK 빌드, 같은 자원·오디오 정책 | Kotlin Activity, JNI, EGL/Surface, 오디오 focus, 파일/키보드 | Surface 재생성과 GL 컨텍스트 소멸을 구별. 게임 상태 유지와 GPU 자원 재업로드 검사 |
| 웹 | 같은 코어를 Wasm 빌드, Frame·버튼·메뉴·저장 바이트 | WebGL 렌더러, 브라우저 포인터/키보드, 오디오 시작 허용, IDBFS/저장 완료 | 예외 설정·파일 준비 완료 후 게임 생성, WebGL 소멸/복구, 저장 완료 전에 성공 표시하지 않음 |
| PWA | 웹 게임과 동일 산출물 | 설치·오프라인 자원 캐시·업데이트 | service worker 캐시와 사용자 저장 분리. 설치/재시작/오프라인/버전 갱신 검사 |
| Tauri Mac | 웹/Wasm 게임과 자원 | Rust 파일 저장·백업·종료 응답, 창/권한·단축키 연결 | 브라우저 캐시와 별도 앱 데이터 디렉터리 사용. 저장 중 종료·실패·재실행 검사 |

`ResourceAccess::read`와 이미지 크기 조회는 동기식이다. 웹에서는 필요한 패키지를 먼저 준비한 뒤 게임을 생성하면 기존 소비 코드를 유지할 수 있다. 현재 `MDRenderer::imageSize`는 파일 메타데이터를 읽으며, 플랫폼이 달라도 같은 크기값을 반환해야 한다.

WebGL은 현재 GLES1 호출을 그대로 실행하는 API가 아니다. HeavyMach의 WebGL 렌더러에서 텍스처·블렌드·메시 제출 방법을 참고하고 MachDefense의 `Frame` 계약에 맞게 구현한다. 게임 버튼은 C++에서 계속 그린다. PWA 설치 안내처럼 게임 바깥 화면과 실제 텍스트 입력만 웹 UI를 사용한다.

Tauri는 웹 게임의 정적 산출물을 포장할 수 있다. 같은 웹 코드를 유지하고 호스트 저장/종료 연결을 별도 폴더에 둔다. [Tauri 프런트엔드 구성](https://v2.tauri.app/start/frontend/).

## 9. 분리 전후 검증 계획

후속 사용자 지시를 반영한 [모듈화 전후 치트앱 검증 계획](module-validation-plan.md)을 실행 기준으로 사용한다. 원작·포팅판 대조 수정을 먼저 검증·커밋하고 그 결과를 모듈 분리 전후의 기준으로 둔다. MOD-01~15의 조건·판정과 일반 앱 회귀까지 포함한다.

기존 통과 기록은 기준 자료로 보존한다. 아래는 **이번 모듈화에서 실행할 검사**이며 이번 정적 조사 중 다시 돌렸다고 보고하지 않는다.

| 단계 | 검사 | 비교값 |
|---|---|---|
| 기준점 | 현재 일반/치트 빌드·시험 저장·자원 해시·동작 로그 보존 | 커밋·빌드 식별자·음향 라이브러리·실행 조건 |
| 데이터·기본 타입 분리 | SaveData/DataTable/Chunk/난수 검사 | 저장 바이트·슬롯 간격·표 값·난수열 |
| 피해·탄도 | Combat/Projectile/WeaponController/WorldSpecial 및 핵폭탄 sweep | 적/아군 HP·반경 경계·발사/명중 시점·재고·소멸 |
| 이동·지형·표적 | Battle/Bipod/Formation/World, 미션 4·10·11·밀집 터렛·화면 밖 표적 fixture | 매 틱 ActorId·좌표·목표·충돌·포신 각도·난수 소비 |
| 스폰·경제·저장 | Production/Economy/PlayerParts/Settlement/Loot/Campaign/GameSession | 등장 순서·생산 비용/시점·강화 효과·80미션 진행·저장 재읽기 |
| 화면·입력 | GameHUD/Menu/UpgradeDialog/SummaryDialog/Transition/FrameIO + Apple GPU 검사 | 원작 레이어·포구/부품·같은 화면 캡처·버튼 히트박스·회전·폰/패드 비율 |
| 음향 교체 | AudioTests, 전체 무장·생산/메뉴/승패 출력 녹음 | 첫/반복/동시 재생·클리핑·무음·음악 루프·중단/복귀·호출/프레임 ms |
| 모듈 통합 | iPhone/iPad 시뮬레이터 일반/치트 실행 | 프로필→생산→강화→전투→승리/실패→다음 미션→저장/재실행 |
| 성능·기기 | 시뮬레이터 전후 동일 조건, 최종 iPad Instruments·직접 출력 | 프레임 분포·메모리 증가·할당·오디오 대기·파일 접근 |
| 후속 플랫폼 | Android, 웹/PWA, Tauri에서 같은 fixture 및 실제 UI | 상태/저장 일치, 플랫폼 저장 완료, GPU/오디오 복구 |

코어 테스트의 가짜 Host는 실제 스피커 소리를 내지 않는다. 사운드 호출 횟수 검사와 실제 출력음 검사를 모두 둔다. 현재 개인 iPhone은 사용 중이므로 조작하지 않는다. 시뮬레이터로 주 검증을 하고 연결된 iPad에서 최종 확인한다.

같은 SDK·빌드에서는 정확한 상태 비교를 우선한다. 다른 CPU/컴파일러의 부동소수점 차이는 원작 계산 정밀도와 함께 조사하고, 오차 허용값을 늘려 이동·피해 차이를 숨기지 않는다. 전투와 시각 효과의 난수 순서를 함께 비교한다.

## 10. 브랜치와 작업 순서

사용자가 제안한 **현재 작업 정리 → develop 기준점 → 별도 모듈화 브랜치** 순서를 권장한다. 아직 develop 병합을 실행하지 않았다. `dev`로 부른 실제 브랜치명은 저장소 규칙의 `develop`이다.

1. 현재 기능 변경과 실패 증거·TODO를 커밋하고 원격 develop/main의 사용자 변경을 확인한다. 기존 `9e1121e`, `eb8b334`를 보존한다. 남은 지형·화면·음향 문제를 명시한 개발 기준점으로 develop에 병합한다. 원작 재현 완료 릴리스로 태그하지 않는다.
2. develop에서 `refactor/shared-core-audio` 같은 별도 브랜치를 만든다. 기준 빌드와 비교 결과를 고정한다.
3. 포팅판과 다른 렌더·입력·저장·생명주기 연결을 목록화한다. 오디오를 OpenAL Soft/원작 절차로 맞추고 독립 검사·커밋한다.
4. 기본 타입·데이터·파일 경로부터 정리한다. 다음으로 피해·무장, 이동·지형·표적, 생산·경제, 화면 구성 순서로 분리한다. 미해결 버그는 별도 수정 커밋으로 처리한다.
5. 전체 시뮬레이터 일반/치트 검사와 iPad 확인이 끝나면 feature → develop으로 병합한다.
6. 이후 `feat/android-native`, `feat/web-wasm`, `feat/pwa`, `feat/tauri-mac`처럼 플랫폼별 브랜치에서 확장한다. 공유 코어 변경이 필요하면 영향을 받는 기존 플랫폼 검사와 함께 별도 커밋으로 검토한다.

저장 완료 계약은 플랫폼 확장 전에 수정할 필요가 있다. 이 경계를 먼저 정리하면 이후 플랫폼 추가 때 게임 규칙을 고칠 필요를 줄일 수 있다.

## 11. 조사 범위와 남은 확인

확인한 범위는 58개 Core 파일의 헤더·의존 구조, 핵심 실행 함수, Apple 연결 5파일, CMake·XcodeGen·시험 구성, HeavyMach 모듈화 전후 커밋과 현재 Android/Web/Tauri 연결이다. 새 코드 실행·성능 측정·전체 원작 함수 대조를 이번 조사에서 완료한 것은 아니다.

원작 음향 위치/거리 모델·소스 매핑 전수 대조, OpenAL Soft 실제 버전/옵션 확정, 모듈별 게임 동등성 검사, Android·Web·Tauri 빌드는 후속 구현에서 확인한다. 지형·표적·HUD 미해결 항목은 [TODO](usability-todo.md)에 계속 남긴다. 제품 파일을 옮기기 전에 아래 파일 목록을 기준으로 누락을 검사한다.

## 12. 현재 Core 전체 파일 목록

아래는 현재 파일의 1차 이동 목적지다. Battle·World·Projectile·RenderTypes·GameSession처럼 여러 책임이 있는 파일은 해당 폴더로 이동한 뒤 위 모듈 표에 따라 함수별로 나눈다. 최종 함수 분할 전에는 한 파일을 여러 플랫폼에 복제하지 않는다. [해시·include 목록](evidence/module-inventory-20261105.json).

| 현재 파일 | 줄 수 | 1차 폴더 | 현재 내부 헤더 의존성 |
|---|---:|---|---|
| [Battle.cpp](../Sources/Core/Simulation/Spawning.cpp) | 378 | `Simulation` | `Battle.hpp` |
| [Battle.hpp](../Sources/Core/Simulation/Battle.hpp) | 165 | `Simulation` | `Chunk.hpp`, `DarwinRandom.hpp`, `DataTable.hpp`, `RenderTypes.hpp` |
| [Bipod.cpp](../Sources/Core/Simulation/Bipod.cpp) | 136 | `Simulation` | `Bipod.hpp` |
| [Bipod.hpp](../Sources/Core/Simulation/Bipod.hpp) | 45 | `Simulation` | `Battle.hpp` |
| [Button.hpp](../Sources/Core/Presentation/Button.hpp) | 36 | `Presentation` | `RenderTypes.hpp` |
| [Chunk.cpp](../Sources/Core/Data/Chunk.cpp) | 39 | `Data` | `Chunk.hpp` |
| [Chunk.hpp](../Sources/Core/Data/Chunk.hpp) | 21 | `Data` | 없음 |
| [Combat.cpp](../Sources/Core/Simulation/Combat.cpp) | 49 | `Simulation` | `Combat.hpp` |
| [Combat.hpp](../Sources/Core/Simulation/Combat.hpp) | 30 | `Simulation` | `DataTable.hpp` |
| [DarwinRandom.hpp](../Sources/Core/Contracts/DarwinRandom.hpp) | 31 | `Contracts` | 없음 |
| [DataTable.cpp](../Sources/Core/Data/DataTable.cpp) | 105 | `Data` | `DataTable.hpp` |
| [DataTable.hpp](../Sources/Core/Data/DataTable.hpp) | 33 | `Data` | 없음 |
| [Economy.cpp](../Sources/Core/Simulation/Economy.cpp) | 58 | `Simulation` | `Economy.hpp` |
| [Economy.hpp](../Sources/Core/Simulation/Economy.hpp) | 33 | `Simulation` | `DataTable.hpp`, `SaveData.hpp` |
| [Formation.cpp](../Sources/Core/Simulation/Formation.cpp) | 8 | `Simulation` | `Formation.hpp` |
| [Formation.hpp](../Sources/Core/Simulation/Formation.hpp) | 9 | `Simulation` | `RenderTypes.hpp` |
| [FrameLayout.hpp](../Sources/Core/Contracts/FrameLayout.hpp) | 25 | `Contracts` | `RenderTypes.hpp` |
| [Game.hpp](../Sources/Core/Contracts/Game.hpp) | 46 | `Contracts` | `RenderTypes.hpp` |
| [GameHUD.cpp](../Sources/Core/Presentation/GameHUD.cpp) | 270 | `Presentation` | `GameHUD.hpp` |
| [GameHUD.hpp](../Sources/Core/Presentation/GameHUD.hpp) | 80 | `Presentation` | `Button.hpp`, `Game.hpp`, `SpriteFont.hpp`, `FrameLayout.hpp` |
| [GameSession.cpp](../Sources/Core/Session/GameSession.cpp) | 330 | `Session` | `GameSession.hpp`, `Help.hpp`, `Menu.hpp`, `SummaryDialog.hpp`, `Transition.hpp`, `UpgradeDialog.hpp`, `World.hpp`, `ValidationTools.hpp` |
| [GameSession.hpp](../Sources/Core/Session/GameSession.hpp) | 22 | `Session` | `Game.hpp`, `SaveData.hpp` |
| [Help.hpp](../Sources/Core/Presentation/Help.hpp) | 29 | `Presentation` | `Game.hpp` |
| [Loot.cpp](../Sources/Core/Simulation/Loot.cpp) | 89 | `Simulation` | `Loot.hpp` |
| [Loot.hpp](../Sources/Core/Simulation/Loot.hpp) | 40 | `Simulation` | `DarwinRandom.hpp`, `DataTable.hpp`, `RenderTypes.hpp` |
| [Menu.cpp](../Sources/Core/Presentation/Menu.cpp) | 289 | `Presentation` | `Menu.hpp` |
| [Menu.hpp](../Sources/Core/Presentation/Menu.hpp) | 67 | `Presentation` | `Game.hpp`, `SaveData.hpp`, `SpriteFont.hpp`, `Button.hpp`, `DarwinRandom.hpp`, `FrameLayout.hpp` |
| [PlayerParts.cpp](../Sources/Core/Simulation/PlayerParts.cpp) | 39 | `Simulation` | `PlayerParts.hpp` |
| [PlayerParts.hpp](../Sources/Core/Simulation/PlayerParts.hpp) | 16 | `Simulation` | `Battle.hpp`, `Economy.hpp`, `Production.hpp` |
| [Production.cpp](../Sources/Core/Simulation/Production.cpp) | 50 | `Simulation` | `Production.hpp` |
| [Production.hpp](../Sources/Core/Simulation/Production.hpp) | 40 | `Simulation` | `Battle.hpp` |
| [Projectile.cpp](../Sources/Core/Simulation/Projectile.cpp) | 226 | `Simulation` | `Projectile.hpp` |
| [Projectile.hpp](../Sources/Core/Simulation/Projectile.hpp) | 98 | `Simulation` | `Combat.hpp`, `DarwinRandom.hpp`, `RenderTypes.hpp` |
| [RenderTypes.hpp](../Sources/Core/Contracts/RenderTypes.hpp) | 41 | `Contracts` | 없음 |
| [SaveData.cpp](../Sources/Core/Session/SaveData.cpp) | 158 | `Session` | `SaveData.hpp`, `DarwinRandom.hpp` |
| [SaveData.hpp](../Sources/Core/Session/SaveData.hpp) | 48 | `Session` | 없음 |
| [Settlement.cpp](../Sources/Core/Session/Settlement.cpp) | 77 | `Session` | `Settlement.hpp` |
| [Settlement.hpp](../Sources/Core/Session/Settlement.hpp) | 23 | `Session` | `Production.hpp`, `SaveData.hpp` |
| [SpecialAttack.cpp](../Sources/Core/Simulation/SpecialAttack.cpp) | 76 | `Simulation` | `SpecialAttack.hpp` |
| [SpecialAttack.hpp](../Sources/Core/Simulation/SpecialAttack.hpp) | 31 | `Simulation` | `Projectile.hpp` |
| [SpriteFont.hpp](../Sources/Core/Presentation/SpriteFont.hpp) | 42 | `Presentation` | `DataTable.hpp`, `RenderTypes.hpp` |
| [SummaryDialog.cpp](../Sources/Core/Presentation/SummaryDialog.cpp) | 51 | `Presentation` | `SummaryDialog.hpp` |
| [SummaryDialog.hpp](../Sources/Core/Presentation/SummaryDialog.hpp) | 28 | `Presentation` | `GameHUD.hpp`, `Settlement.hpp` |
| [Trail.cpp](../Sources/Core/Simulation/Trail.cpp) | 76 | `Simulation` | `Trail.hpp` |
| [Trail.hpp](../Sources/Core/Simulation/Trail.hpp) | 27 | `Simulation` | `Combat.hpp`, `DarwinRandom.hpp`, `RenderTypes.hpp` |
| [Transition.cpp](../Sources/Core/Presentation/Transition.cpp) | 70 | `Presentation` | `Transition.hpp` |
| [Transition.hpp](../Sources/Core/Presentation/Transition.hpp) | 32 | `Presentation` | `Game.hpp` |
| [UpgradeDialog.cpp](../Sources/Core/Presentation/UpgradeDialog.cpp) | 142 | `Presentation` | `UpgradeDialog.hpp` |
| [UpgradeDialog.hpp](../Sources/Core/Presentation/UpgradeDialog.hpp) | 52 | `Presentation` | `GameHUD.hpp` |
| [ValidationPanel.cpp](../Sources/Core/Validation/ValidationPanel.cpp) | 91 | `Validation` | `ValidationPanel.hpp` |
| [ValidationPanel.hpp](../Sources/Core/Validation/ValidationPanel.hpp) | 44 | `Validation` | `Button.hpp`, `Game.hpp`, `SpriteFont.hpp` |
| [ValidationTools.cpp](../Sources/Core/Validation/ValidationTools.cpp) | 118 | `Validation` | `ValidationTools.hpp`, `World.hpp` |
| [ValidationTools.hpp](../Sources/Core/Validation/ValidationTools.hpp) | 35 | `Validation` | `Game.hpp`, `Economy.hpp`, `Production.hpp`, `ValidationPanel.hpp` |
| [WeaponController.cpp](../Sources/Core/Simulation/WeaponController.cpp) | 69 | `Simulation` | `WeaponController.hpp` |
| [WeaponController.hpp](../Sources/Core/Simulation/WeaponController.hpp) | 48 | `Simulation` | `Combat.hpp`, `DarwinRandom.hpp`, `RenderTypes.hpp` |
| [World.cpp](../Sources/Core/Simulation/World.cpp) | 565 | `Simulation` | `World.hpp`, `Formation.hpp` |
| [World.hpp](../Sources/Core/Simulation/World.hpp) | 144 | `Simulation` | `Bipod.hpp`, `Game.hpp`, `PlayerParts.hpp`, `Loot.hpp`, `Projectile.hpp`, `SpecialAttack.hpp`, `Trail.hpp`, `WeaponController.hpp` |
| [WorldEffects.cpp](../Sources/Core/Simulation/WorldEffects.cpp) | 41 | `Simulation` | `World.hpp` |

## 13. 후속 기능: 긴급지원 조달의 모듈 배치

사용자가 추가로 지시한 긴급 재보급은 현재 동작하지 않는 스킬 버튼 오른쪽 인앱결제 버튼을 대체한다. 결제·광고 API를 붙이지 않는다. 이 절은 신규 기능의 설계 방향이며 구현 완료 내용이 아니다.

| 책임 | 배치안 | 처리 |
|---|---|---|
| 지원 규칙·상태 | `Core/Simulation/EmergencySupport` 신규 후보 | 게임 시작부터 크레딧 30초·셀 45초·혼합 60초 활성화. 지급 가능·수령 상태와 중복 지급 방지 |
| 자원 지급 | 기존 Economy·BattleClock·SaveData를 사용하는 World/Session 연결 | 크레딧과 셀의 기존 갱신 경로·용량 규칙 사용. 지급량과 상한 정책은 후속 기획에서 확정 |
| UI | 공용 GameHUD·Button·SpriteFont, 필요 시 공용 지원 창 | 모든 문구 영문. 스킬 버튼과 같은 글로우, 원작 에셋·폰트·커서로 타이핑 효과 |
| 메시지 | 공용 데이터/텍스트 정의 | J·B·P 이니셜의 고위 군인 직함과 2011년 전장에 돌아온 플레이어를 치하하는 각기 다른 격려문 |
| 플랫폼 | 기존 Frame 렌더·입력만 사용 | UIKit·Android XML/Kotlin·DOM으로 인게임 지원 버튼·창을 새로 만들지 않음 |

게임 진행 시간으로 임계값 도달 여부를 확인하고 상태 전환 때만 UI 상태를 바꾼다. 별도 OS 타이머·스레드·네트워크·초당 파일 저장을 추가하지 않는다. 남은 시간 숫자를 표시한다면 정수 초가 바뀔 때만 문자열을 갱신한다. 타이핑 효과는 미리 준비한 글리프의 표시 개수를 기존 프레임 갱신에서 바꾼다. 글자마다 이미지나 폰트를 다시 만들지 않는다.

수령 가능한 항목이 하나라도 있으면 버튼에 원작 스킬 선택과 같은 글로우를 주고, 없으면 끈다. 현재 스킬 글로우의 원본 대조 과제 MD-AV-09가 남아 있으므로 잘못된 현행 효과를 그대로 기준으로 고정하지 않는다. 원작 발광 레이어·타일·합성·지속 상태를 먼저 확인한다.

지급량, 수령 후 재활성화 여부, 재도전·저장 후 재진입 시 타이머/수령 상태의 수명은 아직 확정하지 않았다. 시간 계산은 일시정지 때 멈추는 기존 전투 시간을 사용하는 안을 우선한다. 신규 상태가 저장에 필요해지면 기존 저장 슬롯의 빈 바이트에 임의로 끼워 넣지 않고 별도 확장 포맷과 복구 절차를 설계한다.

검사는 30·45·60초 직전/도달/직후, 지원창을 닫고 다시 열기, 빠른 중복 터치, 여러 항목 수령, 크레딧·셀 변화, 글로우 켜짐/꺼짐, 정지·복귀·재진입을 포함한다. 지원 기능 전후 같은 동시 사격 조건의 프레임 시간·메모리 할당·파일 접근을 비교한다. 타이머 갱신만으로 게임 저장 쓰기가 발생하지 않아야 한다.

2026-11-05 사용자 정정: 긴급지원은30·45·60초마다 반복 제공한다. 미션별1회 지급 제한을 두지 않는다. [현행 반복 지원 계획](emergency-resupply-plan.md)의 금액·수령·저장 정책이 위 미확정안을 구체화한다.
