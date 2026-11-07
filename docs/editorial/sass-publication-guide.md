# Sass 세션용 MachDefense 집필 안내

2026-11-07 갱신. Apps는 기술·기능·수정·검사, Mags는 개발자의 변과 인터뷰를 중심으로 쓴다. 이 안내는 원고와 근거를 찾는 진입점이다. Apps·Mags 본문 작성과 sass.studio 반영은 Sass 담당 세션이 맡는다.

## 먼저 읽을 문서

1. [개발자의 변 최종본](../RESTORATION.md)과 [인터뷰 원문](developer-interview-20261106.md).
2. [현재 게임 소개](../../README.md), [추가 기능](../public/features.md), [v0.5 배포 결과](../evidence/v05-release-20261107.md).
3. [현재 TODO](../usability-todo.md)와 [문서 점검 결과](../evidence/document-hygiene-20261107.md).

v0.3 [원고 모음](github-manuscripts-20261106.md)은 과거 사본이다. 그 안의 개발자의 변을 최신 최종본 위에 덮어쓰지 않는다. 게임 코드·자료를 확인할 때도 최신 작업 브랜치와 main의 차이를 먼저 본다.

## Apps: 주제와 근거

| 주제 | 설명할 내용 | 자료 |
|---|---|---|
| 원작 복원 | 호환 실행판을 비교 기준으로 새 C++ 게임을 구현한 과정 | [개발 과정](../public/development-history.md), [원본 대조](../evidence/port-source-parity-20261104.md) |
| 플랫폼·모듈 | Core·Audio·SaveTransfer, Apple·Android·Web 연결과 공용 게임 버튼 | [구조](../public/architecture.md), [모듈](../../Modules/README.md), [Android](../evidence/android-native-20261106.md) |
| 반복 긴급지원 | 사용자가 결제 대체와 자금난 완화를 위해 구상한 30/45/60초 지원 | [기획·수치](../public/features.md), [산식](../emergency-resupply-economy.md), [기기 검사](../evidence/emergency-resupply-20261105.md) |
| 코드 기반 복원 | 서버·계정 없이 5슬롯을 MD1 코드로 이전. 슬롯 화면의 공용 버튼과 플랫폼 텍스트 입력·저장 | [사용법](../public/gameplay.md), [플랫폼 검사](../evidence/save-transfer-all-platforms-20261106.md), [추가 레벨 복원](../evidence/level10-recovery-20261107.md) |
| 강화·생산 HUD | 10레벨 확장, 실제 피해·체력·사거리, 전체 설치 목록 스크롤, 레이저 해금 | [수치 계획](../research/level10-plan-20261107.md), [사거리 검사](../evidence/level10-range-damage-20261107.md), [레이저 해금](../evidence/laser-turret-unlock-20261107.md) |
| 그림 제작 | 원작 크기·부품 비례 대조, 채택·기각 경위, Lv.5 무장과 새 몸체 조합 | [최종 몸체 연결](../evidence/turret-body-only-integration-20261107.md), [자원 출처](../../Art/Level10/20261107/runtime-art.json) |
| 트러블슈팅 | 오디오 컨텍스트 반복 연결·메인 스레드 호출·복귀 후 효과음 소실과 UI 연결 누락 | [공개 원고](../public/troubleshooting.md), [세부 기록](../troubleshooting.md) |
| 검증 방식 | 포팅판→재구현→별도 치트앱, 실제 게임 경로의 대량 조건 검사와 직접 플레이 | [검사 결과](../public/validation.md), [사용자 글감](test-method-material-20261107.md) |
| 웹·PWA 운영 | 설치·오프라인·IndexedDB 저장, 첫 업데이트에서 구버전 자원이 남은 문제 | [v0.5 운영 결과](../evidence/v05-release-20261107.md), [후속 계획](../research/pwa-tauri-updates-20261107.md) |

Tauri Mac은 아직 미구현이다. 신규 락온도 기획 단계다. HeavyMach1의 Tauri 앱이나 fastutlego의 HeavyMach1 PR을 MachDefense에 직접 기여된 코드·출시 결과로 서술하지 않는다.

## Mags: 원문과 질문

- Mach 시리즈 전체를 복원하려던 이유, HeavyMach1의 첫 PR 경험, HeavyMach2보다 MachDefense를 먼저 택한 이유는 [인터뷰](developer-interview-20261106.md)와 최종 개발자의 변을 함께 읽는다.
- 폭음·폭발을 관전하는 재미, 결제 대신 반복 지원을 구상한 이유는 [README](../../README.md)와 [추가 기능](../public/features.md)에 있다. 개발자의 변에서 사용자가 옮기라고 한 문단을 다시 넣지 않는다.
- 「나는 이렇게 테스트했다」「나는 이렇게 일을 시켰다」는 [글감 원문](test-method-material-20261107.md)에 있다. p99 16.67ms·25ms 초과 0회는 해당 기기·빌드·측정 구간을 함께 쓰며 모든 장면의 성능으로 확대하지 않는다.
- 기획·직접 플레이·판단은 저자, 구현·검사는 Codex의 수행으로 구분한다. 인터뷰에 없는 답변·감정·비유를 새로 만들지 않는다. 최신 사용자 문구와 강한 비판을 임의로 순화하지 않는다.

## 이미지와 캡처

| 자료 | 사용처와 주의점 |
|---|---|
| [Android Stage33](../evidence/images/android-20261106/normal-iphone-stage33.png), [강화 화면](../evidence/images/android-20261106/normal-iphone-upgrades.png) | Android 정식 앱. 파일명의 iphone은 이전한 저장의 출처이며 촬영 플랫폼이 iPhone이라는 뜻이 아니다. |
| [기존 복원 캡처](assets/machdefense/) | 포팅판·소스판·치트 화면이 섞여 있다. [통합 보고 원고](machdefense-report.md)의 해당 설명과 대조해 캡션을 작성한다. |
| [추가 레벨 그림 명세](../../Art/Level10/20261107/runtime-art.json) | 최종 채택본과 출처·해시. 같은 Art 폴더의 이전 무장 시안과 기각된 타이틀을 현행 이미지로 쓰지 않는다. |
| [통합 보고 사이트](https://heavymach-dev-report-20261025.hiphopxy.chatgpt.site/machdefense.html) | 과거/현재 검수 화면이 누적돼 있다. 각 절의 날짜·빌드·채택 여부를 확인한다. |

추가 이미지·GIF가 필요하면 플랫폼·정식/치트·빌드·장면·원본 위치·해시·캡션을 함께 정리한다. 개인 저장과 복원 코드가 화면에 노출되지 않게 확인한다. 공유하도록 지정한 Quick Play 코드는 [공유 원고](../public/quick-play.md)를 사용한다.

## 한·영 원고와 갱신

방문자 문서 29쌍을 작성했다. 각 문서 제목 아래의 한국어·English 링크로 같은 원고를 전환한다. [대응 목록](../translations/scope.json)과 [검토 해시](../translations/manifest.json)에 전체 경로를 기록했다.

| 주제 | 한국어 | English |
|---|---|---|
| 게임 소개 | [README](../../README.md) | [README](../../README.en.md) |
| 개발자의 변 | [최종본](../RESTORATION.md) | [Developer's statement](../en/RESTORATION.md) |
| 기능·기획 | [추가 기능](../public/features.md) | [Features](../en/public/features.md) |
| 기술·플랫폼 | [기술 구조](../public/architecture.md) | [Architecture](../en/public/architecture.md) |
| 개발 과정 | [과정](../public/development-history.md) | [Development](../en/public/development-history.md) |
| 트러블슈팅 | [수정 기록](../public/troubleshooting.md) | [Troubleshooting](../en/public/troubleshooting.md) |
| 검증 | [검사 결과](../public/validation.md) | [Validation](../en/public/validation.md) |
| 현재·후속 작업 | [상태](../public/implementation-status.md) · [과제](../public/roadmap.md) | [Status](../en/public/implementation-status.md) · [Roadmap](../en/public/roadmap.md) |
| 전체 목차 | [문서](../public/README.md) | [Documentation](../en/public/README.md) |

[코드 라이선스](../../LICENSE-SCOPE.md) · [영문](../../LICENSE-SCOPE.en.md), [자원 조건](../../ASSET-LICENSES.md), [외부 고지](../../THIRD_PARTY_NOTICES.md)도 준비했다. 원작 자원까지 MIT라고 서술하지 않는다.

공개 기능·기획·수정 결과가 바뀌면 해당 원고의 양쪽 언어, 검사 근거, 채택 이미지와 미완료 항목을 함께 갱신한다. 개발 정본·스킬·인터뷰 원문과 과거 검사 기록은 한국어로 유지한다. 번역문이 내부 개발 기준을 바꾸지 않는다.

Sass 허브 서신은 `ProjectPersonalFunnelPage/governance/operations/machdefense-apps-mags-handover-2026-11-07.md`에 작성했다. Apps/Mags 원고 작성과 sass.studio 반영은 그 세션에서 진행한다. 비공개 Sass·Candidate 자료를 공개 게임 저장소로 복사하지 않는다.

[문서 목차](../public/README.md) · [릴리스 이력](../../CHANGELOG.md)
