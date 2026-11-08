# 문서·코드·앱의 일치

| 단계 | 문서 | 소유 내용 |
|---|---|---|
| 요구 | 최신 사용자 지시, game-design, restoration-contract | 원본에서 보존할 경험과 승인 변경 |
| 설계 | design, coordinate/rendering/game-calculation/runtime-policy | 공용 로직·입출력 경계와 분야 규칙 |
| 구현 | Sources, Modules, 플랫폼별 폴더, Tests | 실제 소비자·빌드·입력·검사 |
| 증거 | current-status, evidence, source-provenance | 판본·함수 주소·소스 SHA·기기·결과 |
| 파생 글 | public, editorial, 보고서 | 확인된 사실을 설명하는 원고 |

요구가 바뀌면 설계·코드·검사·상태를 함께 대조한다. 실패가 나면 어느 요구와 소비 경로가 달라졌는지 조사한다. 버그를 정상화하려고 문서를 고치지 않는다.

업무 진입은 document-map.json이다. `python3 tools/document_contract.py check`로 업무 경로, 추적 중이거나 새로 작성한 Markdown의 로컬 인라인 링크·절 앵커, 스킬 출처 해시를 검사한다. docs/reference·이식 스킬·ThirdParty의 보존 사본은 자체 상대경로를 유지하므로 링크 검사에서 제외한다. 외부 URL 응답과 문장 의미는 별도 확인한다. 링크 검사는 기능 정상동작을 증명하지 않는다. 매 구현에는 요구→원본 함수/데이터→소비 코드→검사→실행 결과를 남긴다. 아직 없는 코드는 미구현으로 표시한다.

현재 작업 순서와 완료 여부는 [TODO](usability-todo.md)에서 관리한다. [현재 상태](current-status.md)는 최신 결과를 맨 위에 두고 이전 기록에 당시 단계를 표시한다. [인계](../HANDOVER-CURRENT.md)는 이 두 문서와 해당 검사 근거를 연결한다. 순서가 바뀌면 [재개발 계획](source-reimplementation-plan.md)과 [검증 계획](validation-plan.md)도 같은 순서로 맞춘다.

## 경위서 읽기·재대조 필수

[경위서 검사 절차](incident-review-policy.md)에 따라 작업 전에 분야별 원문을 읽고 적용 항목을 기록한다. 작업 후 같은 경위서를 다시 읽고 실제 변경·검사와 재대조한다. 기록 누락 또는 재검증 뒤 변경은 실패이며 완료 보고·커밋을 금지한다.
