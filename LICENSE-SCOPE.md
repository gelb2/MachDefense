# 코드 라이선스 적용 범위

<!-- language-nav:start -->
[한국어](LICENSE-SCOPE.md) | [English](LICENSE-SCOPE.en.md)
<!-- language-nav:end -->

이 프로젝트에서 새로 작성한 코드 기여분에 [MIT](LICENSE)를 적용한다. 적용 범위는 해당 기여자가 보유한 권리까지다. 원작 자료와 외부 코드는 아래의 개별 조건을 유지한다.

## MIT 적용 대상

| 경로 | 적용 범위 |
|---|---|
| `Sources/Core`, `Sources/Audio`, `Sources/Platform/Apple` | 이 복원판에서 새로 작성한 게임·음향·Apple 연결 코드 |
| `Modules/SaveTransfer` | 새로 작성한 복원 코덱·파일 교체 코드 |
| `Android`, `Web`, `Distribution/Web`, `Apple`, `Modules`, `CMakeLists.txt` | 직접 작성한 플랫폼 연결·웹 화면·빌드 설정. 외부 도구와 자원은 제외 |
| `Tests`, `tools` | 직접 작성한 검사·빌드·문서 도구 |
| `docs/public/media/badges/*.svg` | 원작 그림을 포함하지 않는 문서용 SVG 코드 |

MIT는 저작권과 라이선스 고지를 유지하는 조건으로 코드의 사용·수정·재배포·상업 이용을 허용한다. 내가 이 복원판으로 돈을 벌지 않겠다는 방침은 [개발자의 변](docs/RESTORATION.md)에 적었다.

## 별도 조건이 적용되는 자료

- 원작 이미지·음원·표·맵, 이를 바탕으로 만든 추가 그림·아이콘, 게임 캡처·영상: [자원 이용 조건](ASSET-LICENSES.md).
- `Modules/ThirdParty`, Gradle wrapper, Emscripten 실행 지원 코드, `Compatibility`의 외부 코드·패치, 복사한 스킬: [외부 고지](THIRD_PARTY_NOTICES.md). 파일별 기존 고지를 유지한다.
- 원작 분석 결과·해독 데이터·기존 저작물에서 가져온 표현은 루트 MIT에 포함하지 않는다.
- 개발자의 변·인터뷰·사용자 원문·일반 문서 본문·인용 자료는 코드 라이선스로 일괄 재허락하지 않는다.

[게임 소개](README.md) · [문서 목차](docs/public/README.md)
