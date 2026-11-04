# 교전·수치 계산

유닛 생산·목표 선택·경로·사거리·탄도·피격·방어·보상·성장·스폰 조건을 원본 함수와 데이터에 연결한다. HeavyMach 1의 무기나 AI를 기본값으로 가져오지 않는다. 계산 순서·정밀도·난수·업데이트 간격을 동일 조건으로 비교한다.

현재 [World](../Sources/Core/World.cpp), [Combat](../Sources/Core/Combat.cpp), [WeaponController](../Sources/Core/WeaponController.cpp), [Settlement](../Sources/Core/Settlement.cpp)에 전투·탄환·보상 계산을 구현했다. 함수 주소와 수치 근거는 [원본 분석](evidence/source-analysis-20261104.md), 검사 범위는 [캠페인 검사](evidence/source-cheat-campaign-20261104.md)를 따른다. HUD 누적 점수는 원본처럼 슬롯 +0x2054를 표시한다. 전투별 처치 통계와 누적 점수를 구분한다. 포팅판과 같은 조건의 실제 전투·보상 대조는 [진행 중](evidence/port-source-parity-20261104.md)이다.
