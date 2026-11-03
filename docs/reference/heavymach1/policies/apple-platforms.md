# Apple 플랫폼과 공용 모듈

2026-10-26 작업 브랜치: `feat/apple-workspace-modules`. main의 v0.9 태그는 이동하지 않는다.

`HeavyMach.xcworkspace` 하나에서 세 프로젝트를 연다. iPhone과 iPad는 같은 iOS 앱 target을 사용한다. Mac은 AppKit 앱이며 Catalyst를 사용하지 않는다.

| 프로젝트 | 역할 |
|---|---|
| Modules/HeavyMachModules.xcodeproj | C++ 코어, Apple 공통 실행 코드, 자원 번들 |
| iOS/HeavyMachRestored.xcodeproj | iPhone·iPad 일반/치트 앱과 XCTest |
| macOS/HeavyMachMac.xcodeproj | native Mac 일반/치트 앱과 XCTest |

공용 C++ 코드는 `Sources/Core` 한 벌이다. 일반용 HeavyMachCore와 치트용 HeavyMachValidationCore를 각각 정적 프레임워크로 만든다. HM_VALIDATION이 앱과 코어에서 일치해야 한다. AppleRuntime 프레임워크도 일반/치트를 구분한다. iOS와 macOS SDK별 산출물은 XcodeGen의 플랫폼 접미사로 구별한다.

이미지·음원·MAP/MCH는 HeavyMachAssets.bundle 한 구성에서 가져온다. 앱 안에는 그 플랫폼용 자원 번들이 들어간다. HMGameResourceBundle이 해당 번들을 찾는다. IPA·호환 실행기를 읽거나 외부 폴더에서 그림을 가져오는 경로를 추가하지 않는다. 기존 자원의 SHA-256은 `docs/build-resources.json`으로 검사한다.

공통 Objective-C++ 구현은 기존 `Sources/iOS`에 있다. HMPlatform은 UIView/NSView, UIImage/NSImage 등 OS 차이를 분리한다. HMGLPlatform은 iOS의 ES1.1 컨텍스트와 Mac의 desktop OpenGL compatibility 컨텍스트를 만든다. 그 뒤 텍스처·블렌드·스프라이트·궤적 제출 코드는 공유한다. AppKit의 NSWindow·검증 창은 `Sources/Mac`에 있다.

Mac OpenGL은 Apple이 deprecated한 API다. 이번 구현은 기존 fixed-function 렌더와 비교할 수 있도록 같은 렌더 연산을 유지한다. Metal 전환은 별도 과제로 다루며 이번 모듈화에서 렌더 수식과 에셋을 교체하지 않는다.

## 실행

```sh
xcodegen generate --spec Modules/project.yml
xcodegen generate --spec iOS/project.yml
xcodegen generate --spec macOS/project.yml
open HeavyMach.xcworkspace
```

- iPhone/iPad: HeavyMachRestored 또는 HeavyMachValidation scheme과 기기/Simulator를 선택한다.
- Mac: HeavyMachMac 또는 HeavyMachMacValidation scheme을 선택한다. 창 크기 변경과 macOS 전체 화면을 구현했다. 2026-10-27 macOS 자체 전체 화면 캡처에서 게임이 화면 끝까지 표시됨을 확인했다. 흰 여백은 CUA 창 캡처에 나타났다. [현재 검사 결과](public/apple-platform-progress-20261026.md).
- 키 배치·재지정·터치 병용은 [키보드 정책](keyboard-input-policy.md)을 따른다.
- Mac 앱과 iOS 시뮬레이터의 개발·검사 환경은 Apple Silicon arm64만 지원한다. OpenAL도 arm64로 빌드한다. 2026-10-29 사용자 결정에 따라 Intel Mac과 그 위에서 실행하는 x86_64 iOS 시뮬레이터는 지원 대상에서 제외한다.

```sh
xcodebuild -workspace HeavyMach.xcworkspace -scheme HeavyMachMacValidation \
  -destination 'platform=macOS,arch=arm64' -derivedDataPath build/mac \
  test
```

Mac 일반·치트·테스트 target은 Apple Development 자동 서명을 사용한다. Xcode의 Signing & Capabilities에서 자신의 개발 팀을 선택한다. 이 저장소의 기본 팀은 기존 iOS 설정과 같다. `CODE_SIGNING_ALLOWED=NO`는 컴파일 확인용이며 실행·메모리 검사와 구분한다. ad-hoc 서명(`CODE_SIGN_IDENTITY=-`)에서는 macOS27.0.1의 시스템 등록 서비스가 팀 ID 부재로 연결을 거절했고, 재시도 뒤 약20KB 참조 순환이 관측됐다. 정상 서명본의 후속 결과는 [검사 보고](public/apple-platform-progress-20261026.md)를 따른다.

프레임워크의 중복 소스와 앱별 누락은 `tools/audit_ios_sources.py`가 세 프로젝트의 의존 관계를 따라 검사한다. `.yml`을 바꾸면 관련 프로젝트를 다시 생성하고 generated project도 함께 커밋한다.

## Android 연결

Apple framework 바이너리를 Android에 가져가지 않는다. `Sources/Core`의 같은 CMake target을 NDK로 빌드하고 동일 자원 목록을 Android assets로 묶는다. JNI·Android 생명주기·렌더·음향 연결은 [기존 계획](android-port-plan-20261026.md)의 후속 구현이다. 현재 Apple 모듈화가 Android APK 완료를 뜻하지 않는다.

## 조사 근거

[Apple의 여러 프로젝트와 의존 관계](https://developer.apple.com/documentation/xcode/managing-multiple-projects-and-their-dependencies), [Swift package의 리소스 지원](https://developer.apple.com/documentation/xcode/bundling-resources-with-a-swift-package), [XcodeGen 프로젝트 명세](https://github.com/yonaskolb/XcodeGen/blob/master/Docs/ProjectSpec.md), [Apple Silicon 앱 이식](https://developer.apple.com/documentation/Apple-Silicon/porting-your-macos-apps-to-apple-silicon), [Android CMake 연결](https://developer.android.com/studio/projects/gradle-external-native-builds).

게임도 공용 코어와 자원을 framework/SPM으로 모듈화할 수 있다. 이번에는 사용자가 지정한 workspace 아래 여러 project 구성과 기존 Objective-C++ 연결을 직접 사용할 수 있는 정적 framework를 채택했다.

## 2026-10-27 실제 기기 확인

iPhone Air·iPad Air 13형 M2의 전체 suite는 각각84/85 통과다. 키보드20무기·터치 병용 API 검사는 통과했으며 남은 실패는 표시시간 비교다. Device Hub로 보낸 OS 키 입력에서 이동·점프·조준·사격·무기 전환을 확인했다. 양방향 가로 터치 사격과 홈 복귀 일시정지도 확인했다. 실제 Bluetooth/USB 키보드 검사는 남았다. [상세·측정 범위](public/apple-platform-progress-20261026.md), [증거](evidence/physical-apple-20261027.json).

## 무선 설치와 디버깅

2026-10-27 iPhone 일반 앱 무선 재설치·실행과 iPad 일반·치트 앱 무선 설치를 확인했다. 두 기기에서 LLDB 중단점·변수 조회·재개를 수행했다. iPad 선택 검사8/8과 Time Profiler21.716초 기록이 성공했다. [명령·스킬·검증 범위](wireless-device-workflow.md)를 따른다. 기존 표시시간 후속 과제는 유지한다.
