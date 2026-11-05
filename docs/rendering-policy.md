# 그래픽 렌더링

원본 텍스처 크롭·알파·필터·레이어·부품 조립·애니메이션·버튼 상태를 분석한다. 게임이 실행된 뒤 버튼은 원작 에셋과 게임 렌더·입력 방식으로 구현한다. UIKit/Android View/DOM으로 게임 버튼을 분리하지 않는다. 텍스트 입력 등 원작이 해결하지 못한 OS 기능만 플랫폼 경계에 둔다. GPU context 소실 때 자원을 복원하며 게임 세션을 초기화하지 않는다.

현재 소스판은 [World](../Sources/Core/Simulation/World.cpp)의 부품·투사체·효과와 [GameHUD](../Sources/Core/Presentation/GameHUD.cpp)의 원본 UI 에셋을 공용 Frame으로 모은다. [Apple 렌더러](../Sources/Platform/Apple/MDRenderer.mm)가 GPU에 제출한다. 텍스처는 재사용한다. 처음 쓰는 자원 준비 지연은 [성능 후속 과제](usability-todo.md)에 남아 있다. 원본과의 레이어·방향·레이더 대조 결과는 [비교 기록](evidence/port-source-parity-20261104.md)에 추가한다.

현재 포팅판의 적·아군 유닛 자글거림은 [MD-ART-01](port-observations-20261104.md)로 추적한다. 판본별 에셋·기기 선택·합성 해상도·필터를 대조한 뒤 재개발 렌더 기준을 확정한다.

[공용 에셋 계획](universal-assets-plan.md)에 따라 판본별 원본 품질을 비교한다. 채택 원본·크롭·피벗·논리 크기를 한 명세에서 관리하고 플랫폼별 출력은 그 명세에서 생성한다.

## 텍스처 업로드 — 2026-11-05

준비된 RGBA PNG는 원본 Texture2D처럼 2의 거듭제곱 크기로 투명 패딩한다. 이미지 논리 크기는 그대로 두고 저장 텍스처 크기로 스프라이트 UV를 계산한다. 메시 UV에도 콘텐츠 크기 비율을 적용한다. 알파 premultiply는 포팅판의 소수점 절삭을 따른다. 생성과 업로드는 자원 준비 때 실행한다. [대조 기록](evidence/module-work-20261105.md).

## iPhone의 오른쪽 HUD 상단

안전 영역 때문에 원본 생산 HUD 위가 비면 `GameUI_MachSlot.png`의 빈 금속판 상단 2px만 화면 위까지 이어 그린다. 셀 창·숫자·생산 버튼과 입력 좌표는 옮기지 않는다. 원본 판이 이미 화면 위를 덮는 iPad 배치에서는 추가로 그리지 않는다. 원본 PNG와 월드 표시 범위는 그대로다. [검사와 남은 월드 과제](evidence/iphone-hud-top-20261106.md).
