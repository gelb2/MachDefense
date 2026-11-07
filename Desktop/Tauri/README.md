# MachDefense Tauri Mac

<!-- language-nav:start -->
[한국어](README.md) | [English](README.en.md)
<!-- language-nav:end -->

Apple Silicon Mac에서 같은 WebAssembly 게임을 WKWebView로 실행한다. 게임·그림·음향은 앱에 포함한다. 첫 실행부터 인터넷 연결 없이 플레이할 수 있다. 진행은 브라우저 캐시 밖의 앱 데이터 폴더에 저장한다.

## 빌드

먼저 [웹 일반판](../../Web/README.md)을 빌드·포장한다. Rust와 Node.js, Xcode 명령행 도구가 필요하다.

```sh
npm ci --prefix Desktop/Tauri
npm run build --prefix Desktop/Tauri
```

결과는 `Desktop/Tauri/src-tauri/target/release/bundle/macos/MachDefense Desktop.app`이다. macOS 14 이상·arm64를 대상으로 한다. Windows 앱은 아직 만들지 않았다.

## 저장과 복원

`~/Library/Application Support/com.gelb2.machdefense.desktop/saves/`에 진행을 저장한다. `current.json`이 현재 저장, `backup.json`이 이전 저장, `restore-backup.json`이 코드 복원 전 저장이다. 임시 파일을 완전히 기록한 뒤 교체하고 읽어서 검사한다. 파일 크기·SHA-256·저장 순번을 확인한다.

게임의 슬롯 화면에서 GET CODE와 ENTER CODE를 사용한다. iOS·Android·PWA와 같은 MD1 코드와 다섯 슬롯을 사용한다. 메뉴·HUD·복원 버튼은 공용 게임 코드가 그린다. 텍스트 입력만 WebView 입력란을 사용한다.

## 업데이트와 배포

일반 소스 빌드는 서명 키 없이 만들 수 있다. 배포용 업데이트 파일은 개인키 경로를 `TAURI_SIGNING_PRIVATE_KEY` 환경 변수에 지정한 뒤 아래 명령으로 만든다. 개인키는 저장소에 넣지 않는다.

```sh
npm run release --prefix Desktop/Tauri
python3 Desktop/Tauri/scripts/release_manifest.py
```

새 버전이 있으면 시작 화면에 UPDATE 버튼을 표시한다. 게임 진행을 저장한 뒤 서명된 앱 전체를 내려받아 교체한다. Tauri 업데이트는 PWA의 변경 자원 조각 다운로드와 다르다. 업데이트 파일의 서명과 버전이 맞아야 설치한다. 게임 저장은 앱 번들 밖에 유지한다.

macOS 앱은 개발용 ad-hoc 서명이며 Developer ID 공증을 받지 않았다. 다운로드한 앱이 차단되면 시스템 설정 → 개인정보 보호 및 보안 → 확인 없이 열기를 사용한다. [검사 결과](../../docs/evidence/tauri-desktop-20261107.md).

## 검사

```sh
cargo test --manifest-path Desktop/Tauri/src-tauri/Cargo.toml
node Desktop/Tauri/tests/storage-host.mjs
MD_DESKTOP_AUTOTEST=1 cargo run --manifest-path Desktop/Tauri/src-tauri/Cargo.toml --features validation
```

`validation` 기능을 켠 검사는 별도 `validation/` 저장 폴더를 쓴다. 새 프로필·재실행·미션 진입·저장·종료를 실제 WKWebView에서 검사한다. 선택적으로 `MD_DESKTOP_RECOVERY_FILE`에 로컬 복원 코드 파일 경로를 지정한다. 코드를 Git에 넣지 않는다. 결과는 `validation/saves-cheat/diagnostics.json`에 쓴다.

## 웹판 기여

HeavyMach의 웹판을 기여한 [fastutlego](https://github.com/fastutlego)에게 감사드린다. [HeavyMach PR #24](https://github.com/gelb2/HeavyMach/pull/24)의 WebAssembly 실행부·WebGL2 렌더링·입력·음향·저장 구현과 그 뒤의 PWA·Tauri 확장 절차가 MachDefense의 웹·데스크톱 확장에도 토대가 됐다. MachDefense의 공용 게임을 같은 방식으로 웹에 연결하고, 그 웹 실행기를 Tauri Mac 앱에서도 사용했다.
