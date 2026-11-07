# TODO와 문서 점검 — 2026-11-07

v0.5 배포 뒤 TODO와 문서의 현재 상태를 대조했다. 게임 코드·그림·음원·저장 형식은 변경하지 않았다. 개발자의 변 최종본도 그대로 보존했다.

## TODO

기존 817줄의 체크 기록 324개를 현재 20개 과제와 완료·보류·제안으로 정리했다. 사용자 요구·글감·과거 결과는 [TODO](../usability-todo.md) 안에 보존했다. 체크박스는 당시 상태를 나타내는 문구로 바꿨다. 정리 커밋은 `30b6b3e`다.

## 바로잡은 설명

| 문서 | 기존 문제 | 대조와 수정 |
|---|---|---|
| README·기능 소개 | 10레벨·전체 생산 목록을 미구현으로 표시. 락온은 초기 일격 처치 구상만 설명 | v0.5의 완료 범위와 최신 크로스파이어·메크별 3초 요구로 수정 |
| 기능 소개·기획·모듈 안내 | Apple·Android 복원 메뉴가 미연결이라고 설명 | 실제 MDApplication·Android Runtime·Web Main의 공용 코덱·메뉴·저장 연결과 대조 |
| 강화 그림 기획·기능 소개 | 모든 추가 단계가 최종 원작 외형을 쓴다고 설명 | 원작 Lv.5 무장 + 채택한 새 포탑 몸체, 기지 추가 부품으로 수정. 기각 시안을 현행으로 취급하지 않음 |
| 웹 빌드·설치 | 불필요한 `git lfs pull`, 이전 v0.4 APK 링크 | 일반 Git 자원 안내와 최신 릴리스 링크로 수정 |
| Web·검사 안내 | 모든 모바일 PWA 검사가 남았다고 설명 | Galaxy 검사와 사용자 iPhone↔iPad PWA 확인, 남은 iPhone PWA 직접 검사를 구분 |
| Apple·Android 안내 | 실기기 캡처에 simctl 안내, 초기 무기·자원 수를 현행 고정값으로 사용 | 시뮬레이터·실기기 캡처를 구분. 현재 표·자원 목록을 소비한다고 설명 |
| Android 보존 검사 | 최초 이식 SHA 기준을 후속 공용 변경에도 무변경 검사처럼 안내 | 최초 이식 당시 증거로 한정. 승인된 복원·강화·그림 변경으로 생긴 차이를 숨기지 않음 |
| 설계·복원 계약 | 예정 Mac 프로젝트와 구현된 플랫폼이 혼재. 원본 수치 보존과 승인된 확장 관계 불명확 | 미구현 Mac/Tauri를 표시하고 승인 추가 기능을 기획에 연결 |
| 초기 계획·원고 사본 | 과거의 “현재”, “최신”, 미완료 문장이 현행 지시처럼 보임 | 작성 시점과 현행 문서 링크를 추가. 과거 수치·사용자 원문은 보존 |
| 무기·전환 검사 절차 | 치트앱 자동검사·전체 무장 검사를 아직 미구현으로 표시 | 실제 Apple 자동검사·v0.5 결과와 남은 동일 조건 성능 대조를 구분 |

## 코드와 검사 근거 연결

| 내용 | 소비 코드 | 기존 실행 근거 |
|---|---|---|
| 복원 코덱·레벨 검증·파일 교체 | [SaveTransfer](../../Modules/SaveTransfer/README.md), [Menu](../../Sources/Core/Presentation/Menu.cpp), [GameSession](../../Sources/Core/Session/GameSession.cpp) | [플랫폼별 복원](save-transfer-all-platforms-20261106.md), [추가 레벨 복원](level10-recovery-20261107.md) |
| Apple·Android 복원 서비스 | [MDApplication](../../Sources/Platform/Apple/MDApplication.mm), [Runtime](../../Android/app/src/main/cpp/Runtime.cpp) | [기기 검사](save-transfer-all-platforms-20261106.md) |
| 웹 생산 목록 휠 입력 | [Main](../../Web/src/Main.cpp), [app.js](../../Distribution/Web/app.js), [GameHUD](../../Sources/Core/Presentation/GameHUD.cpp) | [HUD 검사](level10-hud-20261107.md) |
| 레벨별 성능·그림 | [수치 계획](../research/level10-plan-20261107.md), [그림 명세](../../Art/Level10/20261107/runtime-art.json) | [사거리·피해](level10-range-damage-20261107.md), [몸체 연결](turret-body-only-integration-20261107.md), [v0.5](v05-release-20261107.md) |
| PWA 버전 전환 | [service worker](../../Distribution/Web/sw.js), [저장](../../Distribution/Web/storage.js) | [배포 후 첫 갱신 실패](v05-release-20261107.md#운영-배포-후-검사) |
| Tauri | [예정 폴더](../../Desktop/Tauri/README.md) | 앱 없음. 구현·파일 저장·종료·업데이트 검사는 남음 |

업무 연결표에는 실제 SaveTransfer 파일·소비 코드·검사, 현재 강화·그림 근거, Tauri 예정 범위를 추가했다. 다음 작업자는 초기 계획만 읽고 현행 구현을 놓치지 않도록 연결했다.

## 링크 검사 보강

기존 검사기는 README·진입 문서·docs 바로 아래 Markdown만 확인했다. 하위 문서에서 누락된 대상 12개와 존재하지 않는 절 앵커 1개를 찾았다. 보고서 이미지·과거 화면 기록은 기존 보고 사이트의 실제 보관 경로에 연결했고, 저장소 안의 잘못된 상대경로는 해당 근거 파일로 고쳤다.

검사기는 Git 추적 문서와 새 문서를 대상으로 하위 폴더의 로컬 인라인 링크·절 앵커를 확인한다. 보존한 타 프로젝트·스킬·외부 라이브러리 사본은 상대경로를 바꾸지 않는다. 스킬 원본은 별도 해시로 검사한다. 외부 사이트 응답과 문장 의미는 이 검사기의 범위에 포함하지 않는다.

- `python3 Tests/test_document_contract.py`: 하위 폴더의 누락 파일·앵커, 코드블록 제외, 외부 주소 제외, 한글 경로, 중복 제목 등 3개 검사 통과.
- `python3 tools/document_contract.py check`: 업무 경로·로컬 문서 링크·절·스킬 출처 검사 통과.
- `python3 tools/check_modules.py`: 공용 Core 41개 파일 목록·플랫폼 헤더 분리·OpenAL 원본 317개 파일 보존 검사 통과.
- `git diff --check`: 공백 오류 없음.

이번 점검에서는 앱을 다시 빌드하거나 기기 검사를 새로 실행하지 않았다. 위 게임 검사 결과는 연결한 기존 실행 기록이다.

## 남은 문제

PWA 최초 갱신, 전장 카메라 휠·키보드 입력, 길찾기, 실제 기기 틱·셀 생산량, 녹음·장시간 성능 대조 등은 [현재 20개 과제](../usability-todo.md#현재-남은-과제)에 남겼다. 문서 정리로 게임 버그를 완료 처리하지 않았다.
