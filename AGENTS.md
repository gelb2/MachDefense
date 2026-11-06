# MachDefense 작업 진입

[현재 인계](HANDOVER-CURRENT.md), `.agents/policy.md`, `.agents/project.md`를 읽는다. [정본](docs/canonical.md), [문서 위계](docs/document-governance.md), [기획](docs/game-design.md), [복원 계약](docs/restoration-contract.md), [설계](docs/design.md), [현재 상태](docs/current-status.md)에서 작업 범위를 확인한다. `python3 tools/document_contract.py plan --task <업무>`로 관련 정책·스킬·검사 경로를 찾는다.

- 이 저장소는 원본 IPA를 분석해 MachDefense를 소스 수준에서 복원하는 독립 프로젝트다. HeavyMach 1 복원 순서를 따른다. Mesabloo 포크를 개발 기반이나 필수 의존성으로 두지 않는다. 기존 포크는 수정하지 않는다.
- **원본 IPA는 HeavyMach 1·2·MachDefense 모두 GitHub·LFS·릴리스·보고 사이트·클라우드·외부 분석 서비스에 업로드하지 않는다. 비공개 저장소도 예외가 아니다.** 압축·이름 변경·분할·인코딩으로 반입하지 않는다. 원본과 추출 실행 바이너리는 저장소 밖에 유지한다. 로컬 REA만 사용하고 외부 전송이 있는 도구는 사용하지 않는다.
- 2026-11-06 사용자 지시로 저장소를 공개한다. 원본 IPA와 사용자 저장은 계속 외부 반출 금지다. 게임 코드 구현은 develop에서 분기한다. feature→develop→main 순서이며 main에 feature를 직접 병합하지 않는다. 원격 사용자 수정과 기여자 커밋을 보존한다.
- 게임 규칙·부품·좌표·프레임·무장·진척·저장 형식은 MachDefense 원본에서 확인한다. HeavyMach 1 수치·해상도·슬롯 크기를 옮기지 않는다. 함수 주소·입력 SHA·Evidence ID를 새 구현과 연결한다.
- 인게임 버튼은 원작 에셋·렌더·입력 방식으로 만든다. 텍스트 입력 같은 OS 기능만 플랫폼 어댑터로 연결한다. 전투 이벤트마다 DB/파일을 쓰지 않는다.
- 원본·사용자 저장·실패 증거·기존 프로젝트를 보존한다. 일반/치트 앱은 식별자·저장을 분리한다. 검사 후 로그를 남기고 앱 종료와 홈 화면 복귀를 각각 확인한다.
- 글쓰기에는 write-project-text, 원본 분석에는 reverse-engineer-anything과 restore-machdefense를 사용한다. [스킬 목록](docs/skills.md)은 모든 이식 스킬과 원본을 연결한다. 원본 스킬 사본은 참고용이며 이 프로젝트 지침이 우선한다.
- 문서·코드·앱 결과가 서로 설명돼야 한다. 미구현·추정·검사 실패를 완료로 쓰지 않는다. 자동 새 채팅·90% 강제 전환·인계 승인 절차를 도입하지 않는다.
