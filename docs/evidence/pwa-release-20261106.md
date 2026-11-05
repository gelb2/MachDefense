# PWA 배포 검사 — 2026-11-06

정식 게임 빌드: `753d96288ca122ee`. 소스는 `release/pwa-20261106`에서 기존 웹 구현과 최신 main 문서를 통합했다. 원본 패키지와 사용자 저장은 배포 입력에 없다.

## 완료한 로컬 검사

- 새 경로에서 Emscripten Release 정식 빌드 성공. 치트 기능 비활성.
- Chrome에서 설치형 PWA를 설치하고 독립 창, 타이틀·슬롯 화면, 오프라인 재실행 확인. 설치 가능성 오류 0개.
- Chrome·WebKit 정식판에서 아이폰 저장 복사본의 코드 입력·취소·확정·바이트 일치·재출력·잘못된 코드 거부 확인.
- Stage 36 진입, 백그라운드 복귀, GPU 컨텍스트 재생성, 오프라인 재실행, 오래된 창의 저장 거부, DB 오류 때 기존 저장 보존 통과.
- 배포 준비 중 Apple·Android·Resources·Sources·Modules·Tests와 Web C++ 코드는 `feat/web-pwa`와 동일하다. 이전 네이티브/Wasm 37개 및 160조건 대조 결과를 그대로 적용한다.

로컬 근거: `.local/release-check/installed-local/installed.json`, 이전 작업 사본의 `.local/pwa-20261106/browser-regression-chromium-normal.json`. 기기 원본 저장은 변경하지 않았다.

Safari에서도 PLAY IN BROWSER를 직접 눌러 정식 게임의 타이틀 진입을 확인하고 검사 탭을 닫았다.

## 배포 경로

HeavyMach1과 같은 독립 `gh-pages` 브랜치에 정식 WebAssembly·자원·웹 실행 파일만 올린다. `Web/tools/publish.py`가 파일 허용 목록과 SHA-256을 검사한다. 원본 패키지·개인 저장·개발 문서는 이 브랜치에 포함하지 않는다.

GitHub는 현재 비공개 저장소에서 Pages 생성 요청을 HTTP 422로 거절했다. 저장소 공개 또는 별도 웹 배포 저장소 선택을 사용자에게 요청했다. 운영 주소 검사와 운영 설치 검사는 아직 실행하지 않았다.

## Tauri Mac 착수 조건

게임 재작성은 필요 없다. 같은 정식 웹 산출물을 `Desktop/Tauri`에 포함할 수 있다. 현재 저장 호출은 `MDStore.current`와 비동기 `MDStore.write`로 모여 있고, C++은 완료 결과를 기다린다. Tauri용 어댑터가 같은 인터페이스를 제공하면 전투·메뉴·복원 코덱을 수정하지 않고 연결할 수 있다.

추가 구현은 Rust 명령을 통한 앱 데이터 경로의 파일 읽기·임시 파일 후 교체·복원 전 백업, 웹 시작 시 저장 어댑터 선택, 번들 자원의 직접 로딩, 창 종료 시 저장 완료 대기다. Tauri에서는 service worker 다운로드 절차를 실행하지 않고 포함된 자원을 읽는다. 인게임 버튼은 기존 C++ Frame을 사용한다.

Mac에 Cargo/Rust 1.99.1과 Xcode 27.1이 있다. WebKit의 현재 게임 실행 검사도 확보했다. 다음 브랜치에서 구현·빌드할 수 있으며, 아직 Tauri 앱을 만든 것은 아니다. 완료 판정에는 앱 종료·재실행·캐시 삭제 후 저장 유지·복원 코드 왕복·음향 복귀 검사가 필요하다.
