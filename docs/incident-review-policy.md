# 경위서 사전 읽기와 작업 후 재검증

2026-11-09 사용자 지시. 업무지시 분석은 instructions, 소스코드 작업은 code, 에셋·그래픽·렌더링 작업은 graphics를 적용한다. instructions는 모든 작업에 포함된다. 실제 읽을 원문은 [.agents/incident-review.json](../.agents/incident-review.json)에 있다. 그래픽 작업은 code도 함께 선택한다.

## 작업 전

1. 변경을 시작하기 전에 `python3 tools/incident_review.py read --category code --task "구체적인 작업"`을 실행해 출력된 원문을 모두 읽는다. 그래픽은 `--category graphics`도 붙인다. `plan`은 목록 안내일 뿐 읽기 기록을 만들지 않는다.
2. 각 경위서에서 이번 작업에 적용할 항목을 JSON에 적는다. 예: `{"docs/evidence/aircraft-work-failure-20261109.md": ["승인 외형을 바꾸지 않고 실제 소비 경로와 검사 결과를 대조한다."]}`. 명령이 출력한 모든 경위서 경로를 포함해야 한다.
3. `python3 tools/incident_review.py acknowledge --notes .local/task-before.json`으로 적용 항목을 기록한다. 이 단계 전에는 파일을 수정하지 않는다.

새 읽기는 깨끗한 작업 사본에서 시작한다. 이미 존재하는 사용자 변경을 삭제·reset하거나, 작업 뒤 읽은 것을 작업 전 읽기로 소급 처리하지 않는다. 기존 변경은 보존하고 별도 작업 사본을 사용한다. 기록은 각 worktree의 Git 내부 파일 `incident-review-state.json`에 보관한다. 이전 HEAD의 기록은 다음 작업에 재사용할 수 없다.

## 작업 후

1. 실제 변경과 관련 검사를 마친 뒤 커밋할 파일을 stage한다.
2. `python3 tools/incident_review.py read --after`로 작업 전에 읽었던 경위서를 다시 읽는다. 경위서 자체를 수정한 작업은 사전 버전과 수정본을 함께 대조한다. 사전 버전의 해시는 기준 커밋과 대조하고, 수정본은 재읽기 시점의 해시를 기록한다.
3. 사전 적용 항목마다 실제 변경과 검사 근거를 적는다. 형식은 아래와 같다. 모든 경위서의 모든 사전 항목을 같은 순서로 포함한다.

```json
{
  "docs/evidence/aircraft-work-failure-20261109.md": [
    {
      "requirement": "승인 외형을 바꾸지 않고 실제 소비 경로와 검사 결과를 대조한다.",
      "result": "pass",
      "detail": "변경 파일과 승인 자원의 해시, 실제 실행 결과를 대조한 내용",
      "evidence": ["실제로 존재하는 저장소 상대 경로"]
    }
  ]
}
```

4. `python3 tools/incident_review.py review --notes .local/task-after.json`을 실행한다. 적용 대상이 아닌 항목은 `not-applicable`과 구체적인 이유·근거를 적는다. 실패 항목은 `failed`로 남기고 수정한다. 빈 근거와 미해결 항목으로 통과시키지 않는다.
5. `python3 tools/incident_review.py check --staged`가 통과한 뒤 커밋한다. 질문·분석처럼 커밋이 없는 작업도 완료 답변 전에 `check`로 확인한다.

재대조 후 stage 내용, 작업 파일, 경위서, 근거 파일이 바뀌면 검증은 실패한다. 최종 상태에서 재읽기·재대조해야 한다. 작업 범위를 소스·그래픽으로 넓혔는데 해당 사전 읽기가 없으면 실패한다. 경위서 목록을 갱신하는 작업에서는 새 원문도 작업 후 읽고 `read --after --notes <전체 적용 항목 JSON>`으로 새 항목을 추가한다. 기존 사전 적용 항목을 지우지 않는다.

## 실패 처리와 범위

[pre-commit](../.githooks/pre-commit)의 첫 검사로 연결했다. 읽기 없음, 적용 항목 없음, 재읽기 없음, 재검증 없음, 근거 없음, 검증 뒤 변경은 종료 코드 1로 커밋을 차단한다. 읽지 않은 작업을 정상·완료로 보고하지 않는다. 사용자가 명시적으로 폐기한 자동 새 채팅·90% 전환·인계 승인 절차는 다시 도입하지 않는다.

이 도구는 원문 출력, 적용 항목, 파일 해시와 재대조 근거가 있는지 검사한다. 사람이 내용을 이해했는지와 근거 내용의 진실성을 자동으로 증명하지 않는다. 에이전트가 실제로 읽고 해당 항목을 변경·검사와 대조할 책임은 유지된다. Git 훅을 건너뛰거나 기록을 수동 위조하면 보호가 깨지므로 허용하지 않는다. 원작 기능·시각 검수와 회귀 검사는 별도로 실행한다.

[격리 저장소 회귀 검사](../Tests/IncidentReviewTests.py): `python3 Tests/IncidentReviewTests.py`.
