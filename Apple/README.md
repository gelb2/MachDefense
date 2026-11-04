# Apple

`MachDefense.xcworkspace`는 iPhone·iPad 유니버설 앱과 공용 코어·자원 프로젝트를 연결한다. 일반 앱은 `com.gelb2.machdefense.source`, 검사 앱은 `com.gelb2.machdefense.source.cheat`를 사용한다. 포팅판의 앱·저장과 분리된다.

프로젝트 재생성:

```sh
Apple/generate-projects.sh
```

Xcode에서 workspace를 열고 `MachDefense` 또는 `MachDefenseCheat` 스킴과 iPhone/iPad 시뮬레이터를 선택한다. 제품은 `Sources/Core/Game.hpp`의 `createGame` 구현과 준비된 `Resources`를 소비한다. IPA·호환 실행기는 빌드 입력에 없다.

`Sources/Platform/Apple`은 OpenGL ES 1.1 텍스처 렌더링, 입력 좌표 변환, OpenAL 버퍼 재사용, UIKit 창·생명주기·이름 입력, 명시적 원자적 파일 저장을 담당한다. 게임 버튼과 글자는 코어가 만든 스프라이트 목록으로 그린다. UIKit은 게임 버튼을 만들지 않는다.

현재 Apple 음향 백엔드는 시스템 `OpenAL.framework`다. HeavyMach 1의 OpenAL Soft 배포본과는 구분한다. 원작의 음향 재생·감쇠 규칙은 게임 코어와 함께 대조한다.

`MachDefenseRendererTests` 스킴은 시뮬레이터 OpenGL ES의 이미지 방향·크롭·알파·클립·입력 변환·텍스처 재사용을 검사한다. MenuRenderTests는 공용 `Menu`와 실제 자원을 연결해 타이틀, 영문 프로필 생성, 스테이지 선택을 렌더하고 PNG를 남긴다. AudioTests는 음원 80개, 반복 효과음, 배경음악 3곡의 루프 재생·중단·재개를 검사한다.

배경음악은 효과음과 별도 OpenAL source로 재생한다. 같은 곡을 요청하면 재시작하지 않으며, 앱이 비활성화되면 일시정지한다. 오디오 장치가 재생성되면 곡명과 음량을 복원한다.

현재 화면은 원작 iPhone의 320×480 논리 좌표를 유지한다. iPhone·iPad에서 같은 좌표계로 입력과 렌더링을 검사하며, 기기 화면 비율에 따라 여백이 생긴다. 전체화면 및 iPad 전용 배치는 후속 작업이다.

iPad 시뮬레이터의 실행 지연을 조사한 프로세스 샘플에서는 `GLRendererFloat`가 픽셀을 CPU에서 그렸다. 실제 iPad 성능은 기기에서 별도로 측정한다.

검사 앱의 `TEST`는 같은 게임 코어와 저장 구조를 사용한다. 빈 검사 슬롯을 만든 뒤 미션·크레딧·강화·장비·특수공격·기지 파괴 조건을 선택한다. 일반 앱에는 이 진입 버튼이 없다. 직접 입력 검사는 Device Hub에서 화면을 확인하며 진행하고, 각 결과를 `xcrun simctl io <기기 UUID> screenshot <PNG 경로>`로 남긴다. 저장을 확인한 뒤 검사 앱만 종료하고 홈 화면을 확인한다.

렌더러의 `lastFrameMetrics`는 마지막 프레임의 스프라이트 수, 드로우 수, 텍스처·필터·혼합·클립 설정 수와 제출·표시 호출의 CPU 소요 시간을 메모리에 보관한다. 파일에는 쓰지 않는다. 이 시간에는 GPU 완료를 기다리는 별도 동기화를 넣지 않았다. 원작 탄환 궤적은 `TextureMesh.beforeSpriteIndex` 순서에 맞춰 스프라이트 사이에 삼각형 띠로 그린다. `RendererTests`가 궤적과 전경의 겹침 순서 및 색 상태 복원을 검사한다.

## 빠른 기능 자동검사

```sh
python3 tools/run_apple_sweep.py --simulator <시뮬레이터-UDID>
```

저장소 루트에서 실행한다. 80미션 결과·저장, 포팅판 관측값의 강화 구매, 생산·특수공격을 실제 Apple 렌더러·오디오에 연결한다. [검사 범위와 결과](../docs/evidence/apple-automated-sweep-20261104.md). 실기기 성능 측정은 별도로 실행한다.
