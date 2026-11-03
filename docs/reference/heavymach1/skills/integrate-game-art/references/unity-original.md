# 분리 전 Unity 아트 절차 — HeavyMach에 적용하지 않음

AGENTS.md의 범위를 확인하고 `docs/project-workspace.md`에서 제작 checkout을 찾는다. 그곳의 `art/production/current.json`, `docs/current-work.md`, `docs/next-art-production-plan.md`가 현재 입력이다. 없는 자료를 생성하거나 과거 상태로 대신하지 않는다. 좌표·PBR·가동·Unity 게시 규약은 `docs/art-production.md`, VFX는 `docs/vfx-production-reference.md`를 해당할 때 읽는다.

실제 사진·승인 레퍼런스와 M1/T80 경로에 맞춰 원본→베이크→Unity 임포트→실제 Campaign 소비자를 연결한다. 원본과 게임을 같은 카메라·조명에서 비교한다. 파일 생성만으로 연결 완료라 하지 않는다.

적 본체 전고는 플레이어의 최소75%. V4 외형·오른쪽 포신·카메라6.75·HUD·전투 규칙·현 유닛 겹침을 보존한다. 실제 구조·가동 소유권·소켓·논리 포구를 지킨다. 원본 결함을 색·입자·장식으로 가리지 않으며 기하·재질·움직임·맵·효과의 관련 결함을 확인한다.

Blender는 기존 레시피와 `--python-exit-code 1`을 사용한다. 열린 Unity/Blender를 중복 시작하지 않는다. 아트 요청을 전체 원본 재작성·저장 형식 변경·실행 시스템 신설로 확대하지 않는다.

변경 영향의 기능 검사와 동일조건 시각 비교를 수행한다. 기능 통과·Editor 캡처·사용자 미감 승인·실기기 성능을 구분하고 미해결 결함을 밝힌다. 전체 그래픽 후보 이후 모바일이 승인되면 `docs/mobile-delivery.md`와 game-mobile-review 역할로 연결한다. 목표는 iPhone Air/Galaxy S25 FE이며 현재 연결을 확인한다. 시뮬레이터 성능을 실기기로 보고하지 않는다. 리필·주간잔여 제작중단 조건은 AGENTS.md를 따른다. 결과 전달이 요청되면 present-game-artifacts를 사용한다. 보고 페이지의 웹 검증은 공통 engineering-browser를 적용하고, Unity 실제 장면·가동 검증은 연결된 Unity 도구와 기존 관찰 경로를 사용한다.
