# 릴리스 절차

CHANGELOG.md의 버전별 항목을 GitHub Releases 본문의 원본으로 쓴다. README의 버전 배지와 최신 릴리스 링크도 함께 갱신한다. 앱 내부 버전과 Git 태그가 다르면 릴리스 노트에 둘을 적는다.

1. 최신 원격 main·develop을 가져와 사용자 변경과 작업 브랜치의 차이를 확인한다.
2. 작업 브랜치를 develop에 병합한다. 문서는 별도 작업 브랜치에서 작성해 develop에 병합한다.
3. develop을 main에 병합하고, 그 main 병합 커밋에 주석 태그를 붙인다. 기존 태그는 이동하지 않는다.
4. 로컬 훅의 원본 패키지·비밀값·브랜치·승인된 공개 상태 검사를 거쳐 main·develop·태그를 push한다.
5. CHANGELOG 본문의 문서 링크를 태그 고정 URL로 바꿔 GitHub Release를 만든다. 설치 파일을 제공하면 SHA-256과 서명 종류도 기록한다.
6. 원격 태그와 Release 본문·첨부 파일·README 링크를 다시 확인한다.

이 저장소는 2026-11-06 사용자 지시로 공개했다. GitHub Release는 main 병합과 태그가 완료된 뒤 작성한다. 원본 게임 패키지와 사용자 저장 파일은 릴리스에 넣지 않는다.

[버전 이력](../CHANGELOG.md) · [저장소 관리](repository-management.md)
