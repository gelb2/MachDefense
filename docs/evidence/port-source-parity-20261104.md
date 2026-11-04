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

## 초기 유닛 4종 강화 구매

두 앱에서 MachineGun·Missile·Tanker·RifleTurret를 각각 Lv.1에서 Lv.2로 구매했다. 비용은 200 / 400 / 250 / 200Cr, 합계 1,050Cr다. Save & Exit 뒤 저장을 해독해 아래 값이 일치함을 확인했다.

| 항목 | 포팅판 | 소스판 |
| --- | --- | --- |
| 잔액 | 987,250Cr | 987,250Cr |
| 누적 점수 | 274 | 274 |
| 4종 저장 레벨 | 모두 1 — 표시 Lv.2 | 모두 1 — 표시 Lv.2 |
| 장비 순서 | MachineGun → Missile → Tanker → RifleTurret | 동일 |
| 장착 상태 | 모두 장착 | 모두 장착 |

로컬 근거는 `unit-comparison.json`, `port-after-units-level2.sav`, `source-after-units-level2.sav`, 두 앱의 `*-units-level2.png`다. 각 유닛의 다음 단계 비용은 500 / 800 / 500 / 500Cr로 표시됐다.

소스판에서 CLOSE를 누르면 전투는 재개되지만 강화창 그림이 남았다. 포팅판에서는 사라졌다. GameSession이 입력 상태만 hidden으로 바꾸고 UpgradeDialog에 전달하지 않은 것이 원인이었다. 닫을 때 표시·입력 상태를 함께 갱신했다. 두 장비 창의 실제 CLOSE 입력, 그림 제거, 시간 재개와 추가 저장 없음 검사를 추가했다. 수정 전 실패를 확인했고, 수정 후 GameSession·FrameIO·SaveFailure 3종이 통과했다. 로그는 `close-red.log`, `close-tests.log`다. 수정본의 시뮬레이터 빌드·설치 후 유닛 창과 특수기술 창에서 CLOSE를 직접 눌렀다. 두 창 모두 사라졌고 전투가 재개됐다. 특수기술 창을 닫은 직후 셀 값이 547에서 550으로 증가하는 것도 확인했다.

## 초기 특수기술 3종 강화 구매

두 앱에서 AirStrike-Bomb·AirStrike-Missile·AirStrike-Nuclear를 각각 Lv.1에서 Lv.2로 강화했다. 비용은 600 / 620 / 600Cr로 합계 1,820Cr였다. 두 앱 모두 Save & Exit 후 저장을 해독했다. 잔액 985,430Cr, 점수 274, 세 기술의 저장 레벨 1, 보유 수량 2 / 2 / 1, 장착 상태와 순서가 일치했다. 로컬 근거는 `special-comparison.json`, `port-after-specials-level2.sav`, `source-after-specials-level2.sav`다.

## 미션 종료 시 저장 순서 조사

원본 GobHvM_Enemy::onCrash(0xdf1c)는 killEnemy(0x17f98)를 호출한 뒤 처치 점수를 더한다. 적 기지의 killEnemy는 onClearStage(0x169b8)로 들어가며, DlgSummary::initWithClear(0x349c8)가 보상·진행도를 반영하고 저장한 뒤 나머지 적을 파괴한다. 현재 소스의 World::damage → World::finish → GameSession::finish도 결과 저장 후 처치 점수를 더하는 순서다.

따라서 이 순서 자체를 재개발판만의 오류로 판단해 변경하지 않았다. Next 진입과 Save & Exit 뒤 재실행 결과는 아래에 기록했다. 결과창 직후 강제 종료 검사는 남았다. [전체 검사 MD-VALIDATION-03](../usability-todo.md)에 연결한다.

## 이어서 대조할 항목

기지 강화 6종의 한 단계 구매를 확인했다. 유닛·특수기술 전체 강화, 동일 조건 생산·교전·피해·승패·보상, 난수 순서와 연출·음향 대조는 남아 있다. 이 대조가 끝난 뒤 앱 아이콘 작업으로 넘어간다. 전체 항목의 앱 내 연속 검사와 실시간 성능 검사는 이후 별도 단계다.

## 첫 미션 실제 교전·클리어·저장

소스판에서 유닛 3종 생산과 특수공격으로 적 기지를 파괴했다. 결과창은 5기 파괴·아군 손실 0·1분 10초·500Cr 보상·AirStrike-Bomb 3개를 표시했다. Next Stage로 눈 지형의 Stage 2에 진입했다. Save & Exit 후 앱을 완전히 종료하고 다시 실행했다. 슬롯 화면에 Stage 2, 985,966Cr, 점수 1,324가 남았다. 결과창 직후·Next 직후·Save & Exit 뒤 저장의 진행도와 점수는 같았다. 로컬 근거는 `source-combat-comparison.json`, `source-clear-valid.png`, `source-relaunch-after-clear.png`, 각 시점의 `.sav`다.

시험 시작값은 985,430Cr·점수 274·Stage 1 미완료다. 기지 Cannon/Defense는 내부 단계 6, 나머지 기지 강화 4종은 9, Tanker/Missile/MachineGun은 9, RifleTurret는 0, 초기 특수기술 3종은 4와 각 10개다. `combat-valid-fixture.sav`를 보존했다. 각 무기는 정상 공격 경로로 발사했다. 승리 값을 직접 설정하지 않았다.

앞선 포팅판 시험에서도 실제 클리어·500Cr·보상·Stage 2 진입을 확인했다. 그러나 최초 시험 저장은 모든 단계를 9로 넣어 일부 항목의 최대 단계를 넘었다. 포팅판은 실행 시 보정했고 소스판은 존재하지 않는 기지 부품 이름으로 예외 종료됐다. 그 결과는 정상 범위의 동일 입력 비교에 사용하지 않는다. 입력·크래시를 보존하고 원본의 경계값 처리 대조를 TODO `MD-PARITY-BOUNDARY-01`에 등록했다. 정상 범위로 고친 동일 저장을 포팅판에도 넣어 다시 검사했다.

이번 수동 교전은 클릭 시점과 난수를 동기화하지 않았다. 처치 수·보상 종류·플레이 시간의 완전 일치는 아직 검사하지 않았다. 전체 항목을 앱 안에서 빠르게 연속 실행하는 절차도 남아 있다.

포팅판 재검사에서도 Stage 1 클리어·500Cr·MissileTurret 신규 보상·Stage 2 진입·Save & Exit를 확인했다. 결과창 직후와 Next 직후 저장은 986,031Cr·점수 1,324였다. 화면은 후속 처치와 수집을 반영해 986,069Cr·점수 1,334가 됐고 Save & Exit 뒤 파일에도 이 값이 남았다. `port-valid-comparison.json`, `port-valid-clear.png`, `port-valid-*.sav`에 보존했다. 소스판에서는 이번 교전의 결과창 저장 이후 점수 증가가 없었다. 남은 적과 처치 시점을 고정해 대조할 항목이다. 두 앱을 종료하고 시뮬레이터 홈 복귀를 확인했다.

## 관측값의 자동 재검사

[Apple 자동검사](apple-automated-sweep-20261104.md)에 위 구매값을 고정 기대값으로 넣었다. 13개 구매·저장·재읽기가 통과했다. 소스판의 80미션 결과 전환과 생산·특수공격도 함께 자동 실행한다.
