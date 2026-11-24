# MachDefense Web / PWA

<!-- language-nav:start -->
[한국어](README.md) | [English](README.en.md)
<!-- language-nav:end -->

[플레이·PWA 설치](https://gelb2.github.io/MachDefense/) · [사용자 설치 안내](../docs/public/build-and-install.md)

같은 C++20 Core·효과음·자원을 Emscripten으로 빌드한다. WebGL2 출력과 브라우저 입력은 `src/`, 시작 화면·IndexedDB·service worker는 `../Distribution/Web/`에 있다. 게임 메뉴·HUD·복원 버튼은 공용 C++에서 그린다. [Tauri Mac 실행기](../Desktop/Tauri/README.md)는 같은 웹 산출물을 앱에 포함하고 진행을 Mac의 앱 데이터 폴더에 저장한다.

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

HeavyMach의 웹판을 기여한 [fastutlego](https://github.com/fastutlego)에게 감사드린다. [HeavyMach PR #24](https://github.com/gelb2/HeavyMach/pull/24)의 WebAssembly 실행부·WebGL2 렌더링·입력·음향·저장 구현과 그 뒤의 PWA·Tauri 확장 절차가 MachDefense를 웹과 데스크톱으로 확장하는 데도 토대가 됐다. MachDefense의 PWA를 이 구조를 참고해 만들었고, 같은 웹 실행기를 Tauri Mac 앱에서도 사용했다.

## 설치한 PWA 갱신

서비스 워커는 다음 판을 별도 캐시에 준비한 뒤 활성화한다. 내려받거나 재사용한 파일, 데이터 조각, 조립한 데이터 파일의 SHA-256을 각각 검사한다. 검증한 워커는 활성화 후 앱을 제어해 Safari에서도 다시 열면 시작할 수 있게 한다. 이미 검증한 파일은 캐시에 다시 쓰지 않는다. 새 판을 준비하지 못하면 해당 캐시와 보존 중인 판에서 쓰지 않는 조각을 지운다.

공개판 `f9f26e94f1d4e3d6`에서 처음 갱신할 때 새 판 준비에 실패하면 기존 캐시로 게임을 시작할 수 있다. 서비스 워커는 보관한 목록을 기존 공개판의 해시와 대조한 뒤 기존 게임 파일을 검사한다. 시작 화면을 다시 열면 갱신을 재시도한다. 이미 실행 중인 게임은 시작할 때 받은 판으로 계속 동작한다. 저장 데이터베이스와 저장 버전 검사는 유지한다. 배포 검사에서 번들 안의 에셋 경로를 승인한 입력과 대조하고, 공개 서버의 파일을 빌드 기록과 대조한다.

기존 저장으로 갱신과 실행을 확인한다.

```sh
node Web/tests/update-integrity.mjs build/web-normal/dist .local/update-integrity.json /path/to/test/MachDefense.sav
```

첫 슬롯을 사용 중인 시험용 저장 파일을 쓴다. 이 검사는 연결을 끊은 상태에서 시작 화면을 다시 열고, 디코딩한 슬롯과 저장 파일의 바이트를 함께 대조한다.

Chrome에서 저장 공간이 부족한 조건의 첫 갱신을 확인한다.

```sh
MD_QUOTA_HEADROOM=2100000 node Web/tests/legacy-update.mjs /path/to/published-web build/web-normal/dist .local/legacy-quota.json /path/to/test/MachDefense.sav
```

공개판 폴더에서 `f9f26e94f1d4e3d6`을 실행한 뒤 브라우저 저장 공간을 제한한다. 새 워커가 같은 저장으로 기존 게임을 열 수 있는지 확인한다. 제한을 해제하고 다시 열어 같은 워커가 갱신을 마치는지도 확인한다.
