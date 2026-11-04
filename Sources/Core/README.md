# MachDefense 공용 코어

C++20 게임 코드다. Apple SDK·OpenAL·그래픽 API를 직접 호출하지 않는다.

| 폴더 | 담당 |
|---|---|
| Contracts | 플랫폼 연결·프레임 명령·좌표 계약·원본 난수 |
| Data | 표·MCB 자원 읽기 |
| Simulation | 이동·충돌·표적·사거리·탄환·피해·생산·스폰·효과 상태 |
| Session | 프로필·메뉴 전환·저장 직렬화·보상 |
| Presentation | 메뉴·HUD·버튼·글꼴·월드 그리기 명령 |
| Validation | 치트 조건 준비·검사 패널 |

`Contracts/Game.hpp`의 ResourceAccess·HostServices가 플랫폼 연결점이다. `Session/GameSession.cpp`의 createGame으로 시작한다. 월드 갱신 순서는 `Simulation/World.cpp`, 표적은 WorldTargeting, 이동은 WorldMovement, 지형은 WorldTerrain, 사격·피해는 WorldCombat에서 관리한다. 부품 조립과 포구 계산은 MachineModels를 함께 사용한다.

```sh
cmake -S . -B .local/source-build -DCMAKE_BUILD_TYPE=Release
cmake --build .local/source-build
ctest --test-dir .local/source-build --output-on-failure
```

준비된 Resources로 빌드·검사한다. [현재 상태](../../docs/current-status.md), [모듈 분리 전후 대조](../../docs/evidence/module-work-20261105.md).
