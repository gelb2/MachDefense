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

위메이드판 시뮬레이터 비교 후 사용자 지시로 [소스 재개발](source-reimplementation-plan.md)을 시작했다. HeavyMach 1의 C++20 공용 코어·Objective-C++ Apple 실행기·GPU 스프라이트 구성을 따른다. 원작 메뉴 버튼은 공용 게임 코드와 원본 이미지로 그린다. 이름 입력만 OS 텍스트 입력을 사용한다. 영어 전용, 광고 금지, IPA 없는 제품 빌드, 사용자 앱·저장 보존을 유지한다. 후속 웹판도 같은 코어·에셋을 사용하도록 플랫폼 경계를 둔다.

## 재개발 검증 순서 — 2026-11-04 후속 지시

포팅판 대조 → 인게임 일러스트 앱 아이콘 제작 → 아이콘 방향·가격·회전 재검사 → 치트앱 전체 기능 연속 검사 → 성능·음향·메모리·생명주기 검사 → iPhone·iPad 화면 확장 계획 수립 순서로 진행한다. 화면 확장 구현·재검사는 계획 수립 이후다. 세부 조건은 [소스 재개발 계획](source-reimplementation-plan.md)을 따른다. 일반·치트 앱은 같은 게임 코드를 사용하고 저장 공간을 분리한다.
