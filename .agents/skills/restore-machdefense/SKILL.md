---
name: restore-machdefense
description: MachDefense 원본 분석·호환 실행·소스 재구현과 비교 검증에 사용한다.
---
# restore-machdefense

docs/restoration-plan.md와 source-provenance.md를 읽는다. 원본은 외부 경로에서 읽기만 하고 로컬 REA에만 제공한다. 기준 판본을 확인한 뒤 함수/데이터/ABI 증거를 기록하고 새 소비 코드와 연결한다. HeavyMach 1의 주소·메모리 구조·수치를 옮기지 않는다. 원작 버튼·렌더·시간·저장 규칙을 보존하고 같은 조건으로 일반/치트 앱을 검증한다. 기존 hm-defense 포크는 이 복원의 기반이 아니다.
