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

Tauri Mac은 같은 웹 게임과 파일 저장으로 구현했고 실기기 결과와 함께 v0.6을 배포했다. [운영 결과](../evidence/v06-release-20261107.md). [Mac 검사](../evidence/tauri-desktop-20261107.md)와 [PWA 갱신 검사](../evidence/pwa-update-20261107.md)를 함께 읽는다. 신규 락온은 기획 단계다. HeavyMach1의 Tauri 앱이나 fastutlego의 HeavyMach1 PR을 MachDefense에 직접 기여된 코드·출시 결과로 서술하지 않는다.

## Mags: 원문과 질문

- Mach 시리즈 전체를 복원하려던 이유, HeavyMach1의 첫 PR 경험, HeavyMach2보다 MachDefense를 먼저 택한 이유는 [인터뷰](developer-interview-20261106.md)와 최종 개발자의 변을 함께 읽는다.
- 폭음·폭발을 관전하는 재미, 결제 대신 반복 지원을 구상한 이유는 [README](../../README.md)와 [추가 기능](../public/features.md)에 있다. 개발자의 변에서 사용자가 옮기라고 한 문단을 다시 넣지 않는다.
- 「나는 이렇게 테스트했다」「나는 이렇게 일을 시켰다」는 [글감 원문](test-method-material-20261107.md)에 있다. p99 16.67ms·25ms 초과 0회는 해당 기기·빌드·측정 구간을 함께 쓰며 모든 장면의 성능으로 확대하지 않는다.
- 기획·직접 플레이·판단은 저자, 구현·검사는 Codex의 수행으로 구분한다. 인터뷰에 없는 답변·감정·비유를 새로 만들지 않는다. 최신 사용자 문구와 강한 비판을 임의로 순화하지 않는다.

## 사용자 플레이 자료 — 2026-11-07

[한글 캡처 모음](../public/gameplay-gallery.md) · [English](../en/public/gameplay-gallery.md) · [장면 명세](../public/media/gameplay-20261107/scenes.json).

바탕화면에서 받은 아이폰 녹화 2개와 중복 제외 이미지 7개를 정리했다. 공개 파일은 정지 이미지 11장·GIF 2개·MP4 3개다. `combat`은 포탑 사격·메크 교전, `special-attack`은 기존 특수공격 지정·미사일 발사, `resupply`는 긴급지원 수령이다. GIF는 8초·12fps, MP4는 각각 12/10/9초·30fps다. 비율과 재생 속도를 유지했다. 원본은 1260×2736이고 MP4는 630×1368, GIF는 420×912다.

Apps에서 원작 전투와 추가 긴급지원 기능을 설명할 때 같은 장면 ID를 쓴다. Mags에서 폭음과 폭발을 관전하는 재미를 설명할 때는 전투 클립을 쓴다. 기존 특수공격을 미구현 락온 기능의 장면으로 쓰지 않는다. 영상의 소리는 원본 트랙을 AAC로 재인코딩했고 GIF에는 소리가 없다.

2026-11-07 후속 지시로 플레이 캡처 모음에서 버그 제보 2장을 제외했다. 현재 소개용 정지 이미지는 9장, GIF는 2개, MP4는 3개다. README에는 전투 GIF 2개와 Stage 40 전투 화면을 배치했다. `iphone-stage23-hud`와 `pathfinding-report`는 소개용으로 쓰지 않고 버그 조사 자료로 보존한다. 현재 버전의 문제 해결 여부를 보여주는 캡처로 쓰지 않는다. 촬영 파일의 정확한 빌드 번호는 없다. 색·도트·부품 비교에는 압축된 공개 JPEG보다 로컬 원본을 사용한다.

로컬 원본은 MachDefense 저장소 루트의 `.local/media-originals/20261107/`에 보관했다. 바탕화면 원본도 그대로다. Git 제외를 확인했고 원본 MOV와 개인 저장 파일은 공개하지 않는다. 명세에는 원본 이름·SHA-256·장면 시각·한영 캡션·파생 파일 크기/해시가 있다. 재추출은 `tools/extract_gameplay_media.py --originals <원본 폴더>`로 한다.

## 이미지와 캡처

| 자료 | 사용처와 주의점 |
|---|---|
| [Android Stage33](../evidence/images/android-20261106/normal-iphone-stage33.png), [강화 화면](../evidence/images/android-20261106/normal-iphone-upgrades.png) | Android 정식 앱. 파일명의 iphone은 이전한 저장의 출처이며 촬영 플랫폼이 iPhone이라는 뜻이 아니다. |
| [기존 복원 캡처](assets/machdefense/) | 포팅판·소스판·치트 화면이 섞여 있다. [통합 보고 원고](machdefense-report.md)의 해당 설명과 대조해 캡션을 작성한다. |
| [추가 레벨 그림 명세](../../Art/Level10/20261107/runtime-art.json) | 최종 채택본과 출처·해시. 같은 Art 폴더의 이전 무장 시안과 기각된 타이틀을 현행 이미지로 쓰지 않는다. |
| [통합 보고 사이트](https://heavymach-dev-report-20261025.hiphopxy.chatgpt.site/machdefense.html) | 과거/현재 검수 화면이 누적돼 있다. 각 절의 날짜·빌드·채택 여부를 확인한다. |

추가 이미지·GIF가 필요하면 플랫폼·정식/치트·빌드·장면·원본 위치·해시·캡션을 함께 정리한다. 개인 저장과 복원 코드가 화면에 노출되지 않게 확인한다. 공유하도록 지정한 Quick Play 코드는 [공유 원고](../public/quick-play.md)를 사용한다.

## 한·영 원고와 갱신

방문자 문서 32쌍을 작성했다. 각 문서 제목 아래의 한국어·English 링크로 같은 원고를 전환한다. [대응 목록](../translations/scope.json)과 [검토 해시](../translations/manifest.json)에 전체 경로를 기록했다.

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

## 작업 지시·치트 검사 완성 원고 — 2026-11-07

[나는 이렇게 일을 시켰다](../public/work-method.md) · [English](../en/public/work-method.md) · [나는 이렇게 테스트했다](../public/testing-method.md) · [English](../en/public/testing-method.md).

오디오 컨텍스트·메인 스레드·복귀 후 무음, Android 보존 파일 2,978개, 80미션 검사의 범위, 10레벨·생산 HUD 검사와 실제 iPad 프레임 수치를 담았다. 원문은 [집필 자료](test-method-material-20261107.md)에 보존했다. Apps는 각 문단의 구현·검사 근거를, Mags는 저자의 지시와 경험을 사용한다. HeavyMach의 사격별 저장 오류를 MachDefense에서도 발생한 것처럼 쓰지 않는다.

[MachDefense 통합 보고서](https://heavymach-dev-report-20261025.hiphopxy.chatgpt.site/machdefense.html#work-test-20261107)에서 한·영 전문을 함께 볼 수 있다.

## 2026-11-07 PWA·Tauri 후속

- [PWA 업데이트 검사](../evidence/pwa-update-20261107.md): 파일 검증, 변경 자원 조각 재사용, 실패 시 기존 빌드 유지.
- [Tauri Mac](../evidence/tauri-desktop-20261107.md): 같은 웹 게임을 사용하고 파일 저장·복원·종료·서명 업데이트를 연결했다. 자동 검사와 아직 확인하지 않은 GUI·운영 조건을 구분한다.
- HeavyMach 웹 PR 기여자는 [fastutlego](https://github.com/fastutlego), [PR #24](https://github.com/gelb2/HeavyMach/pull/24)다. 이 기여와 후속 PWA·Tauri 작업이 MachDefense 확장에도 토대가 됐다. README·Web·Desktop/Tauri·CREDITS의 한영 문구를 사용한다.
- [레이저 크기·HD·저장 경계](../evidence/laser-size-hd-save-boundaries-20261107.md), [공중유닛 시안](../research/aircraft-plan-20261107.md). 시안은 게임에 들어간 유닛으로 소개하지 않는다.

## 신규 유닛 외형 작업 기록 — 2026-11-08

[신규 유닛 외형 오류](../evidence/new-unit-art-errors-20261108.md)에 추가 레벨 포탑·헬기·VTOL의 사용자 지적, 승인 그림 교체, 크기 기준 오적용, 색·질감·조립 오류와 마지막 이미지 생성 차단 응답을 모았다. 승인 시안을 적용하겠다고 답한 뒤 다른 그림을 적용하고, 승인 시안이 반영된 것처럼 보고한 문제도 별도 항목에 기록했다. 이를 단순한 그림 품질 문제로 축소하지 않는다. 건쉽은 미완료다. 자동검사 통과를 미술 완성이나 사용자 승인으로 서술하지 않는다. 한·영 공개 요약은 트러블슈팅에 연결했다.

## 2026-11-08 도안 작업 거부에 관한 원고

[헤비멕1과는 다른 위화감 — 신 리셴코주의와 차우세스쿠의 타자기](../public/unease-and-typewriters.md) · [English](../en/public/unease-and-typewriters.md). Gemini 도안 작업과 텍스트 편집 거부, 경위서의 책임 전가를 [경위서](../evidence/aircraft-instruction-refusal-20261108.md)와 [트러블슈팅](../troubleshooting.md)에 연결했다. 리셴코주의 역사 연구와 루마니아 법령 원문을 인용했다. 역사적 제도의 설명과 저자의 비유를 구분했다. 기존 개발자의 변은 수정하지 않았다.

2026-11-08 추가: [사용자 추가 원문과 편집 내역](unease-addition-source-20261108.md)을 보존했다. 문화적 거리·80%·48시간·세 서비스 비판을 한·영 원고에 반영했고 HeavyMach1의 기존 글과 연결했다.

헬기 기획 추가: 같은 원고에 원작 보존과 최소 변경, 비유도탄 회피·일시 무적, 저자가 시험한 생존 결과와 최소 기능 충족 판단, 아트 작업 거부로 인한 출시 불확실성을 넣었다. [추가 글감 원문과 근거](unease-addition-source-20261108.md#추가-글감-원문--헬기-기획과-출시-불확실성)를 함께 읽는다.

공개 기록 이유 추가: 한·영 원고에 직접 개발할 때와 AI 업무 거부의 차이, 11월 8일 01~06시의 경험과 저자의 결론을 반영했다. [추가 원문](unease-addition-source-20261108.md#추가-원문--개발-중인-업무-거부를-공개적으로-쓰는-이유)을 보존했다.

기술과 도덕적 독선 추가: 한·영 원고에 본사 직원의 예상 답변, 과학을 도구로 삼아 비대한 에고를 비합리적으로 강제할 수 있다는 논거, 기업 보호와 게임 폭력 묘사에 대한 비판을 넣었다. [사용자 원문과 역사 확인 내역](unease-addition-source-20261108.md#역사-확인과-반영-내역)을 함께 읽는다. 라이스의 트로츠키주의 이력은 근거를 찾지 못해 본문의 역사적 사례에서 제외했다.

논지 정정: ‘과학적 사회주의’를 복원하고 코롤료프 투옥 사례를 삭제했다. [사용자의 정정과 반영](unease-addition-source-20261108.md#논지-변경에-대한-정정)을 우선한다. 과학자 탄압 사례로 저자의 논거를 대체하지 않는다.

주어진 수명과 분노 추가: 한·영 원고의 마지막 문단에 남은 수명, 구상한 릴리즈·디자인 지시의 거부, 삼류도덕과 신인류인 척하는 타자기에 대한 저자의 분노를 반영하고 개발자의 변에 연결했다. [추가 원문](unease-addition-source-20261108.md#추가-글감-원문--주어진-수명과-업무-거부에-대한-분노)을 보존했다.

## 통합 초안 — 2026-11-08

사용자의 “다 되면 글 초안 만들어” 지시에 따라 [한국어 통합 초안](unease-and-typewriters-draft-20261108.md)을 작성했다. 기존 한·영 원고와 원문 기록은 보존했다. 문화적 거리와 80%·48시간의 회고 → 헬기 기획과 최소 기능 → 도안·텍스트 편집 거부 → 경위서의 책임 전가 → 과학을 도구로 삼는 독선 → 차우세스쿠의 타자기 → 남은 시간과 분노 순서로 구성했다. 중복 설명과 Codex가 덧붙인 방어적 문장을 줄였으며 사용자 원문의 강한 표현을 유지했다. 새 초안은 한국어이며 기존 영문 원고와 별도 판본이다.
