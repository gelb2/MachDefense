# MachDefense 인계 — 2026-11-04

위치: `/Users/sokol/Git/ProjectGameDev-Hub/MachDefense`. 독립 비공개 저장소 `gelb2/MachDefense`다. 시작 문서는 AGENTS.md와 docs/canonical.md이며 현재 결과는 docs/current-status.md에 있다.

후속 사용자 지시로 feat/ios-compatibility에서 원본 1.0.5의 iOS 호환 실행판을 개발 중이다. Compatibility/README.md와 docs/current-status.md를 먼저 읽는다. Mesabloo 포크 기반 개발이 아니다.

IPA 19개는 저장소 밖 원본 보관소에 있다. 파일을 저장소·원격·보고 사이트에 복사하지 않는다. `.local/originals.json`은 보관 경로만 담고 Git 제외다. 실제 스킬은 `.agents/skills`, HeavyMach 1 원본은 `docs/reference/heavymach1`에 있다.

새 clone에서는 tools/setup.py로 훅을 설치한다. docs/repository-management.md의 브랜치·반출 규칙을 따른다. HeavyMach 1·Unity·기존 hm-defense와 사용자 저장을 보존한다. 자동 새 세션 전환이나 별도 인계 승인을 요구하지 않는다.

HeavyMach 1·2·MachDefense 모두 광고를 표시하거나 요청하지 않는다. 호환판에도 적용한다. 원본 IPA는 수정하지 않고 광고 API는 로컬 실행기에서 비표시 처리한다.

## 호환판 구현·검사 인계

- 브랜치: `feat/ios-compatibility`. develop·main 병합과 원격 push는 수행하지 않는다.
- 원본: Mach Defense 1.0.5의 ARMv7 실행 파일을 touchHLE 인터프리터로 구동한다. 소스 재구현은 아직 시작하지 않았다.
- 완료: 시뮬레이터 슬롯 생성·Stage 1 진입·유닛 생산/이동, iPhone Air 무선 설치·타이틀/슬롯 화면 확인. 슬롯 생성 때 종료되던 빈 이미지 처리와 Xcode 서명 재실행 문제를 수정했다.
- 최신 수정본의 아이폰 설치는 성공했다. 도구 실행은 잠금으로 실패했다. 사용자가 아이폰에서 직접 플레이하기로 했으므로 그 기기의 앱을 종료하거나 재설치하지 않는다.
- 남은 문제: 이름 입력칸 포커스 후 위치 이동. 구버전 API/키보드 알림 문제라는 사용자 추정과 합성 경로 전환을 함께 조사한다. 아직 원인을 확정하지 않았다.
- 남은 검사: 교전·미션 완료·저장·재실행, iPad 실기기, 음향·성능. 기존 증거를 다시 완료로 확대하지 않는다.
- 현재 빌드/런타임은 `/Users/sokol/Documents/Codex/Builds/MachDefense/compat-ios-20261104`에 있다. 기기용 GUI 빌드는 Xcode DerivedData에 있다. 경로·원본은 `.local/compatibility.json`, 검사 로그는 `.local`에 보관한다.
- 런타임 수정 뒤 `Compatibility/patches/machdefense.patch`도 갱신하고 역적용 검사로 사본과 일치하는지 확인한다. 원본과 HeavyMach 1 사본은 수정하지 않는다.
