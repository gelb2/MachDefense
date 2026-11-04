# 좌표·입력

논리 해상도, 기기별 화면, 원점, 스케일, 터치 변환, 카메라, 파츠 피벗을 원본에서 확인한다. 일반판과 WeMade 판본의 세로 화면 표시는 원본 명세에 있다. HeavyMach 1의 가로 480×320을 적용하지 않는다. 렌더와 히트박스는 같은 변환을 사용하며 화면 가장자리와 회전 입력을 검사한다.

현재 소스판의 논리 화면은 320×480이다. [World](../Sources/Core/World.cpp)는 위쪽이 양수인 월드 좌표를 사용하고 worldToScreen/screenToWorld에서 화면 좌표와 변환한다. [GameHUD](../Sources/Core/GameHUD.cpp)는 원작 UI 좌표를 화면 위쪽 원점으로 변환한다. [Apple 렌더러](../Sources/Platform/Apple/MDRenderer.mm)는 화면에 맞춰 비율을 유지한다. GameSession::resize는 아직 확장을 적용하지 않는다. iPhone·iPad 화면 확장은 [현재 작업 순서](source-reimplementation-plan.md)의 마지막 계획 과제다.

현재 포팅 옵션은 iPhone 모드다. iPad/HD 에셋 선택과 배율은 [MD-ART-01](port-observations-20261104.md)에서 확인한다.

[유니버설 에셋 계획](universal-assets-plan.md): 고해상도 에셋 채택 시 픽셀 크기를 월드 크기로 사용하지 않는다. 원본 논리 크기·파츠 피벗·충돌과 화면/입력 변환을 보존한다.
