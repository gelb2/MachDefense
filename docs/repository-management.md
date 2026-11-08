# 저장소 관리

2026-11-06 사용자 지시로 GitHub 저장소를 공개 전환했다. 기존 HeavyMach 1·Unity·hm-defense를 덮거나 이력을 가져오지 않는다.

초기 빈 main에서 develop을 만들고 작업 브랜치에서 설정한다. 작업 브랜치→develop→main 순서를 유지한다. 실제 개발은 develop에서 새 브랜치를 만든다. main의 사용자 직접 수정은 병합 전에 fetch·본문 대조한다. 기여자의 커밋·작성자 기록을 보존한다. 2026-11-06 사용자 지시에 따라 push 직전에도 origin/main·origin/develop을 fetch하고 문서 변경을 다시 대조한다. 사용자 웹 편집을 보존하며 충돌을 파일 전체 덮어쓰기로 해소하지 않는다. 원격이 검토 이후 다시 바뀌면 push를 중단하고 새 변경을 병합한다.

`python3 tools/setup.py`는 core.hooksPath를 설치한다. pre-commit은 staged blob, pre-push는 도달 가능한 전체 커밋의 blob을 검사한다. IPA 경로·확장자, 원본 SHA, 원본 실행 바이너리 SHA, LFS 원본 포인터, ZIP 내부 Payload/*.app 및 중첩 IPA를 거절한다. 손상되거나 깊이 제한을 넘는 ZIP도 거절한다. gzip·bzip2·xz·7z·RAR·tar는 준비 단계에서 추적을 거절하며, 필요해지면 로컬 내용 검사부터 추가한다. 개인키는 gitleaks로 검사한다. 푸시 때 사용자가 승인한 GitHub public 상태와 main 병합 부모를 확인한다.

ignore·훅을 강제로 우회하거나 Git 밖에서 업로드하면 Git 검사는 막을 수 없다. 그래서 원본 자체를 외부 보관하며 도구·보고·업로드의 입력에도 IPA를 주지 않는다. 이 규칙은 HeavyMach 2에도 동일하다. 공개·비공개에 관계없이 IPA를 외부로 보내지 않는다.

새 clone과 CI에서도 동일 검사를 실행한다. 원격 CI는 업로드 전 차단의 대체가 아니며 로컬 훅과 사전 검사 후에만 push한다. 훅 삭제·비활성화·--no-verify·LFS 우회는 하지 않는다. 새 앱의 배포 파일과 원본 패키지는 구분하며 원본을 포함한 배포 아카이브는 금지한다.

## 웹 배포

정식 게임은 `Web/tools/package.py`로 포장하고 `Web/tools/publish.py`로 독립 `gh-pages` 브랜치에 게시한다. GitHub Pages는 이 브랜치의 루트를 배포한다. 제품 파일 허용 목록과 SHA-256을 검사하며 소스·개인 저장·원본 패키지를 배포 폴더에 섞지 않는다. 게임 소스의 main 병합 규칙은 그대로 유지한다.

## develop·main 갱신 권한 — 2026-11-09

[GitHub 규칙 26816831](https://github.com/gelb2/MachDefense/rules/26816831)은 `develop`과 `main`의 갱신을 제한한다. 예외는 사용자 계정 `gelb2` 하나다. 다른 계정은 직접 push하거나 PR을 병합할 수 없다. 소유자에게도 적용하던 관리자 강제 제한은 해제했다. 기존 PR·`check` 검사·대화 해결 조건은 유지하며 소유자는 예외를 사용할 수 있다.

GitHub API에서 두 브랜치의 활성 `update` 규칙과 단일 사용자 예외를 재조회했다. 다른 계정으로 실제 거절을 발생시키는 실험은 수행하지 않았다. 같은 `gelb2` 인증을 쓰는 도구와 사람은 GitHub가 구분하지 못한다. 개발 작업은 계속 작업 브랜치→develop→main 순서를 따른다.
