# 소스 앱 통합 검사 · 2026-11-04

## 코드와 실행

`GameSession`이 Menu·Transition·World·GameHUD·UpgradeDialog·SummaryDialog를 연결한다. 원본 5슬롯 SaveData를 사용한다. 일반·치트 앱은 별도 bundle ID와 저장 컨테이너를 쓴다. 호환 실행기와 원본 IPA는 제품 빌드 입력에 없다.

CMake의 구성 요소 검사와 GameSession 통합 검사 22개가 모두 통과했다. 로컬 결과는 `.local/source-build/Testing/Temporary/LastTest.log`에 있다. WorldTests는 phone/pad 각각 80맵의 로딩·기지 위치·HP·생산·카메라·승패를 검사한다. GameSessionTests는 실제 리소스와 터치 이벤트로 메뉴·도움말·생산·일시정지·강화창·결과창·다음 스테이지를 검사한다. 승패 검사는 적 또는 아군 기지를 직접 파괴해 발생시킨다.

## 확인한 수정

- 새 Apple 이름 입력창의 requestName이 cancelTouches를 호출하면서 sendTouches의 iterator를 무효화했다. 이벤트 전달 전에 터치 목록을 갱신하고 취소 목록은 별도 사본으로 순회하도록 수정했다. NEW→영문 입력→OK를 직접 재검사했다.
- World 초기 margin178 뒤의 GuiGame.setUIPos 연결이 빠져 맵과 하단 HUD 사이에 검정 영역이 생겼다. phone은 -178+240=62, pad는 -384+562=178이다. 원본 값과 하단 패널 이동을 연결했다.
- 원본 셀 강화 버튼과 영구 기지의 CellStorage 강화는 서로 다르다. 전투 중 셀 레벨·셀 비용과 영구 슬롯의 Cr 비용을 분리한다.
- 특수공격 강화 비용은 UpgCost만이 아니다. 보유 수량×(다음 구입가−현재 구입가)를 더한다. 폭탄2개 보유 시 첫 강화비600 Cr를 검사했다.
- 보상 아이콘은 ItemInfo.Icon의 atlas ID가 아니라 SpAttackList.ItemId로 파일명을 만든다.
- 장비 창 외부 입력, 생산/공습 장비 전환, 기지 버튼 차단, 진형 선택과 공습 선택의 구분을 대조했다.
- GameUI_MachSlot의 원본 layer5와 하단 QuickBtn의 상속 layer9를 대조했다. 배경을 먼저 그리도록 수정하고 3개 패널×3개 패널 위치에서 그리기 순서와 버튼 터치를 검사했다.
- 문서·시뮬레이터 검사에는 원본 IPA·원본 실행 파일·사용자 실제 저장을 올리지 않는다.

## 직접 실행한 iPad 검사

프로필 선택·첫 미션·MachineGun 생산과 이동·공습 목표 지정·재고2→1·기지 강화창 열기/닫기·Save & Exit를 확인했다. 앱을 종료하고 다시 실행한 뒤 저장을 디코딩했다. AirStrike-Bomb1, Missile2, Nuclear1이 유지됐다.

iPad 전환 지연의 2초 main-thread sample1396개 중1254개가 glDrawArrays→GLEngine→GLRendererFloat 경로에 있었다. `.local/source-ipad-sample.txt`에 보존했다. 시뮬레이터의 소프트웨어 렌더 부하이며 실기기 프레임 성능은 아직 측정하지 않았다.

Git 추적 파일과 새 파일2786개를 original_guard의 경로·해시·아카이브 검사로 확인했다. 원본 패키지·실행 파일은 검출되지 않았다.

최종 Apple 일반·치트 증분 빌드는 둘 다 성공했고 해당 로그의 warning/error는0개였다. iPhone에서 QuickBtn이 보이는 최종 화면을 `.local/source-iphone-battle-final.png`에 보존했다. iPad 최종 레이어 수정본의 직접 확인은 Mac 잠금 때문에 남았다.

## 남은 범위

전체 미션 자연 플레이, 모든 강화·공습·진형의 포팅판 대조, 총구·피격·파괴와 일부 궤적 연출, 화면 크기별 배치, 장시간 성능 검사가 남았다. GameCenter·옛 구매 서비스·지원 이메일은 연결하지 않았다. 현재 화면 좌표는320×480이다. 웹·PWA·Tauri는 아직 구현하지 않았다.

원본 함수별 근거는 [분석 기록](source-analysis-20261104.md)과 [메뉴·HUD 기록](menu-layout-20261104.md), 현재 결과는 [상태](../current-status.md)에 연결한다.
