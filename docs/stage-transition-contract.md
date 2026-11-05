# 화면·스테이지 전환

슬롯→메뉴→미션→전투→결과→성장/업그레이드→다음 미션의 실제 원본 전환을 기록한다. 모달 창의 입력 소비와 게임 tick 중단/재개를 연결한다. 배경 전환 뒤 중복 창, 열린 창 뒤 전투 진행, 닫히지 않는 버튼을 검사한다.

[GameSession](../Sources/Core/Session/GameSession.cpp)에 프로필·미션·전투·강화·결과 전환을 연결했다. 결과 저장이 실패하면 다음 미션·재도전·미션 선택으로 이동하기 전에 저장을 재시도한다. [코어 검사](evidence/source-cheat-campaign-20261104.md)는 80개 미션의 승패·보상·해금·재실행을 다룬다. 실제 치트앱을 호스트로 한 80미션·강화·무장 연속 검사는 [전체 검사](evidence/full-upgrade-weapon-device-20261104.md)와 [모듈화 후 기기 검사](evidence/module-ipad-device-20261105.md)에서 수행했다. 같은 조건의 포팅판 교전·음향/연출 대조는 [TODO](usability-todo.md)에 남아 있다.

2026-11-04 사용자 실기기 확인: 적 기지 파괴→미션 클리어, 아군 기지 파괴→실패 창→미션 선택 복귀. 이는 당시 사용자 확인이다. 후속 소스판의 다음 스테이지·저장 재실행은 위 자동검사와 [포팅판 대조 기록](evidence/port-source-parity-20261104.md)에 있다. [MD-PLAY-02/03](port-observations-20261104.md).
