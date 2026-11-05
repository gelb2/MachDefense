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
