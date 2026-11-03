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
