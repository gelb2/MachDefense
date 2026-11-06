# 레이저 포탑의 해금 상태

2026-11-07 사용자 질문에 따라 원본 패키지 3개의 `MachRegen.tble`을 읽고 현재 `Resources/Tables/MachRegen.tsv`와 비교했다.

세 원본 모두 `//LaserTurret 20`으로 주석 처리돼 있다. 현행 테이블도 같다. `DataTable`은 `//` 행을 건너뛰며 `Settlement::addBuild`는 이 테이블의 활성 행에서만 새 유닛을 뽑는다. 초기 보유 유닛 4종에도 레이저 포탑은 없다.

현재 복원판은 특정 미션에 레이저 포탑을 고정 배정하지 않는다. 미션 최초 클리어 때 난수 `%40 <= 9` 조건을 통과하면 미보유 유닛 중 가중 추첨한다. 레이저 포탑은 목록에 없으므로 일반 진행으로 획득할 수 없다. 그림·무장·강화 데이터는 존재해 치트나 해당 유닛을 보유한 복원 데이터에서는 사용할 수 있다.

이번 부품 연결은 보상표·초기 보유 목록·정산 코드를 변경하지 않았다. 레이저 포탑의 일반 해금을 추가하려면 별도의 게임 규칙 변경으로 다룬다.

| 원본 | 보상표 SHA-256 |
|---|---|
| Mach_Defense v1.0.3 - (kr.indieapps.MachDefense) | `2fbfaedbef67699aaad6faf88cf4c0bd819ffdabed707dd7b85153b8d97d9a21` |
| Mach_Defense v1.0.5 - (kr.indieapps.MachDefense) | `2fbfaedbef67699aaad6faf88cf4c0bd819ffdabed707dd7b85153b8d97d9a21` |
| MachDefense v1.0.3 - (com.wemadeent.HeavyMachDefence) | `2fbfaedbef67699aaad6faf88cf4c0bd819ffdabed707dd7b85153b8d97d9a21` |

근거: `Sources/Core/Session/Settlement.cpp`, `SaveData.cpp`, `Sources/Core/Data/DataTable.cpp`, `Resources/Tables/MachRegen.tsv`. 원본 파일은 저장소 밖에서 읽었으며 추가하지 않았다.
