# PWA 배포 검사 — 2026-11-06

정식 게임 빌드: `4662f418410f8fdb`. 소스는 `release/pwa-20261106`에서 기존 웹 구현과 최신 main 문서를 통합했다. 원본 패키지와 사용자 저장은 배포 입력에 없다.

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

최초 Pages 생성은 비공개 저장소 요금제 제한으로 HTTP 422를 반환했다. 사용자 공개 지시 뒤 저장소를 public으로 전환하고 gh-pages 루트를 배포했다. 운영 주소는 https://gelb2.github.io/MachDefense/ 이다.

첫 운영 설치 검사에서 manifest의 `id: "./"`가 사이트 원점으로 해석돼 MachDefense 하위 경로와 달랐다. 명시 id를 제거해 브라우저가 올바른 start_url을 설치 식별자로 사용하도록 수정했다. 최초 실패 로그를 보존했다. 게임 코드·저장소 이름·저장 형식은 바꾸지 않았다. 수정 후 운영 설치 검사에서 설치 오류 0개, 독립 창·슬롯 화면·오프라인 재실행을 확인했다.

## 운영 주소 검사

- GitHub Pages `dfee4ab4144b8fba622e731f32a9965783e3533f` 배포 성공. 정식 빌드 `4662f418410f8fdb`.
- 배포된 11개 파일의 크기와 SHA-256이 로컬 산출물과 모두 같다.
- Mac Chrome 설치형 PWA: 실제 설치·독립 창·타이틀·프로필 선택·오프라인 재실행 통과.
- Chrome 운영 정식판: 복원 취소·42,496바이트 복원·코드 재출력·잘못된 코드 거부·Stage 36 전투·복귀·GPU 재생성·오프라인 재실행·중복 창 충돌·DB 실패 보존 통과. 페이지 오류 0개.
- WebKit도 같은 운영 게임 코드에서 위 기능 검사를 통과했다. 이후 수정은 manifest의 설치 식별자뿐이며 게임 실행 파일과 저장 코드는 같다.

로컬 로그는 `.local/release-check/installed-production/installed.json`, `.local/release-check/production-final/browser-regression-chromium-normal.json`, `.local/release-check/production/browser-regression-webkit-normal.json`, `.local/release-check/production-final/deployed-files.json`이다. 사용자 저장 복사본과 복원 코드는 이 로컬 검사 폴더에만 둔다.

iPhone·iPad·Galaxy의 홈 화면 설치와 실제 터치·음향·장시간 검사는 후속 과제다.

## Tauri Mac 착수 조건

게임 재작성은 필요 없다. 같은 정식 웹 산출물을 `Desktop/Tauri`에 포함할 수 있다. 현재 저장 호출은 `MDStore.current`와 비동기 `MDStore.write`로 모여 있고, C++은 완료 결과를 기다린다. Tauri용 어댑터가 같은 인터페이스를 제공하면 전투·메뉴·복원 코덱을 수정하지 않고 연결할 수 있다.

추가 구현은 Rust 명령을 통한 앱 데이터 경로의 파일 읽기·임시 파일 후 교체·복원 전 백업, 웹 시작 시 저장 어댑터 선택, 번들 자원의 직접 로딩, 창 종료 시 저장 완료 대기다. Tauri에서는 service worker 다운로드 절차를 실행하지 않고 포함된 자원을 읽는다. 인게임 버튼은 기존 C++ Frame을 사용한다.

Mac에 Cargo/Rust 1.99.1과 Xcode 27.1이 있다. WebKit의 현재 게임 실행 검사도 확보했다. 다음 브랜치에서 구현·빌드할 수 있으며, 아직 Tauri 앱을 만든 것은 아니다. 완료 판정에는 앱 종료·재실행·캐시 삭제 후 저장 유지·복원 코드 왕복·음향 복귀 검사가 필요하다.
