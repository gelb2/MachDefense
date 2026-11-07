# MachDefense Web / PWA

<!-- language-nav:start -->
[한국어](README.md) | [English](README.en.md)
<!-- language-nav:end -->

[플레이·PWA 설치](https://gelb2.github.io/MachDefense/) · [사용자 설치 안내](../docs/public/build-and-install.md)

같은 C++20 Core·효과음·자원을 Emscripten으로 빌드한다. WebGL2 출력과 브라우저 입력은 `src/`, 시작 화면·IndexedDB·service worker는 `../Distribution/Web/`에 있다. 게임 메뉴·HUD·복원 버튼은 공용 C++에서 그린다. Tauri 실행기는 아직 없다.

[v0.5 검사 결과](../docs/evidence/v05-release-20261107.md) · [최초 구현 검사](../docs/evidence/web-pwa-20261106.md) · [개발 계획](../docs/save-transfer-web-tauri-plan.md).

## 빌드

저장소 루트에서 실행한다. Emscripten 6.0.13으로 검사했다. 자원은 일반 Git 파일로 함께 받는다. Git LFS와 원본 IPA는 필요하지 않다.

```sh
# 설치한 Emscripten SDK의 emsdk_env.sh를 먼저 source 한다.
emcmake cmake -S Web -B build/web-normal -DMD_WEB_CHEAT=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build/web-normal -j6
python3 Web/tools/package.py build/web-normal/dist
python3 -m http.server 8941 --bind 127.0.0.1 --directory build/web-normal/dist
```

Mac 브라우저에서 `http://127.0.0.1:8941/`를 연다. 다운로드가 끝나면 PLAY IN BROWSER로 시작하거나 설치 버튼을 사용한다. 모바일 PWA 설치에는 기기에서 접근할 수 있는 HTTPS 주소가 필요하다. 공개 주소는 https://gelb2.github.io/MachDefense/ 이다.

치트판은 별도 빌드 폴더와 포트를 쓴다. 일반판과 치트판은 IndexedDB 이름도 다르다.

```sh
emcmake cmake -S Web -B build/web-cheat -DMD_WEB_CHEAT=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build/web-cheat -j6
python3 Web/tools/package.py build/web-cheat/dist --cheat
python3 -m http.server 8940 --bind 127.0.0.1 --directory build/web-cheat/dist
```

## 저장과 복원

슬롯 화면에서 GET CODE → COPY로 원본 5슬롯 전체를 내보낸다. ENTER CODE에 코드를 붙여넣고 게임 화면의 CHECK를 누른다. 확인 창에서 YES를 누르면 전체 슬롯을 교체한다. 교체 전 저장은 IndexedDB의 restoreBackup에 남긴다. 취소·잘못된 입력·기록 실패 때는 기존 진행을 유지한다. 단일 슬롯과 확장 설정은 아직 지원하지 않는다.

진행은 IndexedDB, 설치 파일은 Cache Storage에 둔다. 저장 트랜잭션이 완료된 뒤 게임에 성공을 알린다. 자원 캐시 재다운로드·PWA 업데이트 후에도 진행을 유지한다. 사용자가 사이트 데이터를 삭제하면 복원 코드로 다시 가져올 수 있다.

## 검사

```sh
npm ci --prefix Web/tests
cd Web/tests
npx playwright install webkit
cd ../..
node Web/tests/transfer-ui.mjs .local/pwa-20261106 http://127.0.0.1:8941/
node Web/tests/smoke.mjs
node Web/tests/game-flow.mjs .local/pwa-20261106
node Web/tests/pwa-storage.mjs .local/pwa-20261106
MD_BROWSER=webkit node Web/tests/regression.mjs .local/pwa-20261106 http://127.0.0.1:8941/
```

현재 Chrome 검사는 macOS 설치 경로를 사용한다. `regression.mjs`와 `pwa-storage.mjs`는 검사 폴더의 `iphone/MachDefense.sav` 복사본을 읽고, regression은 같은 폴더의 `recovery.txt`도 읽는다. 사용자 저장을 Git에 넣지 않는다. 빈 진행 검사에는 smoke/game-flow를 사용한다. 초기 프로필·생산·승패·재시작은 game-flow, 저장 복원·생명주기·GPU는 regression, 자원 손상·업데이트·캐시 삭제는 pwa-storage가 검사한다.

Galaxy 설치형 PWA의 복원·오프라인 재실행은 검사했다. 사용자는 iPhone → iPad PWA → iPhone 코드 복원과 미션 진입을 확인했다. iPhone 설치형 PWA의 직접 검사와 기기별 장시간 검사는 남아 있다. Apple·Android에도 같은 코드 메뉴를 연결했다. [연결과 검사 결과](../docs/evidence/save-transfer-all-platforms-20261106.md).

## 네이티브/Wasm 화면 수치 대조

`tests/frame-parity.cpp`를 각각의 Core 정적 라이브러리와 링크한다. include 경로는 `Sources`, `Tests`이며 C++20·예외·assert를 켠다. Wasm은 NODERAWFS와 Node 환경을 사용한다. 각 실행 파일에 `Resources`와 출력 바이너리 경로를 인자로 준다. 같은 출력 폴더에 `frame-native.bin`, `frame-wasm.bin`을 만든 뒤 실행한다.

```sh
python3 Web/tests/compare-frame.py .local/pwa-20261106
```

144,000프레임의 비실수 데이터는 정확히 대조하고 실수 차이의 최대값을 기록한다. 현재 검사 한도는 0.001이다. 원래 전체 비트 해시 결과도 보존하며 이 비교 때문에 게임 계산을 바꾸지 않는다.
