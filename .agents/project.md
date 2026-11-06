# MachDefense 요구

2026-11-04 사용자 지시: 새 비공개 저장소에 HeavyMach 1의 스킬·절차·문서 체계를 갖춘다. 후속 지시로 iOS 원본 호환 실행판 개발을 시작했다. 실행 결과는 docs/current-status.md에서 확인한다.
원본 기준 복원→호환 실행 비교→소스 재구현→동일 조건 검증을 따른다. 기존 hm-defense는 보존만 한다. HeavyMach 1·Unity 소스와 저장을 변경하지 않는다.
IPA는 저장소 밖 원본 보관소에서 읽는다. `docs/source-provenance.md`와 `docs/repository-management.md`의 외부 반출 금지를 따른다. 준비된 에셋의 추후 반입은 원본 패키지·실행 바이너리와 분리해 파일별 출처와 해시를 남긴다.

## 광고 표시 금지 — 2026-11-04

사용자 지시: HeavyMach 1·HeavyMach 2·MachDefense 모두 광고를 표시하지 않는다. 원본 호환판과 소스 재구현판, 모든 플랫폼에 적용한다. 광고 배너·전면 광고·보상형 광고를 추가하지 않고 광고를 요청하지 않는다. 원본에 광고 API 호출이 있으면 광고를 표시하지 않는 호환 처리를 마련한다. 원본 IPA는 수정하지 않는다.

## 공개 배포 — 2026-11-06

사용자가 저장소 공개 전환과 웹·PWA 운영 배포를 지시했다. 정식 웹 산출물은 gh-pages 브랜치와 https://gelb2.github.io/MachDefense/ 에 배포한다. 소스는 작업 브랜치→develop→main 순서로 병합한다. IPA·사용자 저장 반출 금지는 유지한다.
