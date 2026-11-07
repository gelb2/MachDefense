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
