# Sass 세션용 MachDefense 집필 안내

2026-11-11 갱신. 집필은 **MachDefense 저장소 main의 최신 문서**에서 시작한다. 게임 릴리즈는 **v0.7**이며, 이후 main과 운영 웹에 반영한 핫픽스·원고·플레이 캡처를 함께 읽는다. Apps는 기능·구조·수정·검사, Mags는 개발자의 변과 인터뷰·작업 경험을 중심으로 쓴다. Apps·Mags 원고 작성과 sass.studio 반영은 Sass 담당 세션이 맡는다.

## 읽는 순서

1. [개발자의 변](../RESTORATION.md)과 [인터뷰 원문](developer-interview-20261106.md): Mach 시리즈 전체 복원 의도, 구매·플레이 경험과 MachDefense를 먼저 택한 이유.
2. [README](../../README.md), [추가 기능](../public/features.md), [현재 구현 상태](../public/implementation-status.md): 원작 기능과 새 기획, 플랫폼별 구현·배포 범위.
3. [v0.7 릴리즈와 실제 설치 결과](https://github.com/gelb2/MachDefense/releases/tag/v0.7), [변경 이력](../../CHANGELOG.md), [후속 배포 확인](../evidence/sass-guide-refresh-20261111.json): 태그의 배포 파일과 이후 핫픽스를 구분한다.
4. [헬기 통합 원고](../public/unease-and-typewriters.md), [글감·후속 수정 20항목](helicopter-writing-inventory-20261110.md), [현재 남은 과제](../public/roadmap.md): 미완성 출시와 개선 보류, 사용자가 직접 고친 본문을 확인한다.
5. [플레이 캡처 모음](../public/gameplay-gallery.md)과 아래 미디어 표: 실제 장면·파일·캡션·원본 시각을 맞춘다.
6. [구조](../public/architecture.md), [개발 과정](../public/development-history.md), [트러블슈팅](../public/troubleshooting.md), [검사 결과](../public/validation.md): 각 설명을 날짜·기기·빌드별 근거와 대조한다.

## 현재 릴리즈와 후속 변경

| 범위 | 현재 내용 | 근거 |
|---|---|---|
| v0.7 릴리즈 | 헬기 4종·Lv.1–10, 무장·긴급회피·채프/플레어와 채택 타이틀 외곽을 포함한다. 헬기는 미완성 상태로 출시했고 개선은 보류했다. | [릴리즈](https://github.com/gelb2/MachDefense/releases/tag/v0.7), [출시 전 검사](../evidence/v07-release-20261110.md) |
| 배포 플랫폼 | iOS/iPadOS 유니버설 소스, Android 정식·치트 APK, 웹/PWA, Tauri Mac. Android APK는 개발용 서명, Mac 앱은 ad-hoc 서명이며 macOS 14+ Apple Silicon 대상이다. | [설치](../public/build-and-install.md), [구현 상태](../public/implementation-status.md) |
| 강화 목록 핫픽스 | 헬기 추가 뒤 목록과 상세창이 겹치는 문제를 수정했다. 긴 화면은 전체 행을 표시하고, 작은 화면은 목록 안에서 스크롤한다. | [수정과 웹 검사](../evidence/upgrade-layout-hotfix-20261110.md) |
| 체력·가격 핫픽스 | 샤이엔 HP는 같은 레벨 플라즈마 메크의 85%, 벨루가는 플라즈마 메크·탱커 중간값. 구매 셀·강화 크레딧은 모두 25% 인하했다. | [Lv.1–10 수치·검사](../evidence/aircraft-hp-price-hotfix-20261110.md) |
| 후속 운영·설치 | 정식 웹 빌드 f9f26e94f1d4e3d6의 갱신·테이블·저장·오프라인을 확인했다. 후속 main으로 iPhone 정식 Release 0.7(7)을 재설치했고 진행 필드를 보존했다. v0.7 태그의 Android·Mac 배포 파일과 후속 iPhone 설치본을 같은 파일로 서술하지 않는다. | [배포·설치 요약](../evidence/sass-guide-refresh-20261111.json) |
| 최신 소개 자료 | README에 원작에 추가한 기능을 구분했고, Stage 55 헬기 전투 캡처를 원본 PNG 그대로 게시했다. | [README](../../README.md), [캡처와 출처](../public/media/gameplay-20261110/scenes.json) |

v0.7 출시 전 코어 검사는 58개 중 56개 통과, AircraftAudit·AircraftCampaign 2개 실패였다. 후속 핫픽스의 관련 검사 통과를 전체 코어·미술·모든 기기의 성능 통과로 바꾸지 않는다. 미완성인 샤이엔 새 외형·에셋 적용, 도색·입체감·선회·표적 접근/사거리·성능 과제는 [보류 목록](../public/roadmap.md)에 남아 있다. 신규 락온은 기획 단계다.

## Apps: 설명할 기능과 근거

| 주제 | 설명할 내용 | 자료 |
|---|---|---|
| 원작 복원 | 비교 실행을 기준으로 C++ 게임과 원작의 전투·성장·저장·음향을 맞춘 과정 | [개발 과정](../public/development-history.md), [원본 대조](../evidence/port-source-parity-20261104.md) |
| 공용 구조·플랫폼 | Core·Audio·SaveTransfer와 Apple·Android·Web 연결. PWA와 Tauri는 같은 웹 게임을 사용한다. | [구조](../public/architecture.md), [모듈](../../Modules/README.md), [Mac 검사](../evidence/tauri-desktop-20261107.md) |
| 반복 긴급지원 | 결제 대체와 자금난 완화를 위해 구상한 30/45/60초 반복 지원 | [기능](../public/features.md), [산식](../emergency-resupply-economy.md), [실제 검사](../evidence/emergency-resupply-20261105.md) |
| 복원 코드 | 서버·계정 없이 프로필 5개를 한 줄 MD1 코드로 옮긴다. 공용 메뉴와 플랫폼 입력·저장을 쓴다. | [사용법](../public/gameplay.md), [플랫폼 검사](../evidence/save-transfer-all-platforms-20261106.md), [추가 레벨 복원](../evidence/level10-recovery-20261107.md) |
| 10레벨·생산 목록 | 원작 상한 이후 강화 수치·가격·채택 몸체, 전체 설치 목록 스크롤·레이저 해금 | [기능](../public/features.md), [추가 단계](../research/level10-plan-20261107.md), [몸체 연결](../evidence/turret-body-only-integration-20261107.md) |
| 헬기 4종 | 스카웃·샤이엔·벨루가·AH-10의 선회·회피·기존 무장/효과 재사용. 출시한 기능과 미완성 외형·보류 과제를 함께 설명한다. | [기능](../public/features.md), [Android 40레벨 검사](../evidence/android-aircraft-20261109.md), [보류 이유](../public/unease-and-typewriters.md) |
| 타이틀·강화 UI | 원본 중앙 그림 보존, 자연물로 이어진 외곽, 유닛 목록·상세창의 겹침 수정 | [타이틀 구현](../evidence/title-integration-20261110.md), [강화 UI](../evidence/upgrade-layout-hotfix-20261110.md) |
| 수정·검증 | 오디오 컨텍스트 반복 연결·메인 스레드 호출·복귀 후 무음, 치트앱과 직접 플레이로 찾은 차이 | [트러블슈팅](../public/troubleshooting.md), [나는 이렇게 일을 시켰다](../public/work-method.md), [나는 이렇게 테스트했다](../public/testing-method.md) |
| 웹 갱신 | 자원 해시·길이 검사, 변경 조각 재사용, 실패 시 기존 빌드와 진행 보존 | [PWA 검사](../evidence/pwa-update-20261107.md), [후속 운영 확인](../evidence/sass-guide-refresh-20261111.json) |

HeavyMach 웹판 기여자는 [fastutlego](https://github.com/fastutlego), 기여 경로는 [HeavyMach PR #24](https://github.com/gelb2/HeavyMach/pull/24)다. 그 웹 실행부와 후속 PWA·Tauri 작업이 MachDefense 확장에 토대가 된 경위를 [README](../../README.md)·[크레딧](../../CREDITS.md)에 맞춰 쓴다.

## Mags: 현재 원고와 원문

| 이야기 | 읽을 본문 | 원문·후속 기록 |
|---|---|---|
| 시리즈 전체 복원과 MachDefense를 택한 이유 | [개발자의 변](../RESTORATION.md) | [인터뷰 원문](developer-interview-20261106.md) |
| 폭음·폭발을 관전하는 재미와 결제 대체 기획 | [README](../../README.md), [긴급지원](../public/features.md) | [인터뷰](developer-interview-20261106.md) |
| 직접 플레이·지시·검사 과정 | [작업 지시](../public/work-method.md), [테스트](../public/testing-method.md) | [사용자 글감](test-method-material-20261107.md) |
| 헬기 미완성 출시와 개선 중단 | [현재 통합 원고](../public/unease-and-typewriters.md) | [글감·후속 수정 20항목](helicopter-writing-inventory-20261110.md), [통합 내역](unease-integration-20261110.md), [경위서](../evidence/aircraft-work-failure-20261109.md) |
| 별도 방법론 | [AI를 패는 방법론·쇼펜하우어 토론 기술 글감](ai-argument-method-material-20261109.md) | 사용자가 분리한 원고다. 헬기 통합 글에 다시 합치지 않는다. |

현재 헬기 글의 제목은 **「헬기유닛 미완성 릴리즈에 대한 소고. 폭력적인 디자인을 만들 수 없다는 Codex의 저열한 성능에 대한 관측 기록」**이다. 사용자가 11월 10일 로컬 에디터에서 고친 본문과 이후 채팅 정정을 우선한다. 아머드코어 문장은 **“아머드코어의 주제 중 하나가 니체의 위버멘쉬이고”**다. [현재 한국어](../public/unease-and-typewriters.md)·[English](../en/public/unease-and-typewriters.md)가 집필 기준이다.

[11월 8일 초안](unease-and-typewriters-draft-20261108.md), [11월 9일 작업 기록](helicopter-work-record-20261109.md), [이전 글감 묶음](aircraft-refusal-materials-20261109.md)과 [v0.3 원고 모음](github-manuscripts-20261106.md)은 당시 자료다. 글감 수집본도 현재 채택 본문과 별개다. 사용자가 현행 본문에서 삭제한 문장을 과거 자료에서 다시 넣지 않는다. 이미지 생성 오류 7건은 현재 본문의 전문과 경위서에 연결한다.

기획·직접 플레이·판단은 저자, 분석·구현·검사는 Codex의 수행으로 구분한다. 인터뷰에 없는 답변·감정·비유를 만들지 않는다. 사용자의 비판·후속 정정·원문을 임의로 순화하지 않는다. p99나 프레임·검사 수치는 해당 날짜·기기·빌드·측정 구간을 함께 쓴다. [글쓰기 지침](../../.agents/skills/write-project-text/SKILL.md)을 따른다.

## 사용할 이미지와 영상

[한글 캡처 모음](../public/gameplay-gallery.md) · [English](../en/public/gameplay-gallery.md). 현재 소개용 정지 이미지는 **10장**, GIF는 **2개**, 소리 있는 MP4는 **3개**다. 기존 소개용 정지 이미지 9장에 Stage 55 PNG 한 장을 추가했다.

| 장면 ID·파일 | 장면과 사용처 | 원본·길이·명세 |
|---|---|---|
| stage55-helicopters · [PNG](../public/media/gameplay-20261110/iphone-stage55-helicopters.png) | 스카웃·샤이엔과 메크의 전투, 생산 목록의 헬기 4종. Apps의 신규 유닛 소개, Mags의 미완성 출시 기록 | IMG_1199.PNG, 1260×2736, 변형 없는 복사. [한영 캡션·해시](../public/media/gameplay-20261110/scenes.json) |
| combat · [GIF](../public/media/gameplay-20261107/combat.gif) · [MP4](../public/media/gameplay-20261107/combat.mp4) | 원작 포탑 사격·메크 교전. 원작 전투와 관전의 재미 | 11월 7일 녹화 02:38–02:50, MP4 12초·GIF 8초 |
| special-attack · [GIF](../public/media/gameplay-20261107/special-attack.gif) · [MP4](../public/media/gameplay-20261107/special-attack.mp4) | 기존 특수공격의 표적 지정·미사일 발사. 미구현 락온의 장면으로 쓰지 않는다. | 11월 6일 녹화 03:52–04:02, MP4 10초·GIF 8초 |
| resupply · [이미지](../public/media/gameplay-20261107/resupply.jpg) · [MP4](../public/media/gameplay-20261107/resupply.mp4) | 새로 기획한 긴급지원 수령과 영문 메시지 | 11월 6일 녹화 00:40–00:49, MP4 9초 |
| iphone-stage11/23/40/46, android-stage1, upgrade | 전투·전장 배치·플랫폼·성장 설명. [갤러리](../public/gameplay-gallery.md)에서 이미지별로 고른다. | [기존 장면 명세](../public/media/gameplay-20261107/scenes.json) |
| [Android Stage 33](../evidence/images/android-20261106/normal-iphone-stage33.png), [강화 화면](../evidence/images/android-20261106/normal-iphone-upgrades.png) | 별도 Android 정식 앱 검사 화면. 파일명의 iphone은 이전한 저장의 출처다. | 실제 촬영 플랫폼은 Android |

기존 MP4는 H.264/AAC·630×1368·30fps, GIF는 420×912·12fps다. 화면 전체 비율과 재생 속도를 유지했고 GIF에는 소리가 없다. 원본 영상은 11월 6일 4분 12초, 7일 4분 16초다. 11월 10일 바탕화면의 영상 두 개는 기존 명세와 해시가 같아 클립을 중복 생성하지 않았다.

iphone-stage23-hud와 pathfinding-report는 버그 제보 자료로 보존하며 소개용 10장에 포함하지 않는다. 촬영본의 정확한 빌드 번호는 없다. Stage 55 화면이나 옛 영상을 최신 체력·가격·모든 기기의 성능 검증 자료로 사용하지 않는다. 현재 수치는 핫픽스 표, 검사 범위는 해당 증거에서 확인한다.

### 로컬 원본과 재사용

11월 10일 제공 자료의 원본 영상 2개·이미지 7개는 `/Users/sokol/Documents/ProjectGameDev-Internal/media/MachDefense/user-gameplay-20261110/original/`에 보관했다. 그 상위의 `archive.json`에 파일명·해시와 기존 자료의 중복 대조가 있다. 이전 `.local/media-originals/20261107/`는 당시 작업 사본의 기록이며 현재 worktree에 같은 폴더가 있다고 가정하지 않는다. 공개된 파생 파일과 명세는 위 링크로 가져갈 수 있다.

새 캡처를 쓰면 장면 ID·한영 캡션·해시·원본 시각을 함께 가져간다. 화면 위에 문구·배지·새 게임 요소를 합성하거나 원본 비율을 바꾸지 않는다. 원본 MOV·개인 저장·기기 식별자·인증 정보는 공개하지 않는다. 공유하도록 지정한 복원 코드는 [Quick Play](../public/quick-play.md)를 사용한다. 예전 영상 전체를 다시 추출할 때는 해당 명세의 모든 원본을 준비하고 `tools/extract_gameplay_media.py --originals <원본 폴더>`를 사용한다.

## 한·영 원고와 편집

방문자 원고는 **35쌍**이다. [전체 대응 목록](../translations/scope.json)·[검토 해시](../translations/manifest.json)에 경로를 기록했다. [한국어 목차](../public/README.md)와 [English](../en/public/README.md)에서 같은 문서를 전환한다. 개발 지침·경위서·원문 수집 기록과 이 집필 안내는 한국어로 유지한다.

[로컬 에디터 반영 절차](local-editor-workflow.md)는 브라우저 IndexedDB와 내려받은 수정본을 사용한다. 사용자가 수정 통보를 하면 저장본을 대조해 GitHub에 반영한다. 서버 저장이나 저장 버튼의 자동 커밋·푸시를 도입하지 않는다. Sass는 최신 main 본문을 읽으며, 이전 에디터 저장본이나 과거 초안으로 덮어쓰지 않는다.

[코드 적용 범위](../../LICENSE-SCOPE.md), [자원 조건](../../ASSET-LICENSES.md), [외부 고지](../../THIRD_PARTY_NOTICES.md)를 구분한다. 원작 그림·음원까지 MIT라고 쓰지 않는다. 비공개 Sass·Candidate 자료를 이 공개 저장소로 복사하지 않는다.

[기존 통합 보고 사이트](https://heavymach-dev-report-20261025.hiphopxy.chatgpt.site/machdefense.html)의 절은 날짜·채택 여부가 섞여 있으므로 원문과 현재 main을 우선한다. 과거 Sass 허브 인계 서신은 11월 7일 전달 당시의 기록이다. 이번 안내 갱신을 Sass 세션의 집필 완료·수신 확인·서비스 게시 완료로 기록하지 않는다.

[원고 목차](README.md) · [현재 구현](../public/implementation-status.md) · [남은 과제](../public/roadmap.md) · [현행화 대조 근거](../evidence/sass-guide-refresh-20261111.json)
