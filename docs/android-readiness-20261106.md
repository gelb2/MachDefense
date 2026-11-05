# Android 개발 준비 점검 — 2026-11-06

현재 구조로 Android 개발을 시작할 수 있다. Apple 소스와 공용 코어를 바꾸지 않고 Android NDK로 Core·Audio를 컴파일하고, OpenAL Soft와 함께 네이티브 공유 라이브러리로 링크했다. Android 앱 실행기와 출력 어댑터는 아직 만들어야 한다.

## HeavyMach 1 당시와 비교

| 범위 | HeavyMach 1 Android 착수 당시 | 현재 MachDefense |
|---|---|---|
| 전투·이동·피해·성장·저장 | 공용 C++ 코어 사용 | 공용 C++ Core 사용 |
| 메뉴·HUD·게임 진행 연결 | Apple의 `HMGameView.mm`, `HMMenus.mm`, `HMGameHUD.mm`에 있던 연결을 Android C++ 실행기에 구현 | `GameSession`, 메뉴·HUD·버튼·강화·결과창·긴급지원이 이미 공용 C++에 있음 |
| 화면·입력·음향·파일·생명주기 | Android JNI·EGL·오디오·저장 구현 | Android 어댑터 구현 필요 |

HeavyMach의 당시 기준은 `d46e122a`, 첫 Android 구현은 `9d536ce0`, 업그레이드 창 일시정지 수정은 `cda437de`다. 이 범위의 Git 차이에서 기존 `Sources`, `Modules`, Apple 프로젝트, 루트 CMake, 자원 경로의 변경은 없었다. 당시 구현 문서도 보호 대상 2,166개 파일의 동일성을 기록했다. 공용 코어의 C++ 22개와 검증 파일 1개를 NDK로 빌드했고 Android 실행기를 별도로 연결했다.

MachDefense는 메뉴와 게임 진행 연결까지 공용화돼 있다. HeavyMach에서 Android 실행기에 다시 구현했던 메뉴 상태와 강화창의 일시정지 처리를 그대로 공용 코드로 사용할 수 있다. 파일 수는 모듈의 품질을 나타내는 비율이 아니므로 두 프로젝트의 공용화율을 파일 수로 계산하지 않았다.

HeavyMach 확인 자료: `docs/android-implementation-20261030.md`, `docs/android-port-plan-20261026.md`, `docs/evidence/android-boundaries-20261030.json`, 위 세 커밋의 Android CMake와 변경 목록. HeavyMach 저장소는 읽기만 했다.

## 실제 검사 결과

기준은 `fix/hud-title-presentation`의 `410fdb869063e962718ffe87ea84f970a268187d`다. 기존 Xcode scheme 변경은 감사 시작 상태 그대로 보존했다.

| 검사 | 결과 |
|---|---|
| 기존 Core 41개 C++ 파일·Audio의 Android 컴파일 | 통과 |
| 모든 Core·Audio 오브젝트를 포함한 공유 라이브러리 링크 | 통과 |
| 기존 OpenAL Soft의 Android OpenSL ES 백엔드 빌드·링크 | 통과 |
| ELF 아키텍처 | AArch64 |
| ELF LOAD 정렬 | 0x4000 |
| 보호 대상 2,978개 파일의 검사 전후 SHA-256 | 변경 0개 |
| 모듈 경계 검사 | 통과. Core에 플랫폼 SDK 헤더 없음 |
| Android APK·에뮬레이터·Galaxy 실행 | 미실행 |

NDK `26.3.11579264`, SDK CMake `3.22.1`, `arm64-v8a`, API 26으로 검사했다. API 26은 이번 컴파일 조건이며 최종 지원 OS 결정은 아니다. ELF 정렬 검사는 APK의 패키징이나 실제 기기 실행 검사를 대신하지 않는다.

최초 구성은 PATH에 Ninja가 없어 실패했다. SDK에 설치된 CMake·Ninja를 지정해 통과했다. 제품 소스는 수정하지 않았다. 링크용 작은 실행 진입점은 `.local/`에만 만들었다. 실제 Android HostServices 구현이나 게임 실행 결과는 포함하지 않는다.

[검사 결과와 산출물 해시](evidence/android-readiness-20261106.json). 전체 로그·파일별 해시·링크 시험 소스는 `.local/android-readiness-20261106/`에 보관한다.

## 변경 범위

Android 개발에서 `Apple/`, `Sources/`, `Modules/`, `Resources/`, 기존 `Tests/`와 루트 `CMakeLists.txt`를 보존한다. Android 빌드 명세·실행기·어댑터·검사는 `Android/`에 둔다. Android 전용 CMake가 기존 루트와 OpenAL Soft를 읽어 빌드한다. 게임 규칙과 자원을 Android용으로 복제·수정하지 않는다.

| Android에서 만들 부분 | 기존 코드와 연결할 내용 |
|---|---|
| Gradle·정식/치트 앱·JNI 실행기 | `createGame`, `tick`, `frame`, `touch`, `setActive`, `resize` |
| EGL·OpenGL ES 렌더러 | Frame의 스프라이트·메시·클립·블렌드·텍스처 필터·그리기 순서 |
| 자원 로더 | 기존 자원 바이트·이미지 크기. 기존 POT 패딩·알파·UV 처리 재현 |
| 효과음·음악 | 공용 OpenALEffects와 기존 WAV 연결. Android WAV 디코더·MP3 스트리밍 재생 연결 |
| 저장 | 기존 슬롯 바이트와 저장 시점. 앱 내부 파일에 원자적 기록 |
| Activity·Surface·오디오 focus | 비활성 일시정지, 복귀, GL 자원 재생성, 오디오 장치 재개 |
| 검증 연결 | 기존 치트 검사 실행과 화면·입력·저장·OS 중단 검사 |

메뉴·HUD·강화·긴급지원 버튼은 공용 Frame과 터치 판정을 사용한다. Kotlin/XML로 게임 버튼을 다시 만들지 않는다. Apple의 WAV 디코더와 AudioQueue 음악 출력은 Apple 전용이므로 Android 출력 어댑터를 따로 구현한다. 디코딩·버퍼 생성은 발사 때마다 반복하지 않는다.

## 저장 계약 점검 결과

현행 `HostServices::writeSave`는 기록 성공 여부를 즉시 반환한다. Android 앱 내부 파일에도 이 계약을 그대로 구현할 수 있다. 파일 기록이 끝난 뒤 성공을 반환하면 된다. 발사·피해·틱마다 저장을 추가하지 않는다.

이전 모듈 진단에서 비동기 웹 저장을 이유로 Apple까지 저장 API를 바꾸는 계획을 적었다. **Android 개발의 선행 조건에서는 제외한다.** 웹의 비동기 저장 설계는 웹 착수 때 별도로 결정한다. Android 추가를 위해 기존 Apple 저장이나 공용 인터페이스를 바꿀 필요는 이번 검사에서 발견하지 못했다.

Android의 [AtomicFile](https://developer.android.com/reference/android/util/AtomicFile)은 새 파일 기록과 동기화·교체를 지원한다. 동시 호출은 어댑터가 직렬화한다. [GLSurfaceView.Renderer](https://developer.android.com/reference/android/opengl/GLSurfaceView.Renderer)의 컨텍스트 소멸 시에는 GPU 자원을 다시 만든다. [오디오 focus](https://developer.android.com/media/optimize/audio-focus)는 앱 활성 상태에 맞춰 요청·반납한다.

## 개발과 검증 순서

1. 착수 시 develop의 기준 커밋을 고정하고 별도 Android 브랜치를 만든다. 현재 문서 작업과 타이틀 브랜치를 임의로 병합하지 않는다.
2. 보호 대상의 파일 목록·해시와 Apple 빌드 입력을 저장한다. Android의 실행기·렌더러·음향·저장·생명주기를 연결한다.
3. 정식·치트 APK를 컴파일한다. 기존 코어 검사와 같은 데이터·조건을 사용해 생산·전투·강화·지원·승패·진행·저장을 검사한다.
4. 에뮬레이터에서 메뉴 버튼·터치·좌표·음향·중단 복귀·Surface 재생성을 검사한다. 저장을 종료·재실행 후 비교한다.
5. Galaxy 유선·무선 설치와 직접 조작, 효과음 집중 재생·프레임·메모리·잠금 복귀를 검사한다. 사용자 저장은 백업하고 시험 저장과 분리한다.
6. 보호 파일과 Apple 빌드 입력을 다시 비교한다. 예상 밖 변경이 있으면 원인을 해결한 후 병합한다.

현재 남아 있는 지형 이동 정체·HUD 표시·생산 목록 제한은 같은 코어를 사용하는 Android에도 이어질 수 있다. 해당 수정은 기존 TODO에서 관리한다. Android 이식 작업에 섞어 게임 규칙을 바꾸지 않는다.

이번 점검에서는 Android 개발 브랜치 생성·앱 구현·기기 설치·병합을 하지 않았다. [현재 상태](current-status.md)와 [TODO](usability-todo.md)에 착수 조건을 연결했다.
