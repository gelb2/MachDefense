# iPhone 우측 HUD 상단 — 2026-11-06

## 변경

사용자는 월드 표시 범위 문제를 TODO로 유지하고, iPhone의 HUD 위 빈 부분을 원본 에셋으로 가리도록 지시했다. iPad에는 시각적 변경이 없어야 한다.

`GameUI_MachSlot.png`는 256×1024다. GameHUD가 절반 크기로 그리므로 위쪽 좌표는 `layout.top - 28`이다. iPhone의 큰 안전 영역에서는 위에 빈 줄이 생긴다. 원본 CgBI PNG를 기존 자원 준비 절차로 변환한 픽셀과 현재 PNG를 대조해 일치함을 확인했다. 회색 판은 원본 에셋에 포함된다.

[GameHUD.cpp](../../Sources/Core/Presentation/GameHUD.cpp)에서 빈 부분이 있을 때만 같은 이미지의 상단 2px를 이어 그린다. 셀·생산 버튼·터치 위치는 유지한다. 이미지 파일, 월드·전투·저장·플랫폼 호스트 코드는 변경하지 않았다.

## 검사

- [GameHUDTests](../../Tests/GameHUDTests.cpp): 변경 전 상단 덮임 검사 실패를 재현했다. 변경 후 iPhone 2종·iPad 2종·Galaxy·320×480 기준의 덮임, 셀 강화·생산 입력 검사가 통과했다. iPad·Galaxy·기준 크기는 추가 상단 스프라이트를 만들지 않는다.
- `testExpandedViewportAndInputs`: 수정 전·후 각각 iPhone Air 시뮬레이터 iOS 26.5에서 통과. 5개 화면·안전 영역 조건의 600틱 전투·저장·난수 일치와 메뉴·기지 강화·포탑 배치·레이더·복귀 입력을 검사했다.
- iPhone Air의 전투 화면 3장은 우측 위 70×19픽셀만 변경됐다. 제목·프로필·미션 화면은 픽셀이 같다.
- iPad 2종의 우측 HUD는 전부 픽셀이 같다. 제목·프로필·미션·카메라 아래쪽·기지 강화 화면 10장도 전체 픽셀이 같다. 카메라 위쪽의 월드에는 각각 6개·3개 픽셀에서 색상 채널 1/255 차이가 있었다. HUD의 위치·그림 차이는 없었다.

로컬 증거: 이 작업 트리의 `.local/hud-top/before.xcresult`, `after.xcresult`, `before-captures`, `after-captures`, `pixel-diff.json`, `ipad-pixel-detail.json`, `red.log`, `device/before.sav`.

## iPhone 설치

Xcode GUI에서 기존 개발팀으로 Release 정식 앱을 서명·빌드했다. `codesign --verify --deep --strict`를 통과했고 iPhone Air(iOS 27.2)에 무선으로 설치했다. 일반 앱 번들 ID는 `com.gelb2.machdefense.source`다. 타이틀 실행을 확인했다.

설치 전·후 저장 파일 SHA-256은 `a00d6e45af4d2d629a83dad9c2aa57bb8f16c1e7279bf86e13005a453dff285b`로 같았다. 게임 저장을 덮어쓰거나 삭제하지 않았다. 실제 iPad의 앱은 재설치하지 않았다.

Device Hub에서 화면과 홈 버튼은 동작했지만 게임 화면 터치가 진입으로 이어지지 않았다. 창 전환·재연결 후에도 같아 실기기 생산·메뉴 직접 검사는 미완료다. 위의 입력 검사는 C++ 및 시뮬레이터 결과다. 관련 로그는 `.local/hud-top/device/`에 보존했다.

## 남은 과제

월드 배경 끝과 유닛 표시 범위의 차이는 [MD-WORLD-HUD-TOP-01](../usability-todo.md#iphone-우측-hud-상단--2026-11-06)로 남겼다. 이번 작업은 원본 HUD로 iPhone 상단의 노출 부분을 덮는다.

## 사용자 확인과 병합 준비

2026-11-06 사용자가 “아이폰 에셋으로 덮힌거 확인 했다”라고 설치본의 표시를 확인하고 병합을 지시했다. HUD 표시 수정은 사용자 확인까지 완료했다. 원격 터치 도구를 통한 생산·메뉴 직접 검사는 앞서 기록한 미완료 상태를 유지한다. 앱 종료 뒤 저장 파일도 설치 전 해시와 같았고 홈 화면 복귀를 확인했다.
