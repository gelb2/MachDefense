# 외부 코드와 라이선스 고지

<!-- language-nav:start -->
[한국어](THIRD_PARTY_NOTICES.md) | [English](THIRD_PARTY_NOTICES.en.md)
<!-- language-nav:end -->

## 게임과 빌드에 쓰는 코드

| 구성요소 | 위치·용도 | 라이선스와 원문 |
|---|---|---|
| OpenAL Soft 1.22.2 | `Modules/ThirdParty/openal-soft`, Apple·Android 효과음. 출처·해시는 `openal-soft-source.json` | 주요 소스 LGPL-2.0-or-later, 일부 BSD-3-Clause. [LGPL](LICENSES/OpenAL-Soft-LGPL-2.0.txt), [BSD](LICENSES/OpenAL-Soft-BSD-3-Clause.txt). 파일별 고지 유지 |
| stb_image | `Modules/ThirdParty/stb/stb_image.h`, PNG/JPEG 디코딩 | MIT/Public Domain 선택형 중 MIT 사용. Copyright © 2017 Sean Barrett. [MIT](LICENSES/stb-MIT.txt) |
| Gradle 8.13 wrapper | `Android/gradlew`, `gradlew.bat`, `gradle/wrapper/gradle-wrapper.jar` | Apache-2.0. [wrapper 원문](LICENSES/Gradle-wrapper.txt), [배포 고지](LICENSES/Gradle.txt), [공식 소스](https://github.com/gradle/gradle/tree/v8.13.0) |
| Emscripten | 웹 빌드와 OpenAL·파일 시스템 등 실행 지원 | MIT/NCSA 선택형. [설치 SDK 고지](LICENSES/Emscripten.txt), [공식 라이선스](https://github.com/emscripten-core/emscripten/blob/main/LICENSE) |
| musl | Emscripten의 C 라이브러리 | MIT와 파일별 고지. [SDK의 COPYRIGHT](LICENSES/musl-COPYRIGHT.txt) |

Apple·Android는 OpenAL Soft를 정적으로 연결한다. 라이브러리 소스·빌드 설정과 앱 소스는 저장소에 있다. Android의 재개 수정은 [opensl-resume.cmake](Android/cmake/opensl-resume.cmake)가 빌드 폴더 사본에 적용한다. 라이브러리를 고쳐 다시 링크할 때 이 수정분도 함께 사용한다. 앱 빌드 경로는 [Apple](Apple/README.md)·[Android](Android/README.md)에 있다. LGPL 제6절에 따라 배포본과 대응하는 라이브러리·앱 소스와 재링크 자료를 제공하고, 해당 수정의 디버깅을 위한 역분석을 금지하지 않는다.

웹은 `Web/CMakeLists.txt`의 `-lopenal`로 Emscripten OpenAL 구현을 사용한다. OpenAL Soft 네이티브 바이너리를 웹에 넣지 않는다. 웹 검사 도구 Puppeteer·Playwright는 개발 의존성이며 각 npm 패키지의 고지를 따른다.

## 비교 실행기와 보관 자료

| 자료 | 출처와 적용 조건 |
|---|---|
| `Compatibility/patches`의 touchHLE 코드 | MPL-2.0. [라이선스](LICENSES/touchHLE-MPL-2.0.txt). 원본·수정 대상 커밋은 [upstream.json](Compatibility/upstream.json)에 기록 |
| 패치 안의 SDL 코드 | zlib. [라이선스](LICENSES/SDL-zlib.txt), [SDL2](https://github.com/libsdl-org/SDL/tree/SDL2). 원본 고지를 유지 |
| `Compatibility/patches/rust-sdl2.patch` | Copyright © 2013 Mozilla Foundation, MIT. [고지](LICENSES/rust-sdl2-MIT.txt), 정확한 커밋은 upstream.json |
| engineering-debug/review/test의 참고 스킬 | Copyright © 2025 Addy Osmani, MIT. [보관 고지](LICENSES/engineering-skills-MIT.txt). 각 스킬의 기존 LICENSE 유지 |
| 그 밖의 복사한 스킬·참고 문서 | [복사 출처와 해시](docs/skill-source-manifest.json)와 파일별 고지 유지. 루트 MIT에 포함하지 않음 |

호환 실행판의 원본 게임 패키지는 외부에 업로드하지 않는다. 외부 코드의 고지를 이 프로젝트 저자 이름으로 바꾸지 않는다. 고지 파일의 출처와 해시는 [목록](docs/license-notice-sources.json)에 있다.

[코드 적용 범위](LICENSE-SCOPE.md) · [자원 조건](ASSET-LICENSES.md)
