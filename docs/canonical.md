# 정본

최신 사용자 지시 → 원본 및 [복원 계약](restoration-contract.md)·[기획](game-design.md) → [설계](design.md)·분야 정책 → 코드·검사 → [실행 상태](current-status.md) 순서로 대조한다.

현재 소스 재개발과 대조 기준은 **위메이드 1.0.3**이다. 최초 호환 실행에는 Mach Defense 1.0.5(kr.indieapps.MachDefense)를 썼다. HeavyMach 1의 함수 주소·프레임 시간·해상도·저장 슬롯·밸런스는 MachDefense 정본이 아니다. [원본 명세](source-provenance.md)와 새 분석 결과로 확정한다.

[문서 위계](document-governance.md), [업무 연결표](document-map.json), [복원 단계](restoration-plan.md), [TODO](usability-todo.md)를 따른다. 이전 프로젝트의 정책 사본은 docs/reference/heavymach1 아래에 출처로 보존한다. 그곳의 완료 기록·과거 지시·기기 식별자·도구 세션은 이 프로젝트에 적용하지 않는다.

## 광고 표시 금지 — 2026-11-04

사용자 지시: HeavyMach 1·HeavyMach 2·MachDefense 모두 광고를 표시하지 않는다. 원본 호환판과 소스 재구현판, 모든 플랫폼에 적용한다. 광고 배너·전면 광고·보상형 광고를 추가하지 않고 광고를 요청하지 않는다. 원본에 광고 API 호출이 있으면 광고를 표시하지 않는 호환 처리를 마련한다. 원본 IPA는 수정하지 않는다.

포팅판에서 사용자가 확인한 동작과 재개발 비교 항목은 [플레이 관측 기록](port-observations-20261104.md)에 모은다. 재개발·UI·그래픽 작업 전 읽고 관련 ID를 검사에 연결한다.

신규 버전의 공용 에셋 세트와 iPhone/iPad 유니버설 목표는 [에셋 사전조사 계획](universal-assets-plan.md)을 따른다.

## 영어 전용과 재개발 전 판본 조사 — 2026-11-04

신규 앱은 모든 에셋·인게임 텍스트·플랫폼 안내를 영어로 제공한다. 다국어 지원 기능은 범위에 없다. 소스 재개발 전에 [전체 판본 조사](version-audit-plan.md)를 수행하고 기준 코드·에셋과 재개발 계획을 정한다.

## 위메이드 기준 확정과 iPhone 우선 검사

사용자 후속 지시: 기존 indieapps 포팅판 → 별도 위메이드 1.0.3 포팅판 검사 → 소스 재개발 순서다. 최신 미션·에셋 기준은 위메이드판으로 정한다. 먼저 iPhone 시뮬레이터에서 검사한다. 기존 사용자 기기의 앱과 저장을 덮어쓰지 않는다. [판본 비교 근거](evidence/version-audit-20261104.md)와 [통합 보고 원고](editorial/machdefense-report.md)를 참고한다.


## 소스 재개발 착수

위메이드판 시뮬레이터 비교 후 사용자 지시로 [소스 재개발](source-reimplementation-plan.md)을 시작했다. HeavyMach 1의 C++20 공용 코어·Objective-C++ Apple 실행기·GPU 스프라이트 구성을 따른다. 원작 메뉴 버튼은 공용 게임 코드와 원본 이미지로 그린다. 이름·복원 코드 입력과 클립보드 등 OS 기능만 플랫폼에 연결한다. 영어 전용, 광고 금지, IPA 없는 제품 빌드, 사용자 앱·저장 보존을 유지한다. 후속 웹판도 같은 코어·에셋을 사용하도록 플랫폼 경계를 둔다.

## 재개발 검증 순서 — 2026-11-04 후속 지시

포팅판 대조 → 인게임 일러스트 앱 아이콘 제작 → 아이콘 방향·가격·회전 재검사 → 치트앱 전체 기능 연속 검사 → 성능·음향·메모리·생명주기 검사 → iPhone·iPad 화면 확장 계획 수립 순서로 진행한다. 화면 확장 구현·재검사는 계획 수립 이후다. 세부 조건은 [소스 재개발 계획](source-reimplementation-plan.md)을 따른다. 일반·치트 앱은 같은 게임 코드를 사용하고 저장 공간을 분리한다.

## 원본 수정과 모듈화 — 2026-11-05

사용자 지시 순서는 원본·포팅판의 검증된 절차로 차이를 수정하고, 수정본을 검증·커밋한 뒤 공통 코드를 분리하는 것이다. [모듈 구조](../Modules/README.md), [실행 근거](evidence/module-work-20261105.md)를 따른다. 게임 규칙과 인게임 UI를 공용 Core에 둔다. 후속 Android·웹/PWA·Tauri 실행기는 플랫폼 연결만 구현한다. 긴급지원은 기존 게임 시간·자원·버튼·글꼴로 구현했다. [지급 산식](emergency-resupply-economy.md)과 [실행 검사](evidence/emergency-resupply-20261105.md)를 따른다.

## Android — 2026-11-06

Android 실행기는 기존 Core·Audio·자원을 그대로 사용한다. 게임 UI도 공통 Frame으로 그린다. 플랫폼 연결은 Android/에 둔다. Android 최초 이식에서는 Apple·기존 공통 코드·자원을 보존했다. 이후 사용자 승인으로 추가한 공용 복원·강화·그림 변경은 해당 후속 기록을 따른다. [구현·검사](evidence/android-native-20261106.md), [빌드](../Android/README.md).

## 복원 코드·웹/PWA·Tauri 계획 — 2026-11-06

[공용 복원 모듈과 웹 실행기 계획](save-transfer-web-tauri-plan.md)을 따른다. 복원 형식·검증·절차와 인게임 UI는 공용으로 두고, 웹·PWA·Tauri는 하나의 웹 실행 코드를 사용한다. Web·PWA는 `feat/web-pwa`에서 구현·로컬 검사했다. 이 절은 최초 로컬 구현 시점의 기록이다. 이후 모바일 검사와 운영 배포는 아래 완료 기록 및 최신 실행 상태를 따른다. Tauri는 아직 미구현이다. [실행 근거](evidence/web-pwa-20261106.md).

## 웹·PWA 공개 — 2026-11-06

사용자가 저장소 공개와 PWA 운영 배포를 지시했다. 게임 주소는 https://gelb2.github.io/MachDefense/ 이며 독립 gh-pages 브랜치의 정식 산출물을 사용한다. [배포 검사](evidence/pwa-release-20261106.md)에서 실제 확인 범위와 Tauri 착수 조건을 확인한다.

## 복원 코드 3플랫폼 연결 조사 — 2026-11-06

[동시 연결 계획](save-transfer-native-plan-20261106.md)에 착수 당시 공용 코덱·네이티브 미연결 상태와 백업 복구·게임 버튼 보강 계획을 기록했다. 현재 연결 결과는 다음 절에 있다. [외부 웹 공간 조사](research/lamplighter-20261106.md)는 참고 자료이며 게임 요구나 기술 스택을 변경하지 않는다.

## 복원 코드 연결 완료 — 2026-11-06

iPhone·iPad·Android·Web/PWA는 같은 MD1 코덱과 공용 게임 메뉴를 사용한다. 플랫폼은 코드 텍스트 입력·키보드·클립보드·파일 또는 IndexedDB 저장을 연결한다. 게임 액션은 원작 에셋과 C++ 입력으로 처리한다. [현행 구현·기기별 검사·배포](evidence/save-transfer-all-platforms-20261106.md)를 따른다. 전투 중 복원 작업이나 추가 저장을 실행하지 않는다.

## 현행 배포·추가 기능 — v0.5

배포 범위는 iPhone·iPad·Android·웹/PWA다. 복원 코드는 전체 5슬롯을 공용으로 처리한다. 영구 강화 상한 10, 생산 목록 스크롤, 레이저 해금과 추가 단계 그림을 적용했다. 포탑 Lv.6–10은 원작 Lv.5 무기부와 채택한 새 몸체를 사용한다. [기획](game-design.md) · [v0.5 배포 근거](evidence/v05-release-20261107.md).

Tauri Mac과 새 락온은 미구현이다. PWA 구버전 캐시의 최초 갱신 문제도 남았다. 작업 완료·보류·제안은 [TODO의 현행 목록](usability-todo.md)에서 구분한다. 날짜가 붙은 초기 계획과 증거의 미완료 표시는 그 시점의 상태이며 최신 요구를 덮지 않는다.
