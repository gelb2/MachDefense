# 위메이드 포팅판과 소스판 대조 — 2026-11-04

기준은 위메이드 1.0.3이다. 실행 파일 SHA-256은 `6ec47ea094abaa31bab10058fe47bb25d46c6038b2e88a7da2629899f84af112`다. 원본 분석 파일은 저장소 밖에 보관한다. [현재 순서](../usability-todo.md)의 첫 작업을 수행 중이다.

## 같은 저장으로 기지 강화 구매

iPhone Air iOS 26.5 시뮬레이터 하나에서 두 앱을 번갈아 실행했다. 원본 비교 앱과 새 소스 치트앱의 기존 시험 저장을 각각 백업했다. 사용자 실기기의 저장은 변경하지 않았다. 동일한 위메이드 시험 저장을 소스 앱에도 넣었다.

| 항목 | 시작값 | 포팅판 구매 후 | 소스판 구매 후 |
| --- | --- | --- | --- |
| Cr | 996,100 | 988,300 | 988,300 |
| 기지 강화 6종 | 표시 Lv.2, 저장값 1 | 표시 Lv.3, 저장값 2 | 표시 Lv.3, 저장값 2 |
| 누적 점수 | 274 | 274 | 274 |
| 완료 미션 수 | 0 | 0 | 0 |

두 앱에서 Base Cannon, Base Defense, Build Time, Cr Research, Cell Research, Cell Storage를 각각 선택하고 UPGRADE를 한 번씩 눌렀다. 합계 7,800Cr가 차감됐다. 다음 구매 비용은 차례로 2,400 / 2,400 / 2,800 / 2,000 / 3,200 / 2,800Cr로 일치했다. 포 공격력 46(+14), 방어력 표시 200(+100), 셀 저장량 1,000(+400)도 대조했다. Save & Exit 뒤 저장의 실제 바이트를 해독해 비교했다. 소스판을 다시 실행해 잔액과 점수 보존을 확인했다.

로컬 자료는 `.local/parity-20261104`의 `port-before.sav`, `port-after-level3.sav`, `source-after-level3.sav`, `base-comparison.json`, 두 앱의 `*-base-level3.png`다.

## 대조 중 발견한 HUD 누락

- 소스판의 전투 HUD 점수는 0이었다. 포팅판은 기존 누적 점수 274를 표시했다. GameSession이 전투별 World 통계를 넘기고 있었다. 원작 GuiGame::tick(0x276c4)이 읽는 슬롯 +0x2054로 연결했다.
- 소스판은 RaderBG 배경만 그렸고 아군·적·기지 표식과 카메라 범위가 없었다. DlgRadar의 init(0x23fec), draw(0x24634), drawMapMark(0x24450), setCamBottom:Top:(0x24310)과 QobLine::setLine(0x41dc4)을 확인했다. 원본 RaderMark.png·MovePath.png, 월드 좌표의 1/8 배율, 네 종류 표식과 카메라 범위를 연결했다.

GameSession 회귀 검사는 수정 전 표식 누락으로 실패했다. 수정 뒤 GameHUD·GameSession·World·FrameIO 4개가 통과했다. 시험 프로필의 점수 274 표시, 지도 범위 밖 표식 제외, 카메라 이동에 따른 범위 표시, 전투 중 파일 쓰기 여부를 검사한다. 시뮬레이터 새 빌드·설치·실행 후 실제 점수와 레이더 표시를 확인했다. 캡처는 `source-radar-score-fixed.png`, 검사 로그는 `hud-tests.log`다.

## 유닛 강화 대조 진행

소스 앱에서 초기 유닛 4종을 각각 Lv.1에서 Lv.2로 구매했다. 표시 잔액은 988,300Cr에서 987,250Cr로 바뀌었다. 화면은 `source-units-level2.png`에 보존했다. 이 시점에 Mac이 잠겨 Save & Exit, 저장 파일 비교와 포팅판의 같은 구매는 남았다. 이 결과를 두 앱 일치로 집계하지 않는다.

## 미션 종료 시 저장 순서 조사

원본 GobHvM_Enemy::onCrash(0xdf1c)는 killEnemy(0x17f98)를 호출한 뒤 처치 점수를 더한다. 적 기지의 killEnemy는 onClearStage(0x169b8)로 들어가며, DlgSummary::initWithClear(0x349c8)가 보상·진행도를 반영하고 저장한 뒤 나머지 적을 파괴한다. 현재 소스의 World::damage → World::finish → GameSession::finish도 결과 저장 후 처치 점수를 더하는 순서다.

따라서 이 순서 자체를 재개발판만의 오류로 판단해 변경하지 않았다. 결과창 직후 강제 종료, Next 진입, Save & Exit 뒤 재실행 때 점수가 어떻게 남는지는 두 앱에서 직접 비교할 항목이다. [전체 검사 MD-VALIDATION-03](../usability-todo.md)에 연결한다.

## 이어서 대조할 항목

기지 강화 6종의 한 단계 구매를 확인했다. 유닛·특수기술 전체 강화, 동일 조건 생산·교전·피해·승패·보상, 난수 순서와 연출·음향 대조는 남아 있다. 이 대조가 끝난 뒤 앱 아이콘 작업으로 넘어간다. 전체 항목의 앱 내 연속 검사와 실시간 성능 검사는 이후 별도 단계다.
