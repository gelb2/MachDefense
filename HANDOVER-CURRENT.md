# 2026-11-07 레이저 해금 구현·화풍 대조

`feat/level10-effects-art`에서 발칸 보유 + 30미션 클리어 레이저 지급, 기존 저장의 프로필 선택 시 소급 지급을 구현했다. [정책·검사](docs/evidence/laser-turret-unlock-20261107.md). 기기 앱 설치와 운영 배포는 남았다. [신규 포탑 색·도트 대조](docs/research/turret-art-review-20261107/README.md)에서 어두운 색조·두꺼운 포신·넓은 음영을 확인했다. 그림은 그대로이며 수정 시안이 다음 과제다.

아래는 이전 기록이다.

# 2026-11-07 승인 부품 연결·화풍 스킬

작업 브랜치 `feat/level10-effects-art`. 승인 부품 44개 연결과 실기기 검사, iPhone/iPad 일반·치트 앱 4개 설치를 마쳤다. 저장 4개는 설치 전후 동일하다. [검사](docs/evidence/level10-art-integration-20261107.md), [화풍 스킬](.agents/skills/mach-series-art/SKILL.md). develop `1f4dca7`에 병합했다. main·태그·운영 PWA는 변경하지 않았다.

사용자 후속 검수: 포탑 그래픽 정상 표시 확인. 라이플 포탑의 색조·굵기·색채·도트 처리에는 원작과 다른 임의 해석이 보인다는 지적이 있어 원본 재대조 과제로 남겼다. 레이저 포탑은 발칸 해금 이후 현 정책에 맞는 미션에 지급하고, 이미 44미션까지 진행한 저장에도 소급 지급하도록 요청했다. 무장·사운드 등 전체 연결 검사도 필요하다. [TODO](docs/usability-todo.md). 아직 해금 규칙이나 라이플 그림은 추가 수정하지 않았다.

사용자 추가 지적: 라이플·발칸 등 신규 포탑 전반이 원작의 도트·화풍과 달리 임의로 어두운 색으로 만들어져 보인다. 재검토 범위를 신규 포탑 전체의 몸체·무장으로 확대했다. 팔레트·밝기·명암·도트와 실제 게임 크기 비교를 TODO에 기록했다. 이번 요청에서는 그림이나 게임 코드를 변경하지 않았다.

아래는 이전 시점의 기록이다.

# 2026-11-07 추가 레벨 효과·외형

2026-11-07 후속: 원본 세 판본 포탑 Lv.1~5는 몸체·무장 참조가 모두 바뀐다. 몸체 3→4는 변화가 작다. 이전 조립 시안의 min-fit 축소 오류를 발견해 길이 고정 계산으로 바꾸고 몸체·무장 35개를 다시 그렸다. 일부 폭 차이와 도트·피벗 수정은 남았다. 게임 코드는 변경하지 않았다. [근거](docs/evidence/turret-original-stage-change-20261107.md).


후속 시안: 공용 포탑 몸체 Lv.6–10 5개를 추가해 전체 44개 부품 시안을 마련했다. 원작 크기·연결점으로 6종×6단계를 비교하는 [몸체·무장 조립 보고서](https://heavymach-dev-report-20261025.hiphopxy.chatgpt.site/machdefense.html#turret-body-concepts-20261107)를 게시했다. 게임 코드는 변경하지 않았다. [시안·프롬프트](Art/Level10/20261107/README.md).

2026-11-07 후속: 원작 포탑 몸체 5단계·무장 30개와 원작/추가 조립 60개를 대조했다. 39개 시안에서 공용 몸체 Lv.6–10 5개가 빠졌다. 강화에 따른 크기 확대 금지와 원작 부품 비례·연결점 기준을 스킬·TODO에 추가했다. 길찾기 사진은 바탕화면에서 확보했다. [대조](docs/evidence/turret-body-scale-20261107.md).

작업 사본은 `.local/worktrees/save-transfer`, 브랜치는 `feat/level10-effects-art`다. develop `55d9d2b`에서 분기했다. [현재 상태](docs/current-status.md), [효과 수정 검사](docs/evidence/level10-effects-20261107.md), [39개 외형 시안](Art/Level10/20261107/README.md)을 먼저 읽는다. 원작 무기 211행과 기존 게임 소스·클라이언트·저장을 유지했다. 효과 수치 수정은 작업 브랜치에 있으며 게임 운영 배포·기기 설치는 하지 않았다. 락온 처치는 아직 TODO다.

# 2026-11-07 사거리·공격력 후속

포탑 6종과 기지 기관총의 추가 단계 사거리를 수정했다. [전수 점검과 수정 결과](docs/evidence/level10-range-damage-20261107.md), [10레벨·HUD 검사](docs/evidence/level10-hud-20261107.md)를 먼저 읽는다. CTest 40개, Chrome·WebKit 치트 검사, 최신 iPad 검사 2개가 통과했다. v0.4 정식·치트 앱 4개 설치, develop→main 병합, 태그·릴리스와 PWA 배포를 마쳤다. [운영 검사](docs/evidence/v04-release-20261107.md)도 읽는다. 기존 캐시의 첫 탭 재열기에서는 구버전 런타임이 남았고 브라우저 완전 재실행 후 최신 데이터·저장 보존·오프라인 실행을 확인했다. 업데이트 안내 개선은 TODO다. 이온 주변 피해 반경·플라즈마 지속 피해 조사는 TODO다. 아래 미구현 기록은 당시 상태다.

추가 사용자 요청으로 [10레벨 복원 코드](docs/evidence/level10-recovery-20261107.md)를 전수 점검했다. 게임·복원 코드는 그대로이며 검사와 안내만 보강했다.

# 현재 작업 — 타이틀·HUD·원작 대조

2026-11-07: 모든 영구 강화 상한 Lv.10, 신규 단계는 원작 최종 레벨의 외형·발사체·효과·음원 재사용으로 계획을 확정했다. [13개 항목·59단계 수치 제안과 구조 점검](docs/research/level10-plan-20261107.md)을 작성했다. 수치 검산과 기존 C++ 발사 처리 240조건을 실행했으며 게임 적용·실기기 성능 검사는 아직 하지 않았다.

2026-11-06 사용자 검수: 1280×800 가로창 후보는 메크·탱크 왜곡으로 기각됐다. 해당 Wide.png를 채택하거나 다음 앱·운영 배포에 포함하지 않는다. 원본 기체의 형태·부품·비율을 보존하는 외곽으로 다시 작업한다. 다른 비율 시안도 승인된 것으로 취급하지 않는다. 생산 HUD는 INSTALL·UNINSTALL을 보존하고 설치된 전체 목록만 스크롤한다. 상세 조건은 [HUD 설계](docs/research/production-hud-scroll-20261106.md)를 따른다.

작업 사본은 `.local/worktrees/save-transfer`, 브랜치는 `feat/title-hud-original-audit`다. 원격 develop `22024dd`에서 분기했다. [TODO](docs/usability-todo.md) 최상단과 [현재 상태](docs/current-status.md)를 먼저 읽는다. 원본 비교와 생산 HUD 설계·강화/생산량 조사 문서는 과제별로 커밋했다.

타이틀 후보는 C++ 4개·Chrome/WebKit 10개 화면 조건을 통과했다. 이미지 연결부 검수와 기기 설치는 남았다. main·develop 병합과 게임 운영 배포를 하지 않았다. 사용자 개발자의 변 최종 원문은 main `40d3fd1` 그대로 보존한다.

새 과제 `MD-IPAD-SCALE-01`은 iPad·데스크톱이 폰 화면의 단순 확대처럼 보인다는 사용자 관측이다. 현재 과제 이후 원작 HD 판본·기기별 자원 선택·투영·화면 범위를 조사한다. 지금은 TODO 등록 상태다.

아래는 이전 작업 기록이다.

# v0.3 릴리스 — 2026-11-06

공용 복원 연결과 Quick Play·아이폰 공유 코드를 기능 브랜치→develop→main 순서로 반영한다. Galaxy 정식 저장 보존, 치트 9조건, PWA 오프라인 전투·재실행을 확인했다. 사용자는 iPhone↔iPad PWA 왕복과 미션 클리어를 확인했다. [현재 상태](docs/current-status.md), [v0.3 변경 이력](CHANGELOG.md), [검사 근거](docs/evidence/save-transfer-all-platforms-20261106.md)를 먼저 읽는다. 아래는 이전 단계 기록이다.

# 웹·PWA 공개 — 2026-11-06

2026-11-06 `feat/save-transfer-all-platforms`: 슬롯 화면의 공용 코드 복원 UI와 Apple·Android·Web 연결을 구현했다. 사용자 iPhone 원본은 읽기 전용 백업으로 보존하고 검사에 복사본을 쓴다. PWA 빌드 `d679d7b1061097b5`를 gh-pages `86cef6e`로 배포하고 운영 주소에서 재검사했다. [최신 검사 결과와 남은 실기기 범위](docs/evidence/save-transfer-all-platforms-20261106.md)를 먼저 읽는다. 아래는 이전 배포 기록이다.


사용자 지시로 GitHub를 공개하고 [MachDefense 웹·PWA](https://gelb2.github.io/MachDefense/)를 배포했다. 작업은 `release/pwa-20261106`에서 정리해 develop→main으로 병합한다. 버전은 v0.2다. PWA 설치 식별 주소를 수정했고 운영 설치·오프라인 재실행·저장·복원 검사를 통과했다. [현재 상태](docs/current-status.md), [검사·Tauri 착수 조건](docs/evidence/pwa-release-20261106.md), [남은 과제](docs/usability-todo.md)를 따른다. 아래는 이전 단계 기록이다.

# 2026-11-06 GitHub 문서 갱신

작업 브랜치 `docs/public-manuscripts-20261106`는 develop `ae08521`에서 분기했다. README와 개발자의 변, 기능·기술·설치·검사·트러블슈팅·크레딧을 사용자 정정에 맞춰 작성했다. 게임 소스·자원·빌드 설정은 수정하지 않았다. 웹·PWA 구현은 별도 `feat/web-pwa`이며 이 문서 병합에 포함하지 않는다. [방문자 문서](docs/public/README.md), [Sass 집필 자료](docs/editorial/sass-publication-guide.md), [전체 원고](docs/editorial/github-manuscripts-20261106.md).

# 현행 작업 — Web·PWA

## 2026-11-06 Web·PWA 구현과 검사

`feat/web-pwa`에서 일반·치트 웹판, 5슬롯 공용 복원 코드, IndexedDB 저장과 오프라인·업데이트를 구현했다. 네이티브/Wasm 각각 37개 검사, 160조건의 게임 상태 대조, Chrome·WebKit 복원·재실행·음향·GPU·저장 실패 검사가 통과했다. 아이폰 Stage 36 저장 복사본을 웹에서 복원했고 아이폰 원본 바이트는 유지했다. Apple·Android 실행기와 기존 계산·음향·자원 파일은 변경하지 않았다. 공용 복원 연결 4파일은 추가 변경했다. [파일별 변경·검사·남은 범위](docs/evidence/web-pwa-20261106.md).

실제 모바일 PWA 설치, 120Hz·장시간 성능과 Tauri Mac은 남았다. develop/main 병합과 게임 공개 배포는 하지 않았다.

작업 사본: `.local/worktrees/iphone-hud-top`, 브랜치 `feat/web-pwa`. main v0.1과 다른 worktree의 기존 변경은 유지한다. 아이폰 저장은 `.local/pwa-20261106/iphone/`에만 보존한다. 아래는 이전 작업 기록이다.

# 2026-11-06 iPhone HUD 상단 후속

현재 수정은 `fix/iphone-hud-top`이며 develop `0835b5a`에서 분기했다. 원본 HUD의 상단 조각으로 iPhone의 노출 영역만 덮는다. iPad 화면을 바꾸지 않는 조건이다. 시뮬레이터 화면·터치 검사와 iPad HUD 픽셀 대조를 마쳤고 iPhone Release 정식 앱을 설치했다. 저장은 설치 전후 바이트가 같다. Device Hub의 게임 화면 입력이 진입으로 이어지지 않아 실기기 직접 생산·메뉴 검사는 남았다. [검사 기록](docs/evidence/iphone-hud-top-20261106.md), [남은 월드 표시 과제](docs/usability-todo.md#iphone-우측-hud-상단--2026-11-06). 사용자가 iPhone 설치본의 덮임을 확인하고 develop 병합을 지시했다. 현재 남은 작업은 TODO 최상단 요약을 따른다.

main의 v0.1은 `08b5380`이다. 별도 `chore/post-release-hygiene`의 타이틀·문서 변경과 원래 checkout의 미커밋 작업은 유지했다. 아래는 이전 단계 기록이다.

# MachDefense 인계 — 2026-11-05

# MachDefense 인계 — 2026-11-06

Android 구현 커밋은 `639ca43`이며 develop에 병합했다. 현재 릴리스 문서는 `docs/first-release`에서 작성했다. develop `5bc233e`에서 별도 worktree로 분기했다. Android 일반·치트 앱을 빌드하고 Galaxy USB와 에뮬레이터에서 검사했다. 아이폰 저장을 Galaxy 일반 앱에 옮겨 Stage 33 진입·장비·강화·자금·재실행 보존을 확인했다. [구현·검사·경로·남은 일](docs/evidence/android-native-20261106.md), [빌드](Android/README.md)를 따른다. Apple·공통 코드·자원 2,978개 파일은 SHA가 같다. 기존 fix/hud-title-presentation checkout의 미커밋 작업은 보존했다. 이번 v0.1은 develop→main 순서로 병합한다. 원격 게시 결과는 Git 태그와 GitHub Release에서 확인한다.

현재 작업: `feat/emergency-resupply` (`95a865a`에서 분기). 승인된 반복 지원을 공통 Core와 원작 에셋 UI로 구현했다. 최신 결과·설치·남은 범위는 [긴급지원 기록](docs/evidence/emergency-resupply-20261105.md). 이후 아래 모듈화 인계는 기반 작업 기록이다. iPhone·iPad 정식·치트 설치, 양쪽 치트 자동 검사와 iPad 직접 수령을 마쳤다. iPhone 정식 앱은 사용 중인 최신 진행도를 보존했다. 지원 창 프레임 p99 16.67ms·25ms 초과0회. 보고서 게시 성공. 남은 직접 검사와 난이도 비교는 실행 기록을 따른다. develop/main에는 병합하지 않았다.

최신 후속: 사용자가 복귀 후 효과음 소실을 발견했다. OpenAL Soft 장치 pause/resume 누락을 iPad에서 재현·수정했고 샘플 진행을 포함한 오디오·전투 검사 6개가 통과했다. [효과음 복귀 수정](docs/evidence/effects-resume-20261105.md).

현재 브랜치는 `refactor/shared-core-audio`다. 기존 재개발·계획을 `0243083`에 보존하고 로컬 develop의 `8109ae6`에서 분기했다. 원본 오디오·텍스처 대조 수정은 `f623eba`에 커밋했다. 이후 공용 Core·Audio 분리와 검사 결과를 `193ed2f`에 커밋했다. 최신 결과는 [모듈화 실행 기록](docs/evidence/module-work-20261105.md), 남은 과제는 [TODO](docs/usability-todo.md)를 따른다. 원격 push·main 병합·제품 배포는 하지 않았다. [통합 보고서](docs/report-site.md#최근-갱신--2026-11-05-모듈-구현검사)는 게시했다.

2026-11-05 06:08 iPhone 정식 앱도 최신 모듈·오디오 수정본으로 빌드·설치했다. 기존 Stage 17·Lv.65 저장 불러오기, 전투·생산·메뉴 재개를 확인했다. 검사 후 저장을 원래 바이트로 복원하고 앱을 종료했다. [기기 검사 기록](docs/evidence/module-ipad-device-20261105.md).

Core는 Contracts·Data·Simulation·Session·Presentation·Validation로 나뉘고 효과음은 Audio 모듈을 사용한다. 게임 갱신·난수 순서를 보존했다. 원본 IPA와 실행 바이너리는 저장소 밖에서 분석에만 사용한다. 새 checkout에 들어가는 소스와 Resources만으로 일반 앱 Release 빌드를 확인했다.

C++ 34개와 분리 전후 160조건·144,000틱 비교, iPhone 시뮬레이터 26개 검사는 통과했다. iPad 25개와 재실행 1개, 10분 전투·20회 월드 시작, 일반/치트 직접 조작·저장 재실행도 확인했다. 실제 iPad 최신 일반·치트 설치, 자동검사 27개, 10분 전투, 직접 버튼·저장·복귀 검사도 마쳤다. [실기기 결과](docs/evidence/module-ipad-device-20261105.md). 원작 출력음 녹음 대조는 남아 있다. 오디오 세션 경고 2건은 후속 수정으로 iPad 검사 6개·실행 경고 0개를 확인했다. [수정 결과](docs/evidence/audio-session-20261105.md). 테스트 성공을 기존 길찾기·화면 밖 표적·HUD 그림 과제의 해결로 해석하지 않는다. 해당 문제는 TODO에 남아 있다.

HeavyMach 1 저장소는 읽기만 했다. 사용자의 iPhone을 조작하지 않는다. 일반/치트 저장과 원본·실패 로그를 보존한다. 원본·디컴파일 출력은 Git·보고 사이트에 넣지 않는다. 현행 정책은 AGENTS.md·docs/canonical.md와 문서 연결표를 따른다.

아래는 이전 단계의 기록이다. 현재 브랜치·결과는 위 문서와 최신 실행 기록을 우선한다.

## 이전 단계 — 호환판 구현·검사 인계

- 브랜치: `feat/ios-compatibility`. develop·main 병합과 원격 push는 수행하지 않는다.
- 원본: Mach Defense 1.0.5의 ARMv7 실행 파일을 touchHLE 인터프리터로 구동한다. 이 기록 당시에는 소스 재구현을 시작하기 전이었다.
- 완료: 시뮬레이터 슬롯 생성·Stage 1 진입·유닛 생산/이동, iPhone Air 무선 설치·타이틀/슬롯 화면 확인. 슬롯 생성 때 종료되던 빈 이미지 처리와 Xcode 서명 문제는 재발 후 실행 파일 생성 의존 관계를 다시 고쳤고 두 번의 증분 빌드를 확인했다.
- 사용자의 후속 지시와 잠금 해제 뒤 아이폰 수정본을 직접 검사했다. 기지·네 유닛 강화 상세의 증감값 표시·닫기와 생산·교전·저장·재실행을 확인했다. 최신 근거는 docs/evidence/ios-compatibility-upgrade-error-20261104.md다. 실제 강화 구매는 남았다.
- 남은 문제: 이름 입력칸 포커스 후 위치 이동. 구버전 API/키보드 알림 문제라는 사용자 추정과 합성 경로 전환을 함께 조사한다. 아직 원인을 확정하지 않았다.
- 남은 검사: 교전·미션 완료·저장·재실행, iPad 실기기, 음향·성능. 기존 증거를 다시 완료로 확대하지 않는다.
- 현재 빌드/런타임은 `/Users/sokol/Documents/Codex/Builds/MachDefense/compat-ios-20261104`에 있다. 기기용 GUI 빌드는 Xcode DerivedData에 있다. 경로·원본은 `.local/compatibility.json`, 검사 로그는 `.local`에 보관한다.
- 런타임 수정 뒤 `Compatibility/patches/machdefense.patch`도 갱신하고 역적용 검사로 사본과 일치하는지 확인한다. 원본과 HeavyMach 1 사본은 수정하지 않는다.

## 치트 검증 후 재개발 지시

사용자가 아이폰 일반 포팅판을 플레이한다. 그 앱은 종료·재설치하거나 저장을 변경하지 않는다. [치트 검증 계획](docs/compatibility-cheat-validation-plan.md)에 따라 시뮬레이터용 별도 포팅판 치트앱부터 준비한다. 포팅판 전체 기능을 검사한 뒤 소스 재개발판과 대조한다.

## 사용자 플레이 관측 인계

[관측 기록](docs/port-observations-20261104.md)에 적/기지 파괴·미션 클리어·패배 후 미션 선택 복귀의 사용자 확인, 가격 누락·한글 이름·유닛 자글거림의 비교 요청을 모았다. 재개발 전과 UI/그래픽 변경 때 읽는다. 현재 아이폰 일반 포팅판은 사용자가 플레이 중이므로 조작하지 않는다.

신규 버전은 공용 에셋을 쓰는 유니버설 구성을 목표로 한다. [고해상도 자원 사전조사](docs/universal-assets-plan.md)를 재개발 전 수행한다. HD판이 존재한다는 추측과 현재 확보한 판본을 구분한다.

2026-11-04 언어 후속 지시: 사용자는 캐나다·미국 계정 구매판에서 유닛 이름까지 모두 영문이었다고 확인했다. 현재 기준은 indieapps 1.0.5이며 로컬 아이폰 빌드의 포함 IPA 해시가 일치한다. 시뮬레이터 로그에서는 원본에 `ko, en`/`KR` 전달을 확인했다. 실제 언어 선택 원인은 [언어 조사](docs/port-observations-20261104.md)에 따라 판본·기기 언어·지역·슬롯 국가를 나누어 확인한다.

## 당시 작업 우선순위 — 판본 조사 착수 시점

신규 앱은 영어만 지원한다. 다음 작업은 [전체 판본 조사와 재개발 계획](docs/version-audit-plan.md)이며, 별도 HD판 존재와 추가 빌드부터 확인한다. 현재 3개가 모든 출시판이라고 간주하지 않는다. 기준 코드·영문/고해상도 자원을 비교하고 계획을 정한 뒤 소스 재개발로 진행한다.

## 2026-11-04 판본 조사 후속

현재 기준은 위메이드 1.0.3. 기존 indie105 → 별도 위메이드 포팅 검사 → 소스 재개발 순서다. 사용자의 실기기 ordinary 앱을 변경하지 않았다. iPhone Air iOS 26.5의 별도 audit 앱으로 첫 전투·생산·교전·저장·강화 6종 구매와 재실행 보존을 검사했다. 결과·남은 항목은 docs/evidence/version-audit-20261104.md와 docs/current-status.md.

외부 runtime에 frameInterval 0 보정과 WeMade GuiBanner 광고 차단을 추가하고 machdefense.patch에 반영했다. 역적용 check 통과. 일반 실기기 binary에는 아직 이번 변경을 반영하지 않았다. 별도 앱 패키지는 validation/package-version.py로 생성한다. 위메이드와 indie의 저장 크기·슬롯 간격이 다르므로 교환 금지. `.local/version-audit`의 원본 시험 저장과 fixture는 보존했다.

통합 보고 사이트의 `/machdefense.html` 게시 성공. 소스 커밋 `eb29ffc3244569c9156173f01a4ab44f50b38720`, 배포 `appgdep_6aea4fdca940819188366867d076cd52`, 버전 `appgprj_6addbf55aea081918d3d182d7e496e88~appgver_e86ac7470d1481918356b98fd00d0c40`. 기존 목차에 연결했다. 새 캡처 3개는 무손실 WebP로 변환하고 픽셀 일치를 검사했다. 로컬 페이지의 표·링크·이미지를 확인했다. 배포 뒤 Chrome은 `ERR_INTERNET_DISCONNECTED`를 표시해 운영 화면의 브라우저 확인은 남았다. Sites 배포 상태는 `succeeded`다. 시뮬레이터 시험 앱은 종료하고 홈 화면 복귀를 확인했다.


## 최신 작업 — 소스 재개발

`afa58fc`에서 `feat/source-reimplementation`을 만들고 기존 변경을 보존했다. `docs/source-reimplementation-plan.md`와 `docs/current-status.md`의 C++ 소스 재개발 절을 먼저 읽는다. 앞선 “미착수” 기록은 당시 상태다. 새 코어·Resources·Apple workspace와 `createGame` 연결을 구현했다. 전투·생산·강화·저장 통합 검사를 진행했으며, 전체 미션·연출·화면 배치의 남은 범위는 아래 최신 소스 앱 연결을 따른다. 포팅판 실행을 소스판 실행으로 보고하지 않는다. `.local/source-build`의 CTest와 `.local/apple-*-tests.*`의 시뮬레이터 검사 증거를 재사용한다. 원본 분석은 저장소 밖 source-20261104/analysis에 있다. IPA·원본 실행 파일·전체 디컴파일 출력을 Git에 넣지 않는다.

## 최신 소스 앱 연결

`GameSession.cpp`의 createGame을 구현했다. Apple 일반·치트 시뮬레이터 앱 빌드, iPhone 직접 프로필 생성과 생산·이동을 확인했다. 기존 사용자 기기 포팅 앱은 보존했다. 현재 전체 재개발 완료가 아니며, 최신 남은 범위는 current-status와 source-session-validation을 따른다. 과거 createGame 미구현 문장은 당시 단계의 기록이다.

## 소스판 보고서 게시와 다음 진입

기존 `/machdefense.html` 갱신 배포가 성공했다. 사이트 소스 `c3e6f79aebe450e5185bfd74734ac840f949eaf7`, 배포 `appgdep_6aea657c47d881919a1d014749bee850`, 버전 `appgprj_6addbf55aea081918d3d182d7e496e88~appgver_79f3fe38d60c81918987f79eccd7a26d`. 현재 확인범위·22개 검사·오류 수정·최종 iPhone 캡처·남은 작업을 같은 페이지에 추가했다. 사이트 배포만 수행했고 MachDefense 저장소의 커밋·푸시·develop/main 병합은 수행하지 않았다.

새 일반/치트 앱 모두 두 시뮬레이터에 설치했다. 직접 검사의 상세와 캡처 위치는 `.local/source-simulator-validation-20261104.md`에 있다. 사용자의 실기기 포팅 앱·저장은 보존했고 시험 시뮬레이터를 종료했다. 마지막 iPad HUD 레이어 수정본의 직접 검사는 Mac 잠금으로 남았다. iPhone 최종 화면과 공통 HUD 회귀검사는 통과했다.

이 보고서 게시 당시에는 치트 검증 화면을 다음 작업으로 정했다. 이후 구현 결과는 아래 치트 앱·80개 캠페인 절에 있다. 당시 계획은 다음과 같았다: 원작 기능·진형·전투 연출을 완성하면서80개 미션의 진행·보상·저장을 검사한다. 그다음 전체 회귀·장시간 성능을 검증하고, 이후 iPhone·iPad 화면을 확장한 뒤 재검사한다. 최신 순서는 docs/source-reimplementation-plan.md를 따른다. 새 소스판 전체 완료로 보고하지 않는다.

보고 사이트는 256MiB에 가까워졌다. 이번 archive는 macOS xattr·AppleDouble을 제외한 ustar로268406272바이트다. `.local/site-tar-bin/tar`를 PATH 앞에 두고 Sites source helper로 포장했다. 게임 자료와 캡처의 픽셀을 삭제·변경하지 않았다. 다음 이미지 추가 전에는 사이트 용량을 재정리해야 한다. 원본 IPA/바이너리는 계속 반출 금지다.

## 이전 순서와 작업 보존 — 2026-11-04

사용자가 치트앱 계획 기록과 지금까지 작업의 커밋을 지시했다. `feat/source-reimplementation`에서 커밋하며 develop/main 병합·원격 push는 포함하지 않는다. 치트앱 준비 → 원작 기능 및80개 미션 진행·저장 검사 → 전체 검증 → iPhone/iPad 화면 대응 → 재검사 순서로 정본·계획·검증·TODO를 맞췄다. 통합 보고 원고에도 같은 순서를 반영했다. 이 순서 수정분은 로컬 원고이며 사이트에는 아직 재게시하지 않았다.

## 치트 앱·80개 캠페인 구현 후속 — 2026-11-04

`feat/source-reimplementation`에서 TEST5쪽과 ValidationTools를 구현했다. 최신 결과는 docs/evidence/source-cheat-campaign-20261104.md. 최종 CTest31종, Apple XCTest10건, 최신 일반·치트 빌드 성공. 80개 진행·저장과 치트 조건80개 교전을 나누어 검사했다. 총구·탄피·궤적·잔해를 연결했고 원본 진형 아이콘 no-op을 확인했다. 결과 저장 실패 후 Next/Retry/Select Stage 재시도와 TEST 오류창 입력도 수정했다.

사용자 기기 포팅판은 변경하지 않았다. 시뮬레이터 직접 검사를 마치고 앱 종료·홈 복귀·시뮬레이터 종료까지 확인했다. 같은 조건 포팅판 대조·일반 난이도 진행·장시간 메모리/프레임·독립 빌드가 남는다. 이 단계 후 화면 대응을 한다. 최종 로그 `.local/final-core-tests.log`, `.local/source-validation-ui-20261104.md`를 재사용한다.

기능 변경 커밋은31395fb다. 추적 파일만 git archive로 `.local/clean-source-31395fb`에 풀고 새 DerivedData에서 일반 iOS Simulator 앱 빌드 성공. `.local/clean-source-apple-build.log`를 보존했다. 같은 조건 포팅판 대조와 장시간 검사가 남으며 화면 확장은 아직 시작하지 않았다.

## iPad 오디오·UI 수정 작업 보존 — 2026-11-04

`feat/source-reimplementation`에서 오디오 컨텍스트 유지, 보상·오류창의 반복 파일 읽기 제거, 생산·보상 미리보기 회전, 강화 비용 정렬, iPad 두 세로 방향 지원을 수정했다. 치트 앱 전용 프레임 측정은 메모리에 수집하고 비활성 전환 때 CSV로 내보낸다. 일반 앱에서는 켜지지 않는다.

오디오 XCTest 2개와 관련 코어 회귀 7개가 통과했다. iPad 치트 앱의 수정 후 248.82초·14,912프레임 측정과 사용자 개선 확인은 `docs/troubleshooting.md`에 있다. `docs/usability-todo.md` 맨 위의 현재 작업 순서를 따른다. 전체 무기·오디오 중단 복귀·메모리 판정·일반 앱·전체 UI 및 실제 회전 재검사는 남았다. 화면 확장은 그 뒤다.

Xcode GUI가 바꾼 개인 서명 팀과 공유 스킴 형식 3개 파일은 로컬에서 보존하고 이번 기능 커밋에서 제외한다. `Apple/iOS/project.yml`과 두 Info.plist의 방향 수정은 커밋 대상이다. develop/main 병합은 하지 않는다. 원본 IPA·실행 바이너리·로컬 측정 원본은 Git에 넣지 않는다.

## 최신 사용자 작업 순서

포팅판 대조 → 인게임 일러스트 앱 아이콘 제작 → 아이콘 방향·가격·회전 재검사 → 치트앱 전체 기능 연속 검사 → 성능·음향·메모리·생명주기 검사 → iPhone·iPad 화면 확장 계획 수립 순서로 변경했다. 직전 답변 번호는 4 → 6 → 2 → 1 → 3, 그다음 5의 계획 수립이다. 6은 앱 아이콘 제작이며 웹·PWA·Tauri 착수를 앞당기는 지시가 아니다. 다음 작업은 위메이드 1.0.3 포팅판 대조부터 시작한다.

## 포팅판 동일 조건 대조 진행

기지 강화 6종을 두 앱에서 실제 구매하고 저장을 비교했다. 차감 7,800Cr·저장 단계·누적 점수가 일치했다. 대조에서 발견한 누적 점수 표시와 레이더 누락을 수정했고 관련 검사 4개 및 시뮬레이터 화면을 확인했다. [근거와 남은 대조](docs/evidence/port-source-parity-20261104.md). 아직 전체 대조 완료는 아니다.

유닛 4종 Lv.1→Lv.2 구매를 양쪽 앱에서 완료했다. Save & Exit 후 저장 비교에서 잔액 987,250Cr·점수 274·레벨·장비 순서·장착 상태가 일치했다. 상세는 위 근거 문서를 따른다. 소스판 CLOSE 후 강화창이 남는 오류를 추가 수정했고 관련 3종 검사가 통과했다. 마지막 수정본을 iPhone Air 시뮬레이터에 설치하고 유닛·특수기술 창의 CLOSE를 각각 직접 눌러 그림 제거와 전투 재개를 확인했다. 실기기 사용자 저장은 변경하지 않았다.

초기 특수기술 3종도 양쪽 앱에서 Lv.2로 강화하고 Save & Exit 후 비교했다. 비용 합계 1,820Cr, 잔액 985,430Cr, 점수 274, 저장 레벨·수량·장착 상태·순서가 일치했다. `.local/parity-20261104/special-comparison.json`에 결과를 보존했다. CLOSE 수정과 이전 대조 기록의 커밋은 a3ee24f다.

## 2026-11-04 첫 미션 교전 대조 후속

초기 특수기술 강화 대조, 두 앱의 실제 첫 미션 승리·500Cr 보상·Stage 2 진입·저장을 추가 확인했다. 소스판 재실행 슬롯은 985,966Cr·점수 1,324·Stage 2다. 입력 시점·난수를 고정하지 않아 교전 결과의 완전 일치를 주장하지 않는다. 자세한 수치와 자료는 docs/evidence/port-source-parity-20261104.md에 있다. 시험 저장의 잘못된 최대 단계 입력과 소스판 예외를 보존했다. 정상 범위 입력으로 양쪽 재검사를 마쳤다. 원본의 경계값 보정 구현은 TODO MD-PARITY-BOUNDARY-01에 남았다.

사용자가 검사 지연을 지적해 HeavyMach 1 방식의 Apple XCTest를 연결했다. 아래 최신 결과를 따른다. 앱 아이콘 등 나머지 작업 순서는 유지한다.

개발자의 변 작성, 인용 후보 출처 확인, 사용자 글감의 중심 주제를 TODO MD-WRITING-01~03으로 등록했다. 완성 원고는 아직 작성하지 않았다. 이번 변경은 로컬 기능 브랜치에만 기록하며 main/develop 병합·배포를 하지 않는다.

## Apple 자동검사 연결 — 2026-11-04

HeavyMach 1의 XCTest 방식을 적용했다. 실제 렌더러·오디오·시험 저장을 연결한 3종 검사가 빌드 포함 33.083초에 통과했다. 80개 미션 승패·재도전·보상·진행·저장 재읽기, 포팅판 관측값에 따른 13개 강화 구매, 생산 220건·운반봇 배치 60건·특수공격 54건을 검사했다. [실행 결과와 재실행 방법](docs/evidence/apple-automated-sweep-20261104.md).

전체 강화 단계 구매와 모든 일반 무장 검사, 입력·난수를 고정한 포팅판 교전 대조 및 실기기 성능 측정은 남았다. 반복 항목은 이 자동검사에 추가한다.

## 2026-11-04 정식 앱 피드백 수정

전체 강화·무장·iPad 짧은 성능 검사는 완료했다. [측정 결과](docs/evidence/full-upgrade-weapon-device-20261104.md). 이후 사용자가 정식 앱에서 포탑 안내·딤·생산 완료음·복귀 먹통을 발견했고 로딩 일러스트도 확인하도록 지시했다. [수정·검사·설치 상태](docs/evidence/source-ui-audio-lifecycle-20261104.md)를 먼저 확인한다. 현재 작업 파일에는 이 수정과 아이콘·실기기 자동검사 변경이 함께 있다. 사용자 Xcode 서명 설정 변경을 보존한다.


사용자 최신 지시: 현재 정식 빌드를 직접 테스트 중이므로 계속 전달하는 관측을 TODO에 누적한다. iPhone 앱을 임의로 종료·재설치하지 않는다. 다섯 오류 수정본은 iPhone/iPad 정식과 iPad 치트에 설치했다. 그 뒤 안내창 글자 축소 보정은 iPad 치트에만 추가 반영했다. 정식 앱 후속 반영은 사용자 테스트 후 진행한다. iPhone 치트는 3개 앱 한도이며 기존 포팅판 삭제 승인을 기다린다.

## 후속 수정·핵폭탄 검사 — 2026-11-04 23:19

아이패드 치트 앱에 완료 표시 오류·로딩 효과음·버튼 효과음·유닛 겹침 해소 수정본을 설치했다. [수정 및 실기기 검사](docs/evidence/source-ui-audio-lifecycle-20261104.md). 핵폭탄은 원본 피해식과 수치가 일치하며 5레벨 체력 검사가 실제 iPad에서도 통과했다. [핵폭탄 검사](docs/evidence/nuclear-damage-20261104.md).

미션 3 구멍 위 포탑 설치와 미션 4 정체의 포팅판 실행 대조, 피격·플라스마·공간 음향 대조는 남았다. 화면 확장은 계획만 기록했다. iPhone 정식 앱은 사용자 테스트 중이므로 종료·재설치하지 않는다.

## 2026-11-04 23:40 후속 검사

기지 강화 실제 효과·핵폭탄 일반 체력 적 피해/처치·오디오 지연 중단 알림 및 미디어 재설정 복구를 iPad에서 검사했다. 3개 검사 4.275초 통과. [수정과 결과](docs/evidence/base-audio-followup-20261104.md). iPhone 정식 앱은 사용자 플레이 중이라 교체하지 않았다. 지형 포팅판 직접 대조·실제 출력음 녹음·피격/지속 피해음 대조는 남았다.

### 23:58 추가 결과와 남은 실행

- HitBody 3종·지속 피해음 연결 완료. DamageAudioSweep 72조건 iPad 통과. 원본 raw OpenAL source15의 WeMade 포팅판 매핑은 Missile06.wav. 옛 iOS 드라이버 매핑과 공간 위치음향은 미확인.
- 최신 CTest33/33,184.48초 통과. iPad1800프레임/410효과음/쓰기0; 교전3구간 p95 16.67ms,25ms 초과0. `.local/final-audio-performance-ipad.xcresult`.
- 최신 iPad 검사 호스트 앱 일반 실행도 devicectl 성공, PID7675 종료 완료. 직접 화면 확인은 CUA cgWindowNotFound로 막혔다. 사용자에게 Mac 잠금 여부를 확인 요청했다.
- 테스트 플러그인 없는 별도 치트 앱은 `.local/clean-cheat-feedback`에서 컴파일했으나 CLI 서명 errSecInternalComponent로 실패. Xcode GUI 창 접근 복구 뒤 기존 절차로 Clean Build Folder→MachDefenseCheat build→설치→터치·홈복귀 확인 필요. 기기에는 최신 XCTest 호스트가 남아 있다. 이전 깨끗한 앱으로 되돌리면 수정이 빠지므로 덮어쓰지 않았다.
- 사용자 플레이 중인 iPhone 정식 앱은 건드리지 않았다.
- 포팅판 시뮬레이터의 Stage4 시험 저장을 `.local/av-parity-20261104/port-before-stage4.sav`로 복원하고 바이트 일치 확인. 이후 미션3·4 직접 비교 때 다시 시험용 저장을 준비한다.
- 보고서 소스는 `.local/report-site-edit`, 기존 project appgprj_6addbf55aea081918d3d182d7e496e88. 4개캡처·강화/핵폭탄/오디오/절차점검 추가. 배포기록은 아래 최종상태를 확인한다.

### 통합 보고서 배포 완료

2026-11-04 23:58:46 KST, Sites 상태 succeeded. URL: https://heavymach-dev-report-20261025.hiphopxy.chatgpt.site/machdefense.html#feedback-20261104
소스 SHA `3611c5444ba7168f8c5342a73b1f47682f6bec85`, 배포 `appgdep_6aeb4802cfb081918c1194640d18d4e6`. 소유자 전용 유지. Safari 로컬 화면에서 새 절·표·링크를 확인했다. 배포 후 브라우저 열기는 Mac 창 접근 실패로 미실행. 공식 배포 성공 응답 확인.

사이트 압축 파일의 확장 용량 제한을 넘겨 새 캡처를 무손실 WebP로 바꾸고 기존 PNG/GIF도 픽셀·애니메이션 프레임·시간을 유지한 채 압축했다. 기존 이미지 URL은 유지했다. macOS AppleDouble 메타데이터를 COPYFILE_DISABLE=1로 패키지에서 제외한 최종 tar는255.53MiB다. 게임 자원은 변경하지 않았다.

## 2026-11-05 00:08 설치·화면 연결 후속

Xcode에서 Clean Build Folder 후 MachDefenseCheat 빌드 성공. 테스트 PlugIns가 없는 앱과 코드 서명을 확인하고 실제 iPad에 설치·실행했다. `.local/final-feedback-apps/MachDefenseCheat.app`, `.local/final-feedback-ipad-install.log`, `.local/final-feedback-ipad-launch.json`. 위 23:58의 CLI 서명 실패 후속은 이 GUI 빌드로 해결했다.

Device Hub 전체 창에서 좌표 입력이 noWindowsAvailable로 실패했지만, AX의 Switch to compact window를 누른 뒤 좌표 입력이 복구됐다. 서비스·서버 버전과 서명은 일치했고 인증 거절은 없었다. 서비스나 권한은 변경하지 않았다. 실제 iPad에서 TEST 표시, IPADTEST 슬롯, Stage2 선택·진입, RifleTurret 선택 시 Tap to build 안내, 월드 터치 후 생산 대기열과 셀 차감을 확인했다. 백그라운드 복귀 검사는 진행 중이다.

## iPad 직접 조작 — 2026-11-05 00:11

테스트 플러그인이 없는 최종 치트 앱을 Xcode GUI에서 새로 빌드하고 iPad에 설치했다. 서명 검사 통과. 실제 IPADTEST 프로필의 Stage2에서 포탑 선택→Tap to build 안내→위치 지정→생산→배치→사격을 확인했다. 홈 화면으로 나갔다 돌아오자 System Menu가 열렸고, 셀 412에서 멈춰 있었다. Resume Game을 누르자 메뉴가 닫히고 셀 증가·적 이동·포탑 사격이 재개됐다. 복귀 뒤 특수기술 패널과 Menu도 직접 눌렀다. Save & Exit로 타이틀 복귀를 확인하고 프로세스를 종료했다. iPad 홈 화면도 확인했다.

Device Hub 전체 창의 좌표 입력 오류는 작은 기기 창으로 전환한 뒤 해소됐다. 재현 조건과 복구 방법을 apple-wireless-device 스킬에 기록했다. 이번 화면 검사는 오디오 녹음 검사를 포함하지 않는다. 사진 촬영 후 실제 출력음 비교는 남아 있다. iPhone 정식 앱은 변경하지 않았다.

00:13 Safari에서 배포된 통합 사이트의 MachDefense 링크를 열고 수정 표·피격음72조건·1800프레임 결과·남은 과제 본문을 확인했다. 실제 운영 경로는 `/machdefense#feedback-20261104`로 표시된다. 사이트 보고서에는 23:48 자동검사까지 반영돼 있고, 00:11 직접 조작 결과는 현재 저장소 근거 문서에 추가했다.

## 2026-11-05 화면 확장 후속

사용자 최신 지시로 iPhone→iPad 화면 대응을 먼저 진행했다. 전투만 확장하고 메뉴에 여백이 남은 상태를 사용자가 지적하여 메뉴도 수정했다. 시뮬레이터·실제 iPhone/iPad 자동검사가 통과했다. [근거와 남은 검사](docs/evidence/viewport-20261105.md). iPhone 포팅판은 사용자 승인으로 저장을 저장소 밖에 백업하고 삭제했다. HeavyMach 정식과 MachDefense 정식은 유지한다. 아직 이 작업을 커밋·병합하지 않았다.

## 2026-11-05 01:24 iPhone 정식 설치와 iPad 후속

사용자 요청으로 Xcode GUI MachDefense 일반 앱을 빌드해 iPhone에 설치했다. 저장 42,496바이트 전후 일치, 코드 서명·일반 번들·PlugIns 없음 확인. 정식 앱 Stage6 전투 화면 확인 후 아이폰을 더 조작하지 않는다. 이후 기기 작업은 연결된 iPad만 사용한다.

iPad 회색 띠는 이번에 넣은 GameHUD::append의 x=250부터 오른쪽 끝까지 원본 금속 조각을 반복하는 코드 때문이다. 전장 폭256은 그대로 두고 HUD만 오른쪽으로 이동한 결과다. 사용자 요구를 충족하지 않아 미완료로 기록했다. [근거](docs/evidence/viewport-20261105.md), [TODO](docs/usability-todo.md). 아직 회색 띠 수정 코드는 적용하지 않았다. 마지막 타이틀 좌우 샘플 폭1px 수정은 01:23 일반 앱에 포함됐고, iPad 치트 설치본은 그 직전이다.

Device Hub에서 화면이 보이지만 클릭이 전달되지 않고 회전 시 The connection was invalidated가 나왔다. Device Hub 앱만 종료 후 다시 열어 기기 선택·View Screen·compact 전환하자 실제 iPhone 슬롯 생성·이름 입력·미션 진입·생산·메뉴/재개·Save & Exit 입력이 복구됐다. Apple 권한·키체인·기기 신뢰는 변경하지 않았다.

## 2026-11-05 01:40 — iPad 회색 띠 수정과 다음 커밋

- 사용자 요청: 현재 작업을 끝내고 커밋한 뒤, Downloads의 01:32:18 스크린샷에 나온 기지 업그레이드 창 문제를 계획·수정한다. 투두에 원본 위치와 관측을 기록했다. Finder 미리보기로 확인했다.
- 회색 띠 제거. World의 presentationScale은 표시 전용이며 전화 폭에서는 1이다. 맵/유닛/효과/메시를 같은 배율로 그리며 터치와 카메라는 역변환한다. 원래 전투 좌표를 바꾸지 않는다.
- gray-red.log 간격 검사 실패 → gray-green.log 6/6 통과. gray-simulator.xcresult와 gray-ipad.xcresult 성공. 기기검사 21.61초.
- 사용자 iPhone은 정식 앱 01:24 설치·저장 바이트 동일 확인 이후 조작하지 않았다. 향후 기기 검사는 iPad로 한다.
- 다음 버그 원인 후보: base upgrade는 HUD가 bottomRight 기준으로 올라오는데 항목은 layout.center로 이동해 서로 다른 좌표를 사용한다. 커밋 후 화면과 버튼을 HUD 기준으로 맞추고 확장된 하단 배경 끝을 점검한다.

## 2026-11-05 01:48 — 스크린샷 후속 수정

`9e1121e`에 그전 수정 전체를 커밋했다. 이후 기지 업그레이드 창 항목/입력을 하단 HUD 기준으로 통일하고 배경 중앙·하단 틈을 기존 에셋으로 연결했다. core, 시뮬레이터, 실제 iPad 검사 통과. 새 검사는 5화면에서 첫 강화 선택/구매/크레딧 감소/레벨 증가/닫기까지 확인한다. base-ipad.xcresult 본문 3.23초. 지금 최종 치트/정식 앱 설치와 직접 화면 확인을 이어간다.

01:53 추가: 기지 창 위로 생산 아이콘이 비치는 레이어 문제를 직접 발견하고 원본 layer 7<9 순서로 수정했다. core 2/2 통과. 마지막 수정의 기기 재빌드 전 Mac이 잠김(ioreg 확인). 사용자에게 해제 요청했고 시뮬레이터 검사는 계속한다. 최신 설치본은 창 위치/배경 수정까지이며 레이어 수정은 아직 미설치다. iPhone에는 계속 접근하지 않는다.

마지막 레이어 수정의 시뮬레이터 검사는 통과했다(`base-layer-simulator.xcresult`). Mac 잠금은 재확인됐고, CLI 기기 서명은 `errSecInternalComponent`로 실패했다. 후속 순서: 잠금 해제 후 Xcode GUI 기기 빌드 → iPad 화면/입력 검사 → 정식·치트 설치 → 직접 기지 강화 구매/닫기 및 Save & Exit → 앱 종료·홈 복귀. iPhone은 사용하지 않는다.

## 2026-11-05 지형·이동 정밀진단

미션 10·11 원형 구조물 정체와 아군 터렛 밀집 정체를 사용자 피드백으로 기록했다. 코어에서 터렛 없는 MachineGun 단독 정체를 두 미션 모두 재현했다. 원본 MapInfo 바이트는 일치한다. addColVector 네 번째 검사점은 원본과 다르며, 그 점만 원본대로 바꾼 로컬 시험으로는 미션 10 정체가 풀리지 않았다. 제품 코드 수정은 아직 없다. [진단·다음 대조](docs/evidence/terrain-motion-audit-20261104.md). 포팅판 실행 대조를 위해 기존 iPhone Air 시뮬레이터를 부팅했으나 CUA가 Simulator 번들 ID와 Xcode 내 앱 경로 모두 Invalid app으로 반환했다. 사용자 기기는 조작하지 않았다. 기존 iPad 마지막 HUD 레이어 설치·검사도 아직 남아 있다.

## 추가 사용자 지시 — 오른쪽 HUD·모듈화 준비, 2026-11-05

오른쪽 HUD는 사용자가 iPad의 입체감 있는 처리를 선택했다. iPhone의 회색 반복 연장과 설치 버전 차이를 함께 확인하고 수정한다. MD-HUD-DEPTH-01. HeavyMach 1의 모듈화를 심층 대조해 MachDefense의 피해·아군 메크/적 이동·사거리·표적 선택·충돌 등 공용 모듈 후보와 의존성, 분리·검증 순서를 조사한다. MD-MODULE-AUDIT-01. [TODO 상세](docs/usability-todo.md#추가-요청--2026-11-05)에 등록했으며 모듈 구현은 아직 착수하지 않았다.

## 오디오 교체와 모듈화 검사 — 2026-11-05

MD-AUDIO-BACKEND-01을 모듈화 단계에 포함한다. 원본 IPA는 Apple OpenAL 호출, 포팅판은 touchHLE→OpenAL Soft, 새 소스판은 Apple OpenAL 직접 연결임을 확인했다. 포팅판의 검증된 구현과 원작 재생 절차를 재사용하도록 수정할 과제다. 시뮬레이터 일반/치트 빌드와 실제 출력음·전체 기능·저장·프레임 검사를 주 검증으로 사용한다. 오디오 교체와 모듈 분리는 순차 커밋·비교하며 마지막에 iPad 음향 중단/복귀·지연을 확인한다. 아직 코드 교체·모듈 분리·develop 병합을 하지 않았다. HeavyMach 모듈화 이력 조사에는 feat/apple-workspace-modules, 58ade7ae, 63afa4f5를 포함한다.

## 공용 모듈 진단 보고서 — 2026-11-05

[정밀진단](docs/module-audit-20261105.md)과 [Core 58파일 해시/include 목록](docs/evidence/module-inventory-20261105.json)을 작성했다. 문서 진입에 modules 도메인을 추가했다. 목표는 Apple 유니버설·Android 네이티브·웹/PWA·Tauri Mac이다. 실제 모듈 분리·오디오 교체·develop 병합은 아직 수행하지 않았다. 보고 사이트는 기존 MachDefense 페이지에 이어 올린다.

보고서 게시 완료: Sites 판본 159, 배포 `appgdep_6aeb7a6e77e081918c6035d55590ac21` succeeded. [게시 기록](docs/report-site.md). 로컬 화면은 확인했고 운영 Chrome은 네트워크 오류로 직접 확인이 남았다.

긴급지원 조달은 MD-RESUPPLY-01로 등록했다. 기존 인앱결제 버튼을 대체하고 30·45·60초 활성화, 크레딧/셀/혼합 지급, J·B·P의 영문 격려문, 원작 글꼴·타이핑·스킬 글로우를 사용한다. 규칙·수령 상태·중복 지급 방지는 공통 C++에 둔다. 게임 시간과 기존 자원 갱신을 사용하고 플랫폼별 UI·타이머·지급 코드를 만들지 않는다. 지급량·반복 수령·재진입 정책은 미확정이다. 구현은 아직 시작하지 않았다.


## 2026-11-05 생산 HUD·Base Upgrade 후속

생산 HUD 7개 제한 원본 대조·세로 스크롤·순서 편집 연구를 MD-PRODUCTION-HUD-01로 등록했다. Base Upgrade 6종의 단계별 비율과 원본 자료형·주기는 [증거](docs/evidence/base-upgrade-rates-20261105.md)에 정리했다. 현재 강화 표·연산에서 차이를 발견하지 못했고, 10단계 × 61분 가속 검사에서 생산 감소가 없었다. 실제 포팅판 영상 대조는 남았다. 게임 코드는 이번 조사에서 변경하지 않았다.
