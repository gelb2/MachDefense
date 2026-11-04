# 설계

원본 비교 실행기는 Compatibility/ios에 있다. 소스 재개발판은 C++20 코어·공용 자원 번들·Objective-C++ Apple 실행기로 작성 중이다. Apple/MachDefense.xcworkspace에 일반·치트 앱과 Modules 프로젝트를 연결했다. GameSession에 메뉴·전투·강화·보상·저장을 연결했다. 현재 위메이드 1.0.3 포팅판과 기능·표시를 대조하고 실기기 검증을 진행한다. 전체 완료 범위는 [현재 상태](current-status.md), 작업 순서는 [소스 재개발 계획](source-reimplementation-plan.md)을 따른다.

| 경로 | 책임 |
|---|---|
| Sources/Core | MachDefense 게임 로직·상태·수치·원본 데이터 해석 |
| Modules | C++ 코어 라이브러리·공용 에셋 번들·외부 라이브러리 |
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

신규 Apple 앱은 iPhone/iPad 유니버설 구성을 목표로 한다. 공용 에셋 원본·메타데이터를 플랫폼들이 소비하며, [자원 사전조사](universal-assets-plan.md)에 따라 픽셀 해상도와 게임 좌표를 분리한다.
