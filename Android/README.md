# MachDefense Android

Android 일반 앱과 치트 앱은 공통 C++ Core·Audio와 같은 게임 자원을 사용한다. Kotlin은 Surface, 입력 전달, 생명주기, 오디오 포커스, 음악 재생, 프로필 이름 입력을 연결한다. 게임 버튼·메뉴·HUD·전투·강화·긴급지원은 C++에서 그린다.

## 빌드

Android Studio에서 이 `Android/` 폴더를 연다. Android SDK 36, NDK 26.3.11579264, CMake 3.22.1, JDK 17을 사용한다. `local.properties`의 `sdk.dir`은 자신의 SDK 경로로 설정한다. 이 파일은 Git에서 제외한다.

```sh
./gradlew :app:assembleNormalDebug :app:assembleCheatDebug
```

| 종류 | 패키지 | APK |
|---|---|---|
| 일반 | `com.gelb2.machdefense.restored` | `app/build/outputs/apk/normal/debug/app-normal-debug.apk` |
| 치트 | `com.gelb2.machdefense.validation` | `app/build/outputs/apk/cheat/debug/app-cheat-debug.apk` |

Android 8(API 26) 이상 arm64 대상으로 빌드한다. 실제 확인한 기기는 Galaxy SM-S731N과 API 36 arm64 에뮬레이터다. 두 앱은 개발용 서명이며 별도 저장을 쓴다. 공개 배포용 서명은 아직 준비하지 않았다.

`prepare_assets.py`가 저장소의 자원 1,387개를 APK에 넣고 해시 목록을 만든다. 게임 실행에 외부 원본 패키지나 다운로드 서버가 필요하지 않다. APK·Gradle 캐시·개인 SDK 경로는 Git에 넣지 않는다.

## 폴더 책임

- `app/src/main/cpp`: EGL/OpenGL ES 1.1 렌더러, APK 자원 로더, 원자적 저장, JNI 연결.
- `app/src/main/java`: Android Surface·터치·생명주기·MediaPlayer와 텍스트 입력.
- `app/src/cheat`: 치트 전용 검사 명령 수신기. 일반 앱에는 포함하지 않는다.
- `cmake`: 기존 Core/Audio를 링크하고 OpenAL Soft의 OpenSL ES 출력을 선택한다.
- `tools`: 자원 패키징, ADB 검사, 기존 저장 해석·기기 복사 도구.
- `protected-baseline.json`: 개발 시작 시 Apple·공통 소스·자원·기존 검사 2,978개 파일의 SHA-256.

게임 규칙과 게임 UI는 공통 C++ 모듈에서 실행한다. Android 최초 이식은 Apple·Sources·Modules·Resources와 루트 CMake를 보존했다. Android 전용 오디오 재개 보정은 빌드 디렉터리의 사본에만 적용한다.

## 검사와 저장 이전

[구현·검사 기록](../docs/evidence/android-native-20261106.md)에 기기, 실행 범위, 측정값과 남은 과제를 정리했다. `python3 Android/tools/verify_preserved.py`로 보존 파일의 해시를 검사한다. `tools/device.py`는 치트 APK의 상태를 읽는다. `tools/app_smoke.py`는 첫 미션에 진입한 치트 앱에서 실제 ADB 터치와 기존 치트 명령을 함께 사용한다. `tools/campaign_render_sweep.py`는 80개 미션 화면을 차례로 로딩한다.

저장 구조는 기존 `SaveData` 그대로다. 앱의 `files/MachDefense.sav`를 읽고 쓴다. `copy_save_to_device.py`는 대상 앱을 멈추고 기존 저장부터 백업한 뒤 지정한 저장을 복사하고 바이트 일치를 확인한다. 사용자 저장은 `.local/` 등 Git 제외 경로에만 보관한다. 프로필 선택 화면의 `GET CODE`는 다섯 슬롯을 코드로 내보낸다. `ENTER CODE`에서 입력한 코드를 검사하고 확인하면 다섯 슬롯을 복원한다.

버튼·확인 창·코드 형식은 공통 C++ 메뉴와 `Modules/SaveTransfer`가 맡는다. Android는 지정된 사각형에 텍스트 입력란을 띄우고 키보드와 클립보드를 연결한다. 복원할 때 기존 저장을 백업하며, 중간 종료된 복원은 다음 시작 시 복구한다.

`tools/save_transfer_smoke.py SERIAL CODE_FILE SAVE_FILE`은 치트 앱에서 입력·오류·취소·확인·복원·복사·Activity 재생성·재실행을 검사한다. 코드는 파일에서 실제 입력란에 넣고 C++ 메뉴 버튼을 ADB 터치로 누른다. 대상 치트 앱의 저장을 먼저 `.local/`에 백업하고 제공된 저장과 모든 바이트를 대조한다. 일반 앱의 저장은 이 검사에서 건드리지 않는다.

저장 복사 전에는 저장소 루트에서 검사 도구를 빌드한다. 이 도구는 기존 SaveData의 크기·체크섬·인벤토리 검사를 그대로 호출한다.

```sh
c++ -std=c++20 -O2 -I Sources Android/tools/save_inspect.cpp Sources/Core/Session/SaveData.cpp -o .local/save-inspect
```
