# 기술 구성

| 영역 | 구현 |
|---|---|
| 게임 규칙·저장·게임 UI | C++20, `Sources/Core/` |
| 공용 효과음 | C++·OpenAL Soft, `Sources/Audio/`와 `Modules/ThirdParty/` |
| iOS·iPadOS 실행기 | Objective-C++, Xcode 워크스페이스, OpenGL ES 1 fixed-function |
| Android 실행기 | Kotlin·JNI·C++·EGL/OpenGL ES 1.1, Gradle·CMake·NDK |
| 음악 | Apple 음악 재생 연결, Android MediaPlayer |
| 자원 | 공용 이미지·음원·맵·표, `Resources/` |
| 검사·개발 도구 | C++ 검사, Apple XCTest, Python·ADB |

Core에는 피해·이동·생산·경제·강화·미션 진행과 메뉴·HUD가 들어 있다. 플랫폼 실행기는 같은 Frame을 받아 그리며 터치를 게임 좌표로 변환한다. 이름 입력 같은 OS 기능은 각 플랫폼에서 연결한다.

Android 추가 때 기존 Apple·Core·Audio·자원·검사 파일 2,978개를 변경하지 않았다. OpenAL Soft의 Android 출력 재개 오류는 Android 빌드용 사본에서만 보정했다. [원인과 검사](../evidence/android-native-20261106.md).

Apple 앱은 `Apple/MachDefense.xcworkspace`, Android 앱은 `Android/`에서 연다. 플랫폼 구현 파일은 각 폴더에 둔다. [모듈 구조](../../Modules/README.md) · [상세 설계](../design.md).
