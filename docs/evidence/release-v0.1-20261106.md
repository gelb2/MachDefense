# v0.1 릴리스와 후속 브랜치 정리 — 2026-11-06

## 배포

- Android 구현 `639ca43` → develop 병합 `6652b1d`.
- 첫 릴리스 문서 `f0df3d0` → develop 병합 `0835b5a`.
- develop → main 병합 `08b5380`. main의 두 번째 부모는 `0835b5a`이며 두 브랜치의 파일 내용은 같다.
- 주석 태그 `v0.1`은 main 병합 커밋을 가리킨다. 원격 main·develop·태그를 다시 조회해 일치함을 확인했다.
- [GitHub Release](https://github.com/gelb2/MachDefense/releases/tag/v0.1)는 초안이 아닌 게시 상태다. 저장소는 비공개다.

일반 APK SHA-256: `dfa7313cc5c62d7f7763e1a7901c34938765cfa75c8c63fcebea5a4f1827aad0`.
치트 APK SHA-256: `86fd1691973da84209c41185f77efbf7069edf8621287b12c2fed10af8c26271`.
GitHub가 기록한 두 업로드 파일의 digest가 로컬 APK와 같다. SHA256SUMS.txt도 첨부했다.

배포 전 원본 패키지·바이너리 검사 3,451개 blob, Git 이력 30커밋 비밀값 검사, 비공개 상태·main 병합 경로 검사를 통과했다. Android 착수 전 `5bc233e`와 Apple·공용 코드·자원·기존 검사 2,978개 파일이 동일하다.

## 후속 정리

기존 `fix/hud-title-presentation`의 4커밋과 미커밋 문서·TODO·검사 도구 36개 파일을 별도 `chore/post-release-hygiene` 브랜치에 보존한 뒤 main으로 리베이스했다. 충돌은 인계·README·상태·문서 연결표·TODO에서만 발생했다. Android 완료·첫 릴리스 안내와 기존 미완료 요구를 함께 남겼다. 공유 브랜치의 이력이나 태그는 바꾸지 않았다.

제품 코드·에셋·기존 게임 검사는 리베이스 전 `410fdb8`과 같다. Android·Apple·Modules는 main과 같다. 후속 브랜치에서 main과 다른 제품 파일은 타이틀의 Menu.cpp·TitleOutpaint.png/json과 기존 MenuTests.cpp다. 이 타이틀 작업은 v0.1에 포함하지 않았다. 기존 HUD 늘리기는 철회한 상태를 유지한다.

문서 80개의 링크·앵커·업무 경로·스킬 사본, Python 도구 검사 11개, 공용 코어 41개 목록·플랫폼 경계·OpenAL 원본 317파일 검사와 diff 공백 검사를 통과했다. 이번 정리에서는 게임 코드·에셋을 새로 수정하거나 기기 앱을 재설치하지 않았다.

원래 checkout의 미커밋 사본과 Xcode GUI 스킴 메타데이터는 보존했다. `.local/post-release-preservation`에는 원래 diff와 복사 목록이 있다. 후속 작업 위치는 `.local/worktrees/release-followup`이다.
