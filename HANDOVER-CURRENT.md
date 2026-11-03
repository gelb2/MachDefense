# MachDefense 인계 — 2026-11-04

위치: `/Users/sokol/Git/ProjectGameDev-Hub/MachDefense`. 독립 비공개 저장소 `gelb2/MachDefense`다. 시작 문서는 AGENTS.md와 docs/canonical.md이며 현재 결과는 docs/current-status.md에 있다.

이번 요청은 HeavyMach 1과 같은 스킬·절차·문서 체계로 개발 가능한 상태를 만드는 것이다. 게임 개발은 아직 시작하지 않았다. 다음 작업은 docs/restoration-plan.md의 원본 세 판본 비교와 기준 선정이다. Mesabloo 포크 기반 개발이 아니다.

IPA 19개는 저장소 밖 원본 보관소에 있다. 파일을 저장소·원격·보고 사이트에 복사하지 않는다. `.local/originals.json`은 보관 경로만 담고 Git 제외다. 실제 스킬은 `.agents/skills`, HeavyMach 1 원본은 `docs/reference/heavymach1`에 있다.

새 clone에서는 tools/setup.py로 훅을 설치한다. docs/repository-management.md의 브랜치·반출 규칙을 따른다. HeavyMach 1·Unity·기존 hm-defense와 사용자 저장을 보존한다. 자동 새 세션 전환이나 별도 인계 승인을 요구하지 않는다.
