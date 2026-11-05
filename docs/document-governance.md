# 문서·코드·앱의 일치

| 단계 | 문서 | 소유 내용 |
|---|---|---|
| 요구 | 최신 사용자 지시, game-design, restoration-contract | 원본에서 보존할 경험과 승인 변경 |
| 설계 | design, coordinate/rendering/game-calculation/runtime-policy | 공용 로직·입출력 경계와 분야 규칙 |
| 구현 | Sources, Modules, 플랫폼별 폴더, Tests | 실제 소비자·빌드·입력·검사 |
| 증거 | current-status, evidence, source-provenance | 판본·함수 주소·소스 SHA·기기·결과 |
| 파생 글 | public, editorial, 보고서 | 확인된 사실을 설명하는 원고 |

요구가 바뀌면 설계·코드·검사·상태를 함께 대조한다. 실패가 나면 어느 요구와 소비 경로가 달라졌는지 조사한다. 버그를 정상화하려고 문서를 고치지 않는다.

업무 진입은 document-map.json이다. `python3 tools/document_contract.py check`로 파일·링크·스킬 출처를 검사한다. 링크 검사는 기능 정상동작을 증명하지 않는다. 매 구현에는 요구→원본 함수/데이터→소비 코드→검사→실행 결과를 남긴다. 아직 없는 코드는 미구현으로 표시한다.

현재 작업 순서와 완료 여부는 [TODO](usability-todo.md)에서 관리한다. [현재 상태](current-status.md)는 최신 결과를 맨 위에 두고 이전 기록에 당시 단계를 표시한다. [인계](../HANDOVER-CURRENT.md)는 이 두 문서와 해당 검사 근거를 연결한다. 순서가 바뀌면 [재개발 계획](source-reimplementation-plan.md)과 [검증 계획](validation-plan.md)도 같은 순서로 맞춘다.

## 현행 문서와 과거 기록

업무별 소비 코드와 검사는 [대조표](code-document-map.md)로 연결한다. 계획서·인계의 과거 단계는 당시 결과로 보존하고 현행 작업으로 다시 실행하지 않는다. 문서 검사 도구는 유지 문서의 하위 폴더·로컬 파일·절 앵커까지 검사한다. 외부 주소의 접속 상태와 실제 앱 동작은 이 검사에 포함되지 않는다.

[2026-11-06 감사](evidence/project-hygiene-20261106.md)는 현행 안내와 코드·검사 근거의 대조 범위를 기록한다. 보고 사이트의 게시 여부는 [게시 기록](report-site.md)에서 따로 확인한다.
