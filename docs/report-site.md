# 보고와 Sass 전달

MachDefense 보고는 기존 HeavyMach 통합 보고 사이트의 한 페이지를 계속 갱신한다.

- 사이트: https://heavymach-dev-report-20261025.hiphopxy.chatgpt.site
- MachDefense 페이지: https://heavymach-dev-report-20261025.hiphopxy.chatgpt.site/machdefense.html
- Sites project: `appgprj_6addbf55aea081918d3d182d7e496e88`
- 원고: [판본 비교와 복원 진행](editorial/machdefense-report.md)
- 근거: [판본 조사](evidence/version-audit-20261104.md)

기존 사이트의 소유자 전용 공개 범위를 유지한다. 다음 보고는 이 페이지와 목차에 누적한다. 실제 배포 결과는 HANDOVER-CURRENT.md에 기록한다. Sass 세션에는 출처·원고·캡처·현재 상태를 전달하며 해당 CMS는 수정하지 않는다. IPA·실행 바이너리·개인 저장·기기 식별자는 업로드하지 않는다.

## 최근 갱신 — 2026-11-04 23:58

[정식 앱 피드백 수정·iPad 검사](https://heavymach-dev-report-20261025.hiphopxy.chatgpt.site/machdefense.html#feedback-20261104)를 추가했다. 생산·효과음·완료 표시·기지 강화·핵폭탄·오디오 복귀·성능·검사 누락 원인과 남은 대조를 담았다. 실제 iPad 캡처 4장을 포함한다. Sites 배포 상태 succeeded, 기존 소유자 전용 접근 유지.

## 최근 갱신 — 2026-11-05 공용 모듈 진단

[공용 모듈 정밀진단](https://heavymach-dev-report-20261025.hiphopxy.chatgpt.site/machdefense.html#module-audit-20261105)을 기존 페이지에 추가했다. HeavyMach의 브랜치·모듈화 이력, MachDefense Core 58개 파일의 책임·의존성, 오디오 교체, 플랫폼별 연결, 분리·검증 순서와 긴급지원 조달의 공통 코드 배치를 담았다. 원고·파일 목록·TODO 다운로드도 연결했다.

- Sites 판본 159, 소스 `dff14afca0c7b888cba3bb80c9fa860c7236dd81`.
- 배포 `appgdep_6aeb7a6e77e081918c6035d55590ac21`, 상태 `succeeded`. 기존 접근 범위 유지.
- 로컬 Chrome에서 보고서 구조·표·다운로드 연결을 확인했다. 운영 주소를 연 Chrome은 `ERR_INTERNET_DISCONNECTED`를 표시해 운영 화면 직접 확인은 남았다.
- 업로드 용량 제한은 기존 PNG의 무손실 재압축으로 해결했다. 픽셀·파일 경로·기존 원고를 보존했다.


## 최근 갱신 — 2026-11-05 모듈 구현·검사

[모듈화 작업 결과](https://heavymach-dev-report-20261025.hiphopxy.chatgpt.site/machdefense.html#module-work-20261105)를 기존 페이지에 추가했다. 원본 오디오·텍스처 수정, Core/Audio 분리, 분리 전후 144,000틱 비교, C++·시뮬레이터 검사, Instruments·누수·10분 전투, 일반/치트 직접 조작과 남은 과제를 담았다. iPad 시뮬레이터 TEST 패널 캡처를 포함했다.

- 제품 커밋: 원본 절차 수정 `f623eba`, 모듈 분리·검사 `193ed2f`. `refactor/shared-core-audio`에 있으며 제품 원격 push·main 병합은 하지 않았다.
- 사이트 소스: `80ab6814b42fff5a48c969c674a1c775fb75a356`.
- Sites 버전: `appgprj_6addbf55aea081918d3d182d7e496e88~appgver_a4d13efe732c8191995f1923fb0e0602`.
- 배포: `appgdep_6aeb8ed1a4448191983376cd23fac27b`, 상태 `succeeded`. 기존 소유자 전용 접근 유지.
- 로컬 Chrome에서 최종 본문·표를 확인했다. 운영 주소의 Chrome은 기존 `ERR_INTERNET_DISCONNECTED`가 지속됐다. 게시 완료는 Sites의 성공 응답으로 확인했다.
- 첫 업로드는 확장 용량 제한으로 거절됐다. macOS 부가 메타데이터를 제외한 ustar로 다시 포장해 성공했다. 최종 확장 크기는 267,757,568바이트다. 기존 페이지·자료를 보존했다.

## 긴급지원 구현과 실기기 검사 — 2026-11-05

[긴급지원 보고](https://heavymach-dev-report-20261025.hiphopxy.chatgpt.site/machdefense.html#emergency-resupply-20261105)에 반복 지급 산식, 공통 코드, 실제 GPU 캡처, iPhone·iPad 프레임 수치, 직접 수령·저장 보존 결과와 남은 검사를 추가했다.

- 사이트 소스: `7e02d5b4e2a4f85e3dc900bf1a20e11bac0b0e9d`
- 배포: `appgdep_6aebd9a67af4819190b881f074f61b43`
- 버전: `appgprj_6addbf55aea081918d3d182d7e496e88~appgver_1124e23f1df48191ba02a78157d7c82f`
- Sites 응답: succeeded. 기존 소유자 전용 공개 범위를 유지했다.
- 최초 업로드는 용량 제한으로 거절됐다. 새 PNG 두 장을 무손실 WebP로 바꾸고 RGBA 픽셀 일치를 확인한 뒤 재게시했다. 기존 보고서·게임 자원은 삭제하지 않았다.
- 최종 HTML 문구·이미지 연결을 확인했다. 이 게시 직전에는 Mac 창과 브라우저가 도구에 나타나지 않아 최종 브라우저 화면 검사는 하지 못했다.

## GitHub 문서 수정 — 2026-11-06

[수정한 README와 연결 문서](https://heavymach-dev-report-20261025.hiphopxy.chatgpt.site/machdefense.html#md-copy-readme)를 게시했다. 긴급지원 기획 주체·mesabloo의 영향·Internet Archive 출처·자동 실행과 치트 검사·오디오 문제를 보강했다. 판본 선택과 후속 마무리 문장은 본문에서 제외했다. 방문자 원고에서 REA 소개와 Sass 집필 자료를 빼고, README 뒤에 연결 문서 전문을 배치했다. 해당 작업 자료는 개발 문서에서 보존한다.

- 사이트 소스: `13746ae48ef438dadfd6f2f06d19664d92efb8d2`.
- 배포: `appgdep_6aed099ad4388191b41390902c25a251`, `succeeded`.
- 버전: `appgprj_6addbf55aea081918d3d182d7e496e88~appgver_f927d8452ac081918d102942ddf4d529`.
- 기존 소유자 전용 접근을 유지했다. 실제 Safari에서 수정 문구·제목·문서 구분을 확인했다.
- macOS 부가 메타데이터를 제외한 묶음의 파일 내용 SHA를 대조했다. 확장 크기 268,369,920바이트.

## PWA·Tauri Mac 개발과 모듈 수정 계획 — 2026-11-06

[계획 보고서](https://heavymach-dev-report-20261025.hiphopxy.chatgpt.site/machdefense.html#save-web-tauri-plan-20261106)를 기존 MachDefense 페이지에 추가했다. 첫 화면 목차와 전체 보고서 목록에서도 연결한다. 공용 복원 모듈, 단일 웹 실행기, PWA/Tauri 저장, 문제 분류·수정 위치·브랜치·기존 플랫폼 회귀와 단계별 완료 기준을 담았다. 전체 계획·TODO 다운로드를 함께 갱신했다.

- 사이트 소스: `cf6b659813ef1df2cb26fde652d44e3cb4ad007b`.
- 배포: `appgdep_6aece1b85d44819197cc1fe84e28e518`, `succeeded`.
- 버전: `appgprj_6addbf55aea081918d3d182d7e496e88~appgver_0c0d963f54748191af80987a9c795c16`.
- 기존 소유자 전용 접근을 유지했다. 제품 소스·앱·에셋은 변경하지 않았다. 계획은 `docs/shared-save-web-plan`에 있다.
- 로컬 Chrome에서 새 절·구조도·본문을 확인했다. 새 다운로드 파일과 앵커, 문서 연결 검사가 통과했다. 게시 후 운영 주소를 여는 단계는 CUA native pipe 종료로 실패했다. 게시 성공은 Sites 응답으로 확인했다.
- 첫 업로드는 확장 크기 제한으로 거절됐다. 기존 JSON 두 파일의 공백만 줄였고 값 동일성을 검사했다. macOS 메타데이터를 제외하고 USTAR로 포장해 파일 내용의 SHA 동일성을 확인했다. 최종 tar 확장 크기는 268,328,960바이트다. 이미지·영상·이전 보고서는 보존했다.

## Web·PWA 구현과 검사 — 2026-11-06

[통합 보고서](https://heavymach-dev-report-20261025.hiphopxy.chatgpt.site/machdefense.html#web-pwa-20261106)에 실제 구현·검사 결과를 추가했다. 아이폰 Stage 36 복원, 기존 플랫폼 파일 대조, 37개 검사, 160조건의 게임 상태 및 전체 화면 수치 비교, 오프라인·업데이트·음향·GPU·저장 실패와 남은 모바일 실기기 검사를 담았다. 계획·TODO·검사 원고 다운로드도 갱신했다. 이전 HUD·타이틀 기록은 보존했다.

- 사이트 소스: `4fa9b4cc250a347bc635939fa414926a3e31c795`.
- 버전: `appgprj_6addbf55aea081918d3d182d7e496e88~appgver_4c631ebdf48c8191b0d90d49ed076a0d`.
- 배포: `appgdep_6aecf43711488191b93f78be356b8ee4`, `succeeded`.
- 기존 소유자 전용 접근을 유지했다. 보고서 게시이며 게임 공개 배포는 아니다.
- 로컬 Safari에서 새 본문·표·링크를 확인했다. USTAR 포장 전후 파일 내용 SHA가 같고 확장 크기는 268,342,784바이트다.
