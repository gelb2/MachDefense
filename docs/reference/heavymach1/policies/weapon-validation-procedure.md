# 치트 빌드 전체 무기 검증 절차

2026-10-25 사용자 지시. 무기·입력·렌더 변경 시 적용한다. 검증 앱의 실제 GameData에서 `validationWeapons`가 반환하는 모든 플레이어 무기를 열거한다. 필살기3종도 포함한다. 일부 무기 성공을 전체 성공으로 보고하지 않는다.

1. 전용 Simulator와 치트 저장을 사용한다. 빌드 SHA, 기기·OS, 화면 폭, seed, 맵, 장비, 무기명, 무적·무한탄약 조건을 기록한다. 일반 앱과 사용자 저장은 보존한다.
2. 각 무기를 실제 선택 경로로 장착한다. 원본 데이터의 종류·발사 수·탄속·중력·분산·탄약을 기록한다. 적 무기를 플레이어 무기 목록에 섞지 않는다.
3. 원본480과 확장 Air/Pro Max에서 중앙 및 좌우 추가 영역, 위·중간·아래를 검사한다. 짧은 누름, 연사 중 해제, 유지, 드래그, 화면 밖 해제, 취소, 무기 전환·정지 후 재개를 포함한다. 뒤쪽 사격 제한과 무기 고유 탄도는 원작 조건과 대조한다.
4. 실제 View 터치 → 게임 논리 좌표 → 월드 목표점 → 전체 연사 탄환 위치·각도 → 충돌/피해를 추적한다. 앞의 한 발만으로 통과시키지 않는다. 필살기는 누름과 해제 뒤의 탄착군을 각각 기록한다. 원작 분산·중력은 유지하고 터치점에 강제 적중시키지 않는다.
5. 무기별 탄환 생성, 연사 종료, 탄약 소모, 해제 후 새 연사 중단, 목표점 유지, 기본 무기로 복귀를 검사한다. 원본의 진행 중 연사는 해제 후 마무리될 수 있다. 무한탄약으로 소모 검사를 대체하지 않는다.
6. Simulator 실제 GL 화면에서 무기·탄환·폭발·클리어 일러스트와 결과 버튼을 확인한다. 자동 검사와 화면 관측을 별도 기록한다. 관련 수정 뒤 일반 앱과 치트 앱을 모두 빌드한다.
7. 무기별 pass/fail/미검사 표와 원시 로그를 남긴다. 실패를 고친 뒤 해당 무기와 공통 입력 경로를 다시 검사한다. 폰 미실행·출력음 미청취·성능 미계측을 Simulator 통과로 대신하지 않는다.

완료 기준은 무기 목록 전체의 결과 행이 있고 실패가 없는 것이다. 기능 검사와 별도인 실제 폰 성능·발열 검증은 기존 TODO를 따른다.

## 현재 실행 결과와 재실행

2026-10-25 [검사 결과](weapon-result-fix-20261025.md): 20무기×3화면×6해제/정지 입력360조건 통과. 자동 실행 대상은 `HeavyMachRenderTests/HMFramePerformanceTests/testAllWeaponsKeepTargetsAcrossTouchRelease`다. 결과화면은 `testResultIllustrationsCoverExpandedCanvas`와 기존 `HMResourceLifecycleTests/testResultScreensRenderOriginalJPEGIllustrations`를 함께 실행한다. `HeavyMachValidation` scheme을 쓰며 실제 연결된 전용 Simulator ID와 새로운 resultBundlePath를 지정한다. 후속 충돌·피해/출력음/실기기 범위는 위 절차와 결과 문서에서 별도 추적한다.

2026-10-25 추가 실기기 실행: iPhone Air iOS27.0.1에서 동일20무기360조건 통과, 탄환1,080발·목표/궤적 위반0. [실기기 결과와 남은 검수](physical-self-test-20261025.md).

## 쉴카·퉁구스카 회귀 검사

`GroundAATests`에서 흰색 탄환 6/4발, 미사일 2발과 간격·기존 정의 보존을 확인한다. `GroundAARuntimeTests`에서 152개 위치의 착지·좌우 이동과 플레이어 20무기×2차량의 탄환 충돌·피해·파괴를 검사한다. Apple 치트 앱에서는 `testGroundAAAssemblyRadarWeaponsAndNoShotSave`로 레이더 8방향·차체 2프레임·사격·일시정지·저장 호출을 검사하고 OpenGL 화면을 보존한다. `testEveryWeaponKeyboardSelectionAndFire`, `testAutoAimEveryWeaponWithMovementAndJump`, `testTouchFireWhileKeyboardMoves`, `testLuxEightAircraftBossWaveAndPause`도 함께 실행한다.
