# 검사

CMake/CTest는 공용 C++의 데이터·전투·생산·강화·저장·UI·긴급지원을 검사한다. `Tests/*Sweep.hpp`는 코어와 Apple XCTest에서 재사용하는 조건 집합이다. `Apple/Tests`는 치트 앱을 호스트로 GPU·오디오·입력·시험 저장을 검사한다.

실행 명령은 [개발 환경](../docs/development.md), 조건과 기대값은 [검증 계획](../docs/validation-plan.md), 기존 실행 결과는 [현재 상태](../docs/current-status.md)를 따른다. 테스트 파일이 있다는 것과 그 검사를 기기에서 통과했다는 것은 구분한다.

문서 링크·앵커 검사의 회귀 검사는 `python3 -m unittest discover -s Tests/tools -p 'test_document_contract.py'`로 실행한다. 원본 유출 차단 검사는 같은 디렉터리의 `test_original_guard.py`를 사용한다.
