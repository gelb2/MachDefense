# HeavyMach1과 MachDefense 라이선스 비교

2026-11-07. 사용자 지시로 HeavyMach1과 같은 라이선스 구성을 적용할 수 있는지 확인했다. 새 코드 MIT, 원작·추가 그림의 별도 구분, 외부 코드 고지 구조를 MachDefense에 적용했다.

## 대조한 자료

HeavyMach1의 `LICENSE`, `LICENSE-SCOPE.md`, `ASSET-LICENSES.md`, `THIRD_PARTY_NOTICES.md`와 README를 읽었다. 로컬 사본은 `build/cydia-docs`의 `a6842457`이다. MachDefense에는 최상위 LICENSE가 없었고 OpenAL Soft·stb·복사 스킬의 파일별 고지만 있었다.

[MIT 원문](https://opensource.org/license/mit)은 사용·수정·배포·판매를 허용하며 저작권과 허락 고지를 유지하도록 한다. HeavyMach1과 같은 표준 MIT 원문을 새 코드 기여분에 적용했다. 원작 그림·음원·데이터·명칭과 일반 문서 원고를 코드 MIT에 포함시키지 않았다. 저자의 비영리 방침은 개발자의 변에 그대로 두었다.

OpenAL Soft의 `al/source.cpp`는 LGPL 2 또는 이후 버전을 명시한다. 저장된 COPYING 제6절과 BSD-3Clause를 읽고, Apple 정적 빌드와 Android 정적 링크·`opensl-resume.cmake`의 사본 패치를 확인했다. 외부 소스를 바꾸거나 MIT로 재표시하지 않았다. 소스와 빌드 경로를 고지에 연결했다.

stb_image의 끝에 있는 MIT/Public Domain 선택 문구에서 MIT 원문을 보관했다. touchHLE 패치에는 MPL-2.0을 유지한다. [Mozilla 설명](https://www.mozilla.org/en-US/MPL/2.0/FAQ/)과 기존 패치의 파일 경계를 대조했다. 같은 패치의 SDL 부분에는 zlib, rust-sdl2에는 MIT를 따로 적었다.

MachDefense의 Android Gradle 8.13 wrapper는 Apache-2.0이다. [공식 배포 LICENSE](https://github.com/gradle/gradle/blob/v8.13.0/LICENSE)와 실제 wrapper JAR 안의 META-INF/LICENSE를 보관했다. 웹은 OpenAL Soft 바이너리가 아닌 Emscripten `-lopenal` 구현을 사용한다. 설치된 SDK의 [Emscripten 고지](https://github.com/emscripten-core/emscripten/blob/main/LICENSE)와 musl COPYRIGHT도 보관했다.

## 반영

- [LICENSE](../../LICENSE): HeavyMach1과 같은 저작권자 표기의 MIT 원문.
- [코드 적용 범위](../../LICENSE-SCOPE.md), [자원 조건](../../ASSET-LICENSES.md), [외부 고지](../../THIRD_PARTY_NOTICES.md)와 대응 영어판.
- README의 **Code license MIT** 배지와 라이선스 절, 한·영 목차의 고지 링크.
- 고지 원문 11개의 [출처·SHA-256](../license-notice-sources.json).
- 한·영 대응 목록과 검사 대상은 26쌍에서 29쌍으로 확대.

원작 자원의 별도 이용허락 문건은 발견하지 못했다. 해당 자료를 MIT로 재허락하지 않았고, 권리 확보 완료로 기록하지 않았다. 앱·PWA 패키지와 기존 v0.5 태그는 이번 문서 작업에서 변경하지 않았다. 라이브러리 변경 후 재링크 실험은 이번 작업에서 새로 실행하지 않았다.

## 확인

고지 원문 11개의 SHA-256 대조, 29쌍 번역 검사, Markdown 165개·링크/절 1,559개 검사와 문서 검사기 회귀 9개가 통과했다. 게임 실행 파일·자원은 변경하지 않았다.

## 공개된 main 재대조 — 2026-11-07

MachDefense `c87cc732e1260585f18b383e193382f00c01407e`와 HeavyMach `fa3c1a519d2a775d77dd62d5085b08fb86dcf403`의 공개 파일을 다시 받아 대조했다. 위 로컬 사본 조사 이후의 확인이다.

| 항목 | HeavyMach | MachDefense | 비교 결과 |
|---|---|---|---|
| 루트 LICENSE | MIT, Copyright (c) 2026 Jun Young Jee | 같은 원문 | 바이트 일치 |
| 코드 범위 | 새로 작성한 코드에 적용, 외부 코드·원작 자료 제외 | 같은 구분 | 방법 동일. 실제 경로 목록은 프로젝트별로 작성 |
| 원작 그림·음원·데이터 | ASSET-LICENSES에서 별도 설명 | 같은 문서 구조 | 원작 자원을 코드 MIT에 포함하지 않음 |
| 외부 코드 | THIRD_PARTY_NOTICES와 LICENSES 원문 | 같은 구조, Android·웹 의존성도 기재 | 아래 후속 보완 차이가 있음 |
| GitHub 표시 | README MIT 배지, 한·영 범위·자원·고지 | 같은 표시·언어 전환 | 방문자가 코드와 자원의 조건을 따로 확인 가능 |

HeavyMach의 세부 범위 표에는 Android·Web·Distribution·Desktop/Tauri 경로가 없고, 외부 고지에는 Gradle·Emscripten·Tauri 관련 항목이 없다. 현재 README에는 이 플랫폼들의 배포가 안내돼 있다. **라이선스 구분 방식은 같지만, HeavyMach의 세부 목록은 플랫폼 확장 내용을 충분히 반영하지 못했다.** 실제 포함되는 의존성·버전·기여자 고지를 조사해 갱신할 후속 과제로 남긴다. 단순히 MachDefense 목록을 복사하면 HeavyMach의 Tauri 의존성과 외부 PR 기여분을 놓칠 수 있다. 이번 비교에서 HeavyMach 파일은 수정하지 않았다.

MachDefense 라이선스 문서는 develop→main으로 게시했고 해당 main CI가 통과했다. GitHub에서 README 배지→한국어 코드 범위→English→영어 자원 조건→한국어 자원 조건→외부 고지→English를 실제 클릭했다. 여섯 번의 링크 이동이 성공했다. 390×844 브라우저 화면에서 영어 README의 언어 링크와 배지 표시도 확인했다. 실제 휴대폰 검사를 뜻하지 않는다.

TODO는 기존 미완료 20건에 HeavyMach 라이선스 후속 보완 1건을 추가했다. 한·영 문서와 MachDefense 라이선스 적용은 완료 목록에 있다. PWA 첫 업데이트 전환, Tauri 앱 구현, 전장 스크롤·키보드와 기존 게임 검사는 미완료 상태로 유지한다. 게임 코드·자원·v0.5 배포 파일은 바꾸지 않았다.
