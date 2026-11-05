# 설치와 실행

Xcode나 Android Studio에서 프로젝트를 열고 실행할 수 있다. 터미널 빌드가 익숙하지 않다면 IDE에서 기기와 실행 대상을 선택한다.

## 소스 받기

```sh
git clone https://github.com/gelb2/MachDefense.git
cd MachDefense
git switch --detach v0.1
```

이 저장소는 현재 비공개다. 접근 권한이 있는 계정으로 clone한다. 게임에 필요한 코드·이미지·음원·맵을 저장소에 포함했다. Git LFS와 원본 게임 패키지는 이 버전의 빌드에 필요하지 않다.

## iPhone·iPad

Apple Silicon Mac과 Xcode를 사용한다. 프로젝트의 최소 배포 대상은 iOS·iPadOS 15.0이다.

1. `Apple/MachDefense.xcworkspace`를 연다.
2. 일반 앱은 `MachDefense`, 치트 앱은 `MachDefenseCheat` 스킴을 고른다.
3. 시뮬레이터 또는 연결된 iPhone·iPad를 고른다.
4. 실기기는 Signing & Capabilities에서 자신의 개발 팀을 설정하고 실행한다.

일반·치트 앱의 저장은 각각 보존된다. 개발자 신뢰나 개발자 모드를 요구하면 기기 설정에서 처리한다. 워크스페이스 재생성 등 상세 빌드 절차는 [개발 환경](../development.md)에 있다.

## Android

Android 8(API 26) 이상 arm64를 대상으로 한다. 실제 검사한 기기는 Galaxy SM-S731N과 API 36 arm64 에뮬레이터다.

**[v0.1 다운로드](https://github.com/gelb2/MachDefense/releases/tag/v0.1)**에서 일반 APK 또는 치트 APK를 받는다. 기기에서 파일을 열고, 사용한 브라우저나 파일 앱의 설치 허용을 켠 뒤 설치한다. 두 앱은 개발용 서명본이다.

직접 빌드하려면 Android Studio에서 저장소의 `Android/` 폴더를 연다. SDK 36·NDK 26.3.11579264·CMake 3.22.1·JDK 17을 사용한다. `normalDebug` 또는 `cheatDebug` 변형과 기기를 선택해 실행한다. [Gradle 명령과 파일 경로](../../Android/README.md).

## 저장

프로필 슬롯에 미션 진행·크레딧·누적 점수·장비·강화·특수기술 잔량을 저장한다. Apple과 Android는 같은 저장 형식을 쓴다. 아이폰 저장을 Galaxy 일반 앱으로 옮겨 재실행까지 확인했다. 현재 이전은 개발 도구로 수행하며, 플레이어가 코드 한 줄로 옮기는 기능은 후속 과제다.

[게임 조작](gameplay.md) · [검사 결과](validation.md) · [README](../../README.md)
