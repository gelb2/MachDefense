# 교전·수치 계산

유닛 생산·목표 선택·경로·사거리·탄도·피격·방어·보상·성장·스폰 조건을 원본 함수와 데이터에 연결한다. HeavyMach 1의 무기나 AI를 기본값으로 가져오지 않는다. 계산 순서·정밀도·난수·업데이트 간격을 동일 조건으로 비교한다.

현재 [World](../Sources/Core/Simulation/World.cpp), [Combat](../Sources/Core/Simulation/Combat.cpp), [WeaponController](../Sources/Core/Simulation/WeaponController.cpp), [Settlement](../Sources/Core/Session/Settlement.cpp)에 전투·탄환·보상 계산을 구현했다. 함수 주소와 수치 근거는 [원본 분석](evidence/source-analysis-20261104.md), 검사 범위는 [캠페인 검사](evidence/source-cheat-campaign-20261104.md)를 따른다. HUD 누적 점수는 원본처럼 슬롯 +0x2054를 표시한다. 전투별 처치 통계와 누적 점수를 구분한다. 포팅판과 같은 조건의 실제 전투·보상 대조는 [진행 중](evidence/port-source-parity-20261104.md)이다.

## 추가 단계와 남은 원작 대조

10레벨 확장 수치는 [레벨별 계획](research/level10-plan-20261107.md)을, 실제 사거리·피해와 후속 효과는 [사거리 검사](evidence/level10-range-damage-20261107.md)·[효과 검사](evidence/level10-effects-20261107.md)를 따른다. 승인된 추가 행을 원작 수치로 설명하지 않는다.

[지형 후속 대조](evidence/terrain-original-followup-20261106.md)와 [경제·프레임 조사](evidence/economy-frame-audit-20261106.md)의 미해결 조건은 TODO에 남겼다. 조사 완료와 게임 버그 수정 완료를 구분한다.
