# 설치와 실행

v0.1과 main의 앱은 iOS·iPadOS·Android용이다. 아래 웹·PWA와 복원 코드 설명은 `feat/web-pwa` 개발 브랜치의 구현을 다룬다.

Xcode나 Android Studio에서 프로젝트를 열고 실행할 수 있다. IDE에서 기기와 실행 대상을 선택하면 된다.

## 소스 받기

```sh
git clone https://github.com/gelb2/MachDefense.git
cd MachDefense
```

현재 저장소는 비공개이므로 접근 권한이 있는 계정으로 받는다. v0.1은 Apple·Android 릴리스다. 웹·PWA는 `feat/web-pwa` 브랜치에 있다. 게임 코드와 이미지·음원·맵을 함께 받는다.

## iPhone·iPad

Apple Silicon Mac의 Xcode에서 `Apple/MachDefense.xcworkspace`를 연다. 일반 앱은 `MachDefense`, 치트 앱은 `MachDefenseCheat` 스킴을 선택한다. 시뮬레이터나 연결한 기기를 고른다. 실기기에서는 자신의 개발 팀으로 서명한다. 최소 배포 대상은 iOS·iPadOS 15.0이다.

## Android

[Releases](https://github.com/gelb2/MachDefense/releases/tag/v0.1)의 APK를 설치하거나 Android Studio에서 `Android/`를 연다. 빌드 환경은 SDK36·NDK26.3.11579264·CMake3.22.1·JDK17이다. `normalDebug` 또는 `cheatDebug`와 연결한 기기를 선택한다. Android8(API26) 이상 arm64를 대상으로 한다.

## 웹·PWA

`feat/web-pwa`에서 Emscripten6.0.13 SDK 환경을 불러온 뒤 실행한다.

```sh
emcmake cmake -S Web -B build/web-normal -DMD_WEB_CHEAT=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build/web-normal -j6
python3 Web/tools/package.py build/web-normal/dist
python3 -m http.server 8941 --bind 127.0.0.1 --directory build/web-normal/dist
```

Mac 브라우저에서 `http://127.0.0.1:8941/`를 연다. 자원 다운로드가 끝나면 PLAY IN BROWSER로 시작한다. 다른 기기에서 PWA로 설치하려면 그 기기에서 접속할 수 있는 HTTPS 배포 주소가 필요하다. 공개 게임 배포는 아직 하지 않았다. [웹 빌드와 검사](https://github.com/gelb2/MachDefense/blob/feat/web-pwa/Web/README.md)

[게임 소개](../../README.md) · [문서 목차](README.md)
