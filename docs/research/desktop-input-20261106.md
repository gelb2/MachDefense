# Mac 스크롤과 키보드 대응 조사

2026-11-06. **MD-DESKTOP-INPUT-01. 조사·TODO이며 구현하지 않았다.** 기존 Apple·Android·공용 게임 코드와 터치 조작을 유지한다. 이번 사용자 지시는 Mac 입력 차이와 HeavyMach 1 방식의 키보드 대응 가능성을 조사하는 범위다. 공용 코드 변경 승인은 없다.

## 사용자 관측과 확인 범위

| 구분 | 확인한 내용 |
|---|---|
| 사용자 관측 | iPhone과 Galaxy에서는 손가락 스크롤이 정상이다. |
| 사용자 관측 | iPad PWA에서도 통상 기대한 스크롤이 정상이다. 손가락·마우스·키보드·트랙패드 중 어떤 장치를 썼는지는 확인하지 않았다. |
| 사용자 요청 | Mac에서 다른 이유를 연구 TODO로 남기고 HeavyMach 1처럼 키보드로 대응할 수 있는지 확인한다. |
| 소스 확인 | MachDefense 웹에는 `pointerdown/move/up/cancel` 연결이 있다. `wheel`과 게임용 `keydown/keyup` 연결은 없다. |
| 미확인 | 같은 Mac·브라우저·전투 장면에서 클릭 후 드래그와 두 손가락 스크롤의 이벤트를 나란히 기록하지 않았다. iPad에서 같은 장치와 제스처로 비교하지 않았다. |

확정한 구현 누락은 **웹 실행기의 wheel 입력 미연결**이다. iPad 성공을 트랙패드 성공으로 확대하거나, 모바일 터치 게임 로직의 결함으로 해석하지 않는다. 실제 Mac 제스처의 이벤트 기록을 확보하기 전에는 모든 Mac 입력 차이의 원인을 확정하지 않는다.

## 손가락 드래그와 휠이 갈라지는 지점

현재 [app.js](../../Distribution/Web/app.js)는 포인터를 누를 때 캡처하고, 캡처 중 이동만 `md_touch`에 전달한다. [Main.cpp](../../Web/src/Main.cpp)의 `md_touch`는 정규화 좌표를 게임 좌표로 바꾸고 `GameSession::touch`를 호출한다. [WorldPresentation.cpp](../../Sources/Core/Presentation/WorldPresentation.cpp)의 `World::touch`는 전장의 드래그를 카메라 이동으로 처리한다. 따라서 기존 손가락 드래그와 마우스 버튼을 누른 드래그는 이 경로를 사용할 수 있다. 후자의 실제 Mac 동작은 이번 조사에서 검사하지 않았다.

포인터 위치·누름을 전달하는 이벤트와 휠의 스크롤 변화량은 별도 입력이다. 휠을 지원하는 마우스·터치패드의 변화량은 `WheelEvent`의 delta와 단위로 표현된다. 포인터 리스너만 등록했다고 이 값을 게임 카메라에 전달하지는 않는다. [W3C Pointer Events](https://www.w3.org/TR/pointerevents3/#pointerevent-interface), [W3C UI Events의 delta 설명](https://www.w3.org/TR/uievents/#delta)을 참고했다. 특정 iPad·Mac 장치가 실제 어떤 이벤트를 보냈는지는 별도 관측 대상이다.

휠을 가짜 누름·이동·해제로 바꿔 기존 `md_touch`에 넣는 방법은 채택하지 않는다. [GameSession.cpp](../../Sources/Core/Session/GameSession.cpp)의 입력 순서는 메뉴·팝업·HUD를 먼저 처리한다. [GameHUD.cpp](../../Sources/Core/Presentation/GameHUD.cpp)는 특수공격을 누름 순간에 선택하며 일시정지 음량 슬라이더는 이동 중 값을 바꾼다. 마지막 취소 이벤트가 앞선 변경을 되돌리지 않는다. 현재 정식판의 `validationWorld()`는 `nullptr`을 반환하므로 이 검사 API를 일반 카메라 접근에 사용할 수도 없다.

## HeavyMach 1의 실제 웹 구현

비교한 로컬 사본은 `HeavyMach/build/pr24-review`, 커밋은 `e73a77399a2bdff76f4a5a9383fb199abaa5b17d`다. 아래는 코드 읽기 결과다. 이번에 HeavyMach 앱을 새로 실행하거나 키보드를 검사하지 않았다.

| 실제 코드 | 처리 내용 | MachDefense에 참고할 범위 |
|---|---|---|
| [Web/shell.html 48–58, 107–118행](https://github.com/gelb2/HeavyMach/blob/e73a77399a2bdff76f4a5a9383fb199abaa5b17d/Web/shell.html#L48) | `KeyboardEvent.code`를 HID 키로 바꾼다. 입력창은 제외하고 Meta/Ctrl 조합은 게임 입력을 해제한다. 반복 keydown은 게임에 전달하지 않는다. | 물리 키와 액션 분리, 브라우저 단축키 보존, 텍스트 입력 제외. |
| [Sources/Web/WebMain.cpp 39–44행](https://github.com/gelb2/HeavyMach/blob/e73a77399a2bdff76f4a5a9383fb199abaa5b17d/Sources/Web/WebMain.cpp#L39) | 포인터·키·키보드 취소·활성 상태를 별도 API로 전달한다. | 키보드를 가짜 포인터로 만들지 않는 연결 구조. |
| [Sources/Core/KeyboardInput.hpp](https://github.com/gelb2/HeavyMach/blob/e73a77399a2bdff76f4a5a9383fb199abaa5b17d/Sources/Core/KeyboardInput.hpp) | 키 유지 상태·액션 배정·반대 방향 중립·전체 해제를 관리한다. | 키 상태 관리 원칙. HeavyMach 전용 이동·조준 수치와 기본 키 전체를 복사하지 않는다. |
| [Sources/Web/WebGameView.cpp 372–445행](https://github.com/gelb2/HeavyMach/blob/e73a77399a2bdff76f4a5a9383fb199abaa5b17d/Sources/Web/WebGameView.cpp#L372) | 모달·설정·메뉴·전투를 나누고 실제 메뉴 액션과 전투 API를 호출한다. 유지 조준은 게임 tick에서 갱신한다. | 현재 화면과 입력 가능 상태를 게임이 판단하는 구조. |
| [Web/shell.html 178–193행](https://github.com/gelb2/HeavyMach/blob/e73a77399a2bdff76f4a5a9383fb199abaa5b17d/Web/shell.html#L178), [WebGameView.cpp 249–260행](https://github.com/gelb2/HeavyMach/blob/e73a77399a2bdff76f4a5a9383fb199abaa5b17d/Sources/Web/WebGameView.cpp#L249) | blur·비활성에서 누른 키와 사격을 해제하고 게임을 정지한다. | 키를 뗀 이벤트가 누락돼도 입력이 남지 않도록 하는 처리. |

HeavyMach 1은 이미 있는 게임 액션·메뉴 탐색 API와 키 상태 모듈을 연결한다. MachDefense에는 같은 입력 계약이 없다. HeavyMach의 JS 키 리스너를 옮기는 것만으로 지원이 완성되지는 않는다.

## 키보드 대응 후보와 변경 경계

전장 위·아래 이동에 `↑/↓`와 `W/S`를 배정하는 안을 먼저 검토한다. 원작 버튼 그림·배치·터치 동작은 유지한다. 생산·강화·특수공격 단축키와 전체 메뉴 탐색은 별도 후속 범위다. 카메라 키만 추가한 결과를 키보드 전 조작 지원으로 표현하지 않는다.

| 범위 | 기존 게임 코드를 바꾸지 않고 할 수 있는 일 | 추가 계약이 필요한 일 |
|---|---|---|
| 입력 조사 | Web에서 pointer/wheel/key 이벤트 종류·단위·포커스를 기록하고 기기별로 비교한다. 기존 터치 API와 프레임 루프는 그대로 쓴다. | 없음. 진단 기록에 복원 코드·저장 내용을 넣지 않는다. |
| 브라우저 입력 처리 | Web 전용 키 상태 관리, 텍스트 입력·OS 단축키 제외, 반복 제거와 해제 처리를 준비할 수 있다. | 이 처리만으로 게임 카메라가 움직이지 않는다. |
| 전장 카메라 | 기존 카메라 배율·상하 제한·표시 갱신을 재사용할 수 있다. | 정식판에서도 호출 가능한 `GameSession` 카메라 입력 API와 `World`의 카메라 전용 이동 함수가 필요하다. 현재 공개 API에는 없다. |
| 일시정지·메뉴 선택 | 원작 버튼과 액션 처리를 재사용하는 설계가 가능하다. | 현재 화면의 입력 가능 상태, 포커스 이동, 버튼 액션을 안전하게 요청하는 공통 계약이 필요하다. 고정 좌표 클릭으로 대신하지 않는다. |

카메라 전용 API의 최소 후보는 `GameSession.hpp/.cpp`와 `World.hpp/WorldPresentation.cpp`에 추가하는 방식이다. `GameSession`은 활성·전투·전환·모달·일시정지·진행 중 터치를 검사하고, `World`는 유한 변화량·배율·경계를 처리한다. 기존 터치·버튼·저장 경로를 바꾸지 않고 새 입력만 연결하는 설계다. 그래도 Apple·Android가 소비하는 공용 파일 변경에 해당하므로 **현재 조건에서는 구현하지 않는다.** 일반판을 치트로 만들거나 private 상태를 우회해 이 제한을 피하지 않는다.

후속 설계에는 다음 조건을 포함한다.

- `keydown`은 눌림 상태만 바꾼다. OS repeat 횟수로 이동량을 계산하지 않는다. 기존 tick 안에서 일정한 이동을 적용하며 입력 때문에 게임 tick을 추가하지 않는다. 반대 키 동시 입력은 중립이다.
- 복원 코드·프로필 이름·`input`·`textarea`·`contenteditable`·문자 조합 입력 중에는 게임 키를 받지 않는다. 입력창으로 포커스가 이동할 때 이전 유지 키도 해제한다.
- Meta/Ctrl/Alt 조합과 브라우저·OS 단축키를 보존한다. 게임이 실제 소비한 키만 기본 동작을 막는다. 메뉴와 전투에서 방향키가 충돌하지 않도록 현재 화면에 따라 한 곳에 전달한다.
- pause·모달·화면 전환·blur·숨김·pagehide·렌더 컨텍스트 손실에서 유지 입력을 해제한다. 복귀 후 자동으로 이동을 재개하지 않는다. 키를 다시 눌러야 움직이게 한다.
- 휠 대응은 별도로 delta 단위·방향·가속·관성·확대 제스처를 비교한다. 터치 드래그와 키 유지 입력이 동시에 카메라를 소유하지 않도록 우선순위를 정한다.

## 남은 조사와 검사

1. Mac에서 클릭 후 드래그, 마우스 휠, 트랙패드 두 손가락 스크롤을 같은 전투 화면에서 비교한다. 브라우저·설치형 PWA 여부와 실제 이벤트를 남긴다.
2. iPad 성공 사례의 입력장치와 제스처를 확인하고, 같은 종류의 Mac 입력과 비교한다. 현재 사용자 관측은 유지한다.
3. `↑/↓`, `W/S` 카메라 이동 범위와 메뉴·일시정지 키 범위를 정한다. 공통 입력 API의 추가 범위가 정해지기 전에는 구현하지 않는다.
4. 구현 단계에서는 정식·치트, 화면 비율, 상하 경계, 짧은 누름·유지·반대 키·blur, 텍스트 입력·복원, HUD 위 입력·특수공격 선택·음량을 검사한다. Apple·Android 기존 터치와 저장 보존도 확인한다.

이번 결과는 소스 대조와 연구 TODO 기록이다. 키보드·휠 구현, 앱 빌드·설치·배포, 실제 키 입력 검사는 수행하지 않았다. [TODO](../usability-todo.md)의 MD-DESKTOP-INPUT-01에서 이어간다.
