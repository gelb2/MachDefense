# 설치와 실행

## 웹·PWA

[MachDefense 실행·설치](https://gelb2.github.io/MachDefense/)

주소를 열고 자원 다운로드를 마치면 **PLAY IN BROWSER**로 바로 플레이한다. PWA로 설치하면 별도 앱 창에서 열고 오프라인으로도 실행할 수 있다.

- iPhone·iPad: Safari에서 주소를 열고 공유 → 홈 화면에 추가를 선택한다.
- Android: Chrome에서 **INSTALL GAME**을 누른다. 버튼 대신 설치 안내가 나오면 브라우저 메뉴의 앱 설치 또는 홈 화면에 추가를 선택한다.
- Mac: Chrome에서 **INSTALL GAME**을 누르거나 Safari의 파일 → Dock에 추가를 선택한다.

설치한 앱을 열어 다운로드가 완료됐는지 확인한다. 게임 진행은 기기에 저장한다. 다른 기기로 옮길 때는 슬롯 화면의 **GET CODE**로 코드를 받고 **ENTER CODE**에 입력한다. 전체 5슬롯을 교체한다. 사이트 데이터를 지우기 전에는 코드를 보관한다.

## iPhone·iPad

Apple Silicon Mac의 Xcode에서 `Apple/MachDefense.xcworkspace`를 연다. 일반 앱은 `MachDefense`, 치트 앱은 `MachDefenseCheat` 스킴을 선택한다. 시뮬레이터나 연결한 기기를 고른다. 실기기에서는 자신의 개발 팀으로 서명한다. 최소 배포 대상은 iOS·iPadOS 15.0이다.

## Android

[Releases](https://github.com/gelb2/MachDefense/releases/tag/v0.1)의 APK를 설치하거나 Android Studio에서 `Android/`를 연다. 빌드 환경은 SDK36·NDK26.3.11579264·CMake3.22.1·JDK17이다. `normalDebug` 또는 `cheatDebug`와 연결한 기기를 선택한다. Android8(API26) 이상 arm64를 대상으로 한다.

## 소스에서 빌드

```sh
git clone https://github.com/gelb2/MachDefense.git
cd MachDefense
```

게임 코드와 이미지·음원·맵을 함께 받는다. 저장소 접근 권한이 필요하다. 웹판을 직접 빌드하려면 [웹 개발 문서](../../Web/README.md)를 따른다.

[게임 소개](../../README.md) · [문서 목차](README.md)
