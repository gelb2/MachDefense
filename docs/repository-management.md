# 저장소 관리

GitHub 저장소는 비공개로 생성한다. 공개 변경은 별도 사용자 지시가 필요하다. 기존 HeavyMach 1·Unity·hm-defense를 덮거나 이력을 가져오지 않는다.

초기 빈 main에서 develop을 만들고 작업 브랜치에서 설정한다. 작업 브랜치→develop→main 순서를 유지한다. 실제 개발은 develop에서 새 브랜치를 만든다. main의 사용자 직접 수정은 병합 전에 fetch·본문 대조한다. 기여자의 커밋·작성자 기록을 보존한다.

`python3 tools/setup.py`는 core.hooksPath를 설치한다. pre-commit은 staged blob, pre-push는 도달 가능한 전체 커밋의 blob을 검사한다. IPA 경로·확장자, 원본 SHA, 원본 실행 바이너리 SHA, LFS 원본 포인터, ZIP 내부 Payload/*.app 및 중첩 IPA를 거절한다. 손상되거나 깊이 제한을 넘는 ZIP도 거절한다. gzip·bzip2·xz·7z·RAR·tar는 준비 단계에서 추적을 거절하며, 필요해지면 로컬 내용 검사부터 추가한다. 개인키는 gitleaks로 검사한다. 푸시 때 GitHub private 상태와 main 병합 부모를 확인한다.

ignore·훅을 강제로 우회하거나 Git 밖에서 업로드하면 Git 검사는 막을 수 없다. 그래서 원본 자체를 외부 보관하며 도구·보고·업로드의 입력에도 IPA를 주지 않는다. 이 규칙은 HeavyMach 2에도 동일하다. 공개·비공개에 관계없이 IPA를 외부로 보내지 않는다.

새 clone과 CI에서도 동일 검사를 실행한다. 원격 CI는 업로드 전 차단의 대체가 아니며 로컬 훅과 사전 검사 후에만 push한다. 훅 삭제·비활성화·--no-verify·LFS 우회는 하지 않는다. 새 앱의 배포 파일과 원본 패키지는 구분하며 원본을 포함한 배포 아카이브는 금지한다.
