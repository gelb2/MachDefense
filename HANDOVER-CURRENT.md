# MachDefense 인계 — 2026-11-04

위치: `/Users/sokol/Git/ProjectGameDev-Hub/MachDefense`. 독립 비공개 저장소 `gelb2/MachDefense`다. 시작 문서는 AGENTS.md와 docs/canonical.md이며 현재 결과는 docs/current-status.md에 있다.

현재 브랜치는 `feat/source-reimplementation`이다. 위메이드 1.0.3 기준 C++ 재개발 앱의 메뉴·생산·전투·저장 연결을 검증 중이다. `docs/current-status.md`와 `docs/evidence/source-session-validation-20261104.md`를 먼저 읽는다. 아래 포팅판 기록은 이전 단계다. Mesabloo 포크 기반 개발이 아니다.

IPA 19개는 저장소 밖 원본 보관소에 있다. 파일을 저장소·원격·보고 사이트에 복사하지 않는다. `.local/originals.json`은 보관 경로만 담고 Git 제외다. 실제 스킬은 `.agents/skills`, HeavyMach 1 원본은 `docs/reference/heavymach1`에 있다.

새 clone에서는 tools/setup.py로 훅을 설치한다. docs/repository-management.md의 브랜치·반출 규칙을 따른다. HeavyMach 1·Unity·기존 hm-defense와 사용자 저장을 보존한다. 자동 새 세션 전환이나 별도 인계 승인을 요구하지 않는다.

HeavyMach 1·2·MachDefense 모두 광고를 표시하거나 요청하지 않는다. 호환판에도 적용한다. 원본 IPA는 수정하지 않고 광고 API는 로컬 실행기에서 비표시 처리한다.

## 호환판 구현·검사 인계

- 브랜치: `feat/ios-compatibility`. develop·main 병합과 원격 push는 수행하지 않는다.
- 원본: Mach Defense 1.0.5의 ARMv7 실행 파일을 touchHLE 인터프리터로 구동한다. 소스 재구현은 아직 시작하지 않았다.
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

다음에는 치트 검증 화면을 먼저 준비한다. 원작 기능·진형·전투 연출을 완성하면서80개 미션의 진행·보상·저장을 검사한다. 그다음 전체 회귀·장시간 성능을 검증하고, 이후 iPhone·iPad 화면을 확장한 뒤 재검사한다. 최신 순서는 docs/source-reimplementation-plan.md를 따른다. 새 소스판 전체 완료로 보고하지 않는다.

보고 사이트는 256MiB에 가까워졌다. 이번 archive는 macOS xattr·AppleDouble을 제외한 ustar로268406272바이트다. `.local/site-tar-bin/tar`를 PATH 앞에 두고 Sites source helper로 포장했다. 게임 자료와 캡처의 픽셀을 삭제·변경하지 않았다. 다음 이미지 추가 전에는 사이트 용량을 재정리해야 한다. 원본 IPA/바이너리는 계속 반출 금지다.

## 검증 순서 수정과 작업 보존 — 2026-11-04

사용자가 치트앱 계획 기록과 지금까지 작업의 커밋을 지시했다. `feat/source-reimplementation`에서 커밋하며 develop/main 병합·원격 push는 포함하지 않는다. 치트앱 준비 → 원작 기능 및80개 미션 진행·저장 검사 → 전체 검증 → iPhone/iPad 화면 대응 → 재검사 순서로 정본·계획·검증·TODO를 맞췄다. 통합 보고 원고에도 같은 순서를 반영했다. 이 순서 수정분은 로컬 원고이며 사이트에는 아직 재게시하지 않았다.

## 치트 앱·80개 캠페인 구현 후속 — 2026-11-04

`feat/source-reimplementation`에서 TEST5쪽과 ValidationTools를 구현했다. 최신 결과는 docs/evidence/source-cheat-campaign-20261104.md. 최종 CTest31종, Apple XCTest10건, 최신 일반·치트 빌드 성공. 80개 진행·저장과 치트 조건80개 교전을 나누어 검사했다. 총구·탄피·궤적·잔해를 연결했고 원본 진형 아이콘 no-op을 확인했다. 결과 저장 실패 후 Next/Retry/Select Stage 재시도와 TEST 오류창 입력도 수정했다.

사용자 기기 포팅판은 변경하지 않았다. 시뮬레이터 직접 검사를 마치고 앱 종료·홈 복귀·시뮬레이터 종료까지 확인했다. 같은 조건 포팅판 대조·일반 난이도 진행·장시간 메모리/프레임·독립 빌드가 남는다. 이 단계 후 화면 대응을 한다. 최종 로그 `.local/final-core-tests.log`, `.local/source-validation-ui-20261104.md`를 재사용한다.

기능 변경 커밋은31395fb다. 추적 파일만 git archive로 `.local/clean-source-31395fb`에 풀고 새 DerivedData에서 일반 iOS Simulator 앱 빌드 성공. `.local/clean-source-apple-build.log`를 보존했다. 같은 조건 포팅판 대조와 장시간 검사가 남으며 화면 확장은 아직 시작하지 않았다.

## iPad 오디오·UI 수정 작업 보존 — 2026-11-04

`feat/source-reimplementation`에서 오디오 컨텍스트 유지, 보상·오류창의 반복 파일 읽기 제거, 생산·보상 미리보기 회전, 강화 비용 정렬, iPad 두 세로 방향 지원을 수정했다. 치트 앱 전용 프레임 측정은 메모리에 수집하고 비활성 전환 때 CSV로 내보낸다. 일반 앱에서는 켜지지 않는다.

오디오 XCTest 2개와 관련 코어 회귀 7개가 통과했다. iPad 치트 앱의 수정 후 248.82초·14,912프레임 측정과 사용자 개선 확인은 `docs/troubleshooting.md`에 있다. `docs/usability-todo.md` 맨 위의 현재 작업 순서를 따른다. 전체 무기·오디오 중단 복귀·메모리 판정·일반 앱·전체 UI 및 실제 회전 재검사는 남았다. 화면 확장은 그 뒤다.

Xcode GUI가 바꾼 개인 서명 팀과 공유 스킴 형식 3개 파일은 로컬에서 보존하고 이번 기능 커밋에서 제외한다. `Apple/iOS/project.yml`과 두 Info.plist의 방향 수정은 커밋 대상이다. develop/main 병합은 하지 않는다. 원본 IPA·실행 바이너리·로컬 측정 원본은 Git에 넣지 않는다.
