# Apple

`MachDefense.xcworkspace`는 iPhone·iPad 유니버설 앱과 공용 코어·자원 프로젝트를 연결한다. 일반 앱은 `com.gelb2.machdefense.source`, 검사 앱은 `com.gelb2.machdefense.source.cheat`를 사용한다. 포팅판의 앱·저장과 분리된다.

Apple Silicon Mac, Xcode와 iOS Simulator, Python 3, CMake 3.20 이상을 사용한다. 첫 빌드에서 `tools/build_openal.py`가 포함된 OpenAL Soft 소스를 컴파일한다. Homebrew 사용자는 `brew install cmake`로 준비한다. 기존 프로젝트를 열어 빌드할 때 XcodeGen은 필요하지 않다.

프로젝트 재생성:

```sh
Apple/generate-projects.sh
```

Xcode에서 workspace를 열고 `MachDefense` 또는 `MachDefenseCheat` 스킴과 iPhone/iPad 시뮬레이터를 선택한다. 제품은 `Sources/Core/Contracts/Game.hpp`의 `createGame` 구현과 준비된 `Resources`를 소비한다. IPA·호환 실행기는 빌드 입력에 없다.

`Sources/Platform/Apple`은 OpenGL ES 1.1 텍스처 렌더링, 입력 좌표 변환, OpenAL 버퍼 재사용, UIKit 창·생명주기·이름 입력, 명시적 원자적 파일 저장을 담당한다. 게임 버튼과 글자는 코어가 만든 스프라이트 목록으로 그린다. UIKit은 게임 버튼을 만들지 않는다.

효과음은 포팅판에서 사용한 OpenAL Soft 1.22.2를 `MachDefenseAudio` 모듈에 연결한다. Apple 시스템 OpenAL.framework는 링크하지 않는다. 원본의 PCM 형식과 위치 감쇠를 사용한다.

`MachDefenseRendererTests` 스킴은 시뮬레이터 OpenGL ES의 이미지 방향·크롭·알파·클립·입력 변환·텍스처 재사용을 검사한다. MenuRenderTests는 공용 `Menu`와 실제 자원을 연결해 타이틀, 영문 프로필 생성, 스테이지 선택을 렌더하고 PNG를 남긴다. AudioTests는 WAV 효과음 77개, 반복 효과음, 배경음악 3곡의 루프 재생·중단·재개를 검사한다.

배경음악 MP3 3곡은 원본과 같은 AudioFile/AudioQueue로 재생한다. 같은 곡을 요청하면 재시작하지 않으며, 앱이 비활성화되면 일시정지한다. 오디오 장치가 재생성되면 곡명과 음량을 복원한다.

iPhone·iPad 화면 확장은 FrameLayout·BattleGeometry·GameHUD에서 처리한다. 기기별 화면·입력 변환은 [좌표계 정책](../docs/coordinate-policy.md)을 따른다. 오른쪽 HUD의 iPhone 회색 확장과 타이틀 가장자리 개선은 남아 있다.

iPad 시뮬레이터의 실행 지연을 조사한 프로세스 샘플에서는 `GLRendererFloat`가 픽셀을 CPU에서 그렸다. 실제 iPad 성능은 기기에서 별도로 측정한다.

검사 앱의 `TEST`는 같은 게임 코어와 저장 구조를 사용한다. 빈 검사 슬롯을 만든 뒤 미션·크레딧·강화·장비·특수공격·기지 파괴 조건을 선택한다. 일반 앱에는 이 진입 버튼이 없다. 직접 입력 검사는 Device Hub에서 화면을 확인하며 진행하고, 각 결과를 `xcrun simctl io <기기 UUID> screenshot <PNG 경로>`로 남긴다. 저장을 확인한 뒤 검사 앱만 종료하고 홈 화면을 확인한다.

렌더러의 `lastFrameMetrics`는 마지막 프레임의 스프라이트 수, 드로우 수, 텍스처·필터·혼합·클립 설정 수와 제출·표시 호출의 CPU 소요 시간을 메모리에 보관한다. 파일에는 쓰지 않는다. 이 시간에는 GPU 완료를 기다리는 별도 동기화를 넣지 않았다. 원작 탄환 궤적은 `TextureMesh.beforeSpriteIndex` 순서에 맞춰 스프라이트 사이에 삼각형 띠로 그린다. `RendererTests`가 궤적과 전경의 겹침 순서 및 색 상태 복원을 검사한다.

## 빠른 기능 자동검사

```sh
python3 tools/run_apple_sweep.py --simulator <시뮬레이터-UDID>
```

저장소 루트에서 실행한다. 80미션 결과·저장, 전체 강화 단계 구매, 일반 무장 211개, 생산·특수공격을 실제 Apple 렌더러·오디오에 연결한다. 1,800프레임 CADisplayLink 검사도 포함한다. `--mode soak`는 10분 전투·20회 진입·오디오/렌더 복귀를 검사한다. [기존 검사](../docs/evidence/apple-automated-sweep-20261104.md), [전체 강화·무장·기기 측정](../docs/evidence/full-upgrade-weapon-device-20261104.md).

실기기는 Xcode에서 `MachDefenseRendererTests`와 연결된 기기를 선택한 뒤 Test를 실행한다. 기존 치트 앱이 XCTest 호스트이며, 검사 중에는 일반 게임 화면의 오디오를 중복 초기화하지 않는다. 저장은 임시 디렉터리를 사용한다. 검사를 마친 뒤 배포할 앱은 Clean Build Folder 후 `MachDefense`·`MachDefenseCheat` 스킴으로 각각 빌드한다. 테스트 플러그인이 없는 최종 앱을 설치한다.
