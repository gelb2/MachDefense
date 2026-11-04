# MachDefense 공용 코어

C++20으로 원본 저장·표·MCB·유닛 조립·메뉴·생산·무기 계산을 구현한다. 플랫폼 출력·소리·파일·이름 입력은 `Game.hpp`의 ResourceAccess와 HostServices를 통해 연결한다.

`GameSession`과 `createGame`이 메뉴·전투·HUD·저장·결과창을 연결한다. 특수 효과와 원본 대비 전체 플레이 검사는 진행 중이다. 원본 함수 주소와 남은 범위는 [분석 기록](../../docs/evidence/source-analysis-20261104.md), [현재 상태](../../docs/current-status.md)에 있다.

검사:

```sh
cmake -S . -B .local/source-build
cmake --build .local/source-build
ctest --test-dir .local/source-build --output-on-failure
```

준비된 Resources를 사용하므로 위 검사는 IPA를 읽지 않는다.
