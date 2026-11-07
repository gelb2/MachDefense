# 설치와 실행

<!-- language-nav:start -->
[한국어](build-and-install.md) | [English](../en/public/build-and-install.md)
<!-- language-nav:end -->

## 웹·PWA

[MachDefense 실행·설치](https://gelb2.github.io/MachDefense/)

주소를 열고 자원 다운로드를 마치면 **PLAY IN BROWSER**로 바로 플레이한다. PWA로 설치하면 별도 앱 창에서 열고 오프라인으로도 실행할 수 있다.

- iPhone·iPad: Safari에서 주소를 열고 공유 → 홈 화면에 추가를 선택한다.
- Android: Chrome에서 **INSTALL GAME**을 누른다. **HOW TO INSTALL**이 표시되면 브라우저 메뉴의 앱 설치 또는 홈 화면에 추가를 선택한다.
- Mac Chrome: **INSTALL GAME**이 표시되면 설치 창을 연다. **HOW TO INSTALL**이면 Chrome 메뉴 → 전송, 저장, 공유 → 페이지를 앱으로 설치를 선택한다. 이미 설치했다면 기존 앱 아이콘으로 연다.
- Mac Safari: 파일 → Dock에 추가를 선택한다. **HOW TO INSTALL**은 이 경로를 안내한다.

설치 없이 실행하려면 **PLAY IN BROWSER**를 누른다. 설치 창을 취소해도 브라우저 플레이는 가능하다.

설치한 앱을 열어 다운로드가 완료됐는지 확인한다. 게임 진행은 기기에 저장한다. 다른 기기로 옮길 때는 슬롯 화면의 **GET CODE**로 코드를 받고 **ENTER CODE**에 입력한다. 전체 5슬롯을 교체한다. 사이트 데이터를 지우기 전에는 코드를 보관한다.

[PWA 안내와 공유 복원 코드](quick-play.md)

## Tauri Mac

[Mac 앱 다운로드](https://github.com/gelb2/MachDefense/releases/download/v0.6/MachDefense-Desktop-v0.6-arm64.zip). 압축을 풀고 MachDefense Desktop.app을 응용 프로그램 폴더로 옮겨 실행한다. macOS 14 이상 Apple Silicon용이다. 첫 실행부터 게임 자원이 포함되어 있다.

앱은 ad-hoc 서명이며 Apple 공증을 받지 않았다. 실행이 차단되면 시스템 설정 → 개인정보 보호 및 보안 → 확인 없이 열기를 선택한다. 키보드로 텍스트를 입력하고 마우스·트랙패드로 게임을 조작한다. 전장 이동은 눌러 끌기이며 휠·트랙패드 스크롤은 생산 목록에서 동작한다. 전장 키보드 이동은 아직 없다.

슬롯 화면의 GET CODE·ENTER CODE로 다른 플랫폼과 진행을 옮긴다. [빌드·저장·업데이트 설명](../../Desktop/Tauri/README.md).

## iPhone·iPad

Apple Silicon Mac에 Xcode와 iOS Simulator, Python 3, CMake 3.20 이상을 설치한다. Homebrew를 쓴다면 `brew install cmake`로 CMake를 설치할 수 있다. 첫 빌드에서 저장소에 포함된 OpenAL Soft 소스를 CMake로 컴파일한다.

`Apple/MachDefense.xcworkspace`를 열고 일반 앱은 `MachDefense`, 치트 앱은 `MachDefenseCheat` 스킴을 선택한다. iPhone 또는 iPad 시뮬레이터를 선택한 뒤 Run을 누른다. 실기기에서는 자신의 개발 팀으로 서명한다. 최소 배포 대상은 iOS·iPadOS 15.0이다.

워크스페이스와 프로젝트 파일은 저장소에 들어 있다. XcodeGen은 프로젝트를 재생성할 때만 필요하다.

## Android

[Releases](https://github.com/gelb2/MachDefense/releases/latest)의 일반·치트 APK를 설치하거나 Android Studio에서 `Android/`를 연다. 빌드 환경은 SDK36·NDK26.3.11579264·CMake3.22.1·JDK17이다. `normalDebug` 또는 `cheatDebug`와 연결한 기기를 선택한다. Android8(API26) 이상 arm64를 대상으로 한다.

## 소스에서 빌드

```sh
git clone https://github.com/gelb2/MachDefense.git
cd MachDefense
```

게임 코드와 이미지·음원·맵·글꼴 아틀라스·외부 라이브러리 소스를 함께 받는다. 일반 Git 파일이므로 Git LFS 다운로드 단계는 없다. 원본 IPA나 `prepare_resources.py` 실행 없이 받은 파일로 빌드한다. 웹판을 직접 빌드하려면 [웹 개발 문서](../../Web/README.md)를 따른다.

[게임 소개](../../README.md) · [문서 목차](README.md)
