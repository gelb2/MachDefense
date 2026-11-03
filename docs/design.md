# 설계

원본 비교 실행기는 Compatibility/ios의 iOS 프로젝트로 구현 중이다. 소스 재구현 코어와 제품 플랫폼 프로젝트는 아직 미구현이다.

| 경로 | 책임 |
|---|---|
| Sources/Core | MachDefense 게임 로직·상태·수치·원본 데이터 해석 |
| Modules | 검증 후 분리할 공용 기능 |
| Resources | 패키지에서 분리해 준비한 에셋·데이터와 명세 |
| Compatibility | 원본 비교 실행 도구. IPA·실행 바이너리는 외부에 보관 |
| Apple | 하나의 workspace와 하위 iOS/iPadOS·native macOS 프로젝트. Catalyst 제외 |
| Android | Gradle·NDK·Kotlin OS 연결과 플랫폼 생명주기 |
| Web | 웹 런타임·저장·입력·GPU 복구 |
| Desktop/Tauri | 웹 빌드를 소비하는 데스크톱 실행기 |
| Distribution | 배포 포장·PWA 등 배포 구성 |
| Tests | 기능·동등성·플랫폼·도구 검사 |
| docs/evidence | 공개 가능한 검사 요약·해시. 원본 및 개인정보 제외 |

원본 비교판의 구성과 빌드는 [Compatibility](../Compatibility/README.md)를 따른다. ARMv7 명령은 touchHLE 인터프리터, 그래픽은 기존 OpenGL ES 호환 경로, 음향은 기존 OpenAL 호환 경로를 사용한다. 이미 정상인 플랫폼이 생긴 후에는 새 플랫폼 때문에 그 소스·공용 동작을 임의 변경하지 않는다.
