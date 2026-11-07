# HeavyMach·MachDefense 코드와 에셋 라이선스

두 저장소 모두 **소스코드와 에셋의 라이선스 조건을 나눴다. 새로 작성한 코드는 MIT다. 에셋은 MIT 적용 대상에서 제외하고 원작 권리를 유지한다. 에셋을 다른 게임에 사용하거나 판매할 권리를 부여하지 않는다.**

2026-11-07 최신 공개 main을 확인했다. HeavyMach는 `fa606b766983b36b7307d2513469c42576c1aa8f`, MachDefense는 `0d3f84443910eec6db125803c6e1bb3f4d94015a`다.

## 실제 문서 구성

| 항목 | HeavyMach | MachDefense |
|---|---|---|
| 새 코드 | LICENSE와 LICENSE-SCOPE: MIT | LICENSE와 LICENSE-SCOPE: MIT |
| 원작 그림·음원·게임 데이터 | ASSET-LICENSES: 원작 권리 유지, MIT 제외 | ASSET-LICENSES: 원작 권리 유지, MIT 제외 |
| 추가 그림·아이콘·게임 캡처 | 에셋 문서에서 별도 취급, MIT 제외 | 에셋 문서에서 별도 취급, MIT 제외 |
| 다른 게임에 에셋 사용·판매 | 해당 권리를 부여하지 않는다고 명시 | 해당 권리를 부여하지 않는다고 명시 |
| 외부 라이브러리 | THIRD_PARTY_NOTICES와 원래 라이선스 | THIRD_PARTY_NOTICES와 원래 라이선스 |
| 한국어·영어 문서 | 코드 범위·에셋·외부 고지 각각 제공 | 코드 범위·에셋·외부 고지 각각 제공 |

루트 LICENSE 두 개는 바이트까지 같다. SHA-256은 `33ee134b22e5be3986e829d5595d21ff2de970265825c21141c392870ee7a969`다. 두 저장소에서 LICENSE와 한·영 범위·에셋·외부 고지 총 14개 파일을 보존하고 대조했다.

HeavyMach 에셋 문서에는 “원작 에셋을 다른 게임에 사용하거나 단독 판매하는 권리를 이 문서에서 부여하지 않는다.”라고 적혀 있다. MachDefense에는 “이 문서는 해당 그림·음원·데이터를 다른 게임에 쓰거나 판매할 권리를 부여하지 않는다.”라고 적혀 있다. 영어판도 같은 내용을 담고 있다.

## MIT가 적용되는 범위

MIT는 해당 새 코드의 상업 이용을 허용한다. 이 허락에 원작 에셋은 포함되지 않는다. 두 저장소의 코드 범위 문서가 이를 구분한다. 비영리 방침과 공개 목적은 README·개발자의 변에 별도로 적혀 있다.

에셋에 CC BY-NC라는 이름의 표준 라이선스를 붙인 것은 아니다. 현재 적용 문서는 ASSET-LICENSES이며, 원작 권리 유지와 다른 게임 사용·판매권 미부여를 명시한다.

## 문서에서 남은 보완

HeavyMach의 코드 범위 표에는 Android·Web·Distribution·Desktop/Tauri 경로가 빠져 있다. 외부 고지에도 Gradle·Emscripten·Tauri 항목이 없다. 실제 main에는 Gradle 8.13, Emscripten 웹 빌드, Tauri 2.12.5가 있다. 기존 TODO에 등록된 플랫폼 확장 고지 보완이 아직 필요하다.

MachDefense에는 Android·웹 경로와 Gradle·Emscripten 고지가 들어 있다. 양쪽 OpenAL Soft·stb·touchHLE 코드는 각자의 LGPL·MIT·MPL 등 개별 라이선스를 따른다. 이번 검사는 공개 문서와 주요 빌드 의존성 대조이며, Tauri의 모든 간접 의존성을 새로 조사한 결과는 아니다.

이번에는 라이선스·사용자 원고를 수정하지 않았다.

## 확인한 원문

- HeavyMach 코드: https://github.com/gelb2/HeavyMach/blob/fa606b766983b36b7307d2513469c42576c1aa8f/LICENSE-SCOPE.md
- HeavyMach 에셋: https://github.com/gelb2/HeavyMach/blob/fa606b766983b36b7307d2513469c42576c1aa8f/ASSET-LICENSES.md
- MachDefense 코드: https://github.com/gelb2/MachDefense/blob/0d3f84443910eec6db125803c6e1bb3f4d94015a/LICENSE-SCOPE.md
- MachDefense 에셋: https://github.com/gelb2/MachDefense/blob/0d3f84443910eec6db125803c6e1bb3f4d94015a/ASSET-LICENSES.md
- MIT 원문: https://opensource.org/license/mit
- CC BY-NC 4.0 원문: https://creativecommons.org/licenses/by-nc/4.0/

[통합 보고서](https://heavymach-dev-report-20261025.hiphopxy.chatgpt.site/machdefense.html#license-audit-20261107)
