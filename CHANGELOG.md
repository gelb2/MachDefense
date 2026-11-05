# 변경 이력

## v0.1 — iOS·iPadOS·Android 첫 릴리스

2026-11-06. [GitHub Release](https://github.com/gelb2/MachDefense/releases/tag/v0.1) · [소스 태그](https://github.com/gelb2/MachDefense/tree/v0.1) · [설치](docs/public/build-and-install.md)

### 게임

- 위메이드판의 80개 미션을 기준으로 생산·교전·기지와 장비 강화·특수기술·승패·보상·프로필 저장을 연결했다.
- iPhone·iPad 화면에 맞춰 전장과 HUD 표시 영역을 확장했다.
- 원작의 자금 버튼에 30·45·60초 간격의 크레딧·셀·혼합 긴급지원을 추가했다. 광고와 인앱결제는 사용하지 않는다.
- 일반 앱과 조건을 변경할 수 있는 치트 앱을 분리했다.

### 플랫폼과 수정

- iOS·iPadOS는 하나의 Apple 워크스페이스와 공통 C++ 모듈을 사용한다.
- Android는 같은 게임 코드와 자원을 사용하고 Kotlin·JNI·EGL·오디오·파일 저장 연결을 추가했다. 기존 Apple·공통 코드·자원 2,978개 파일은 변경하지 않았다.
- 기지 발사 시 오디오 컨텍스트 처리, 복귀 후 효과음 소실, 생산·보상 아이콘 방향, 가격·선택 효과·메뉴 동작을 대조·수정했다.
- Android 출력부의 중단·재개 오류와 선택적 이미지 로딩 차이를 수정했다.

### 검사

- 실제 iPhone·iPad 일반/치트 설치와 Apple 자동 검사, 80미션 진행·저장, 강화·무장 검사를 수행했다.
- Galaxy·에뮬레이터에서 각각 C++ 35개와 앱 실행 13개가 통과했다. 에뮬레이터의 80미션 화면 로딩, 양쪽의 EGL·Activity 복구도 확인했다.
- 사용자가 Android 정식 앱에서 첫 미션 시작·클리어에 문제가 없음을 확인했다.
- 아이폰의 진행·장비·강화·자금·특수기술을 Galaxy 일반 앱으로 복사해 Stage 33 진입과 재실행 보존을 확인했다.

### 설치 파일과 남은 일

Android 일반·치트 APK는 개발용 서명본이며 앱 내부 버전은 `0.1-android-dev`다. Apple 앱 내부 버전은 `0.1`이며 Xcode로 빌드·서명한다. 소스 릴리스 태그는 세 플랫폼을 묶은 `v0.1`이다.

길찾기 정체, 생산 HUD 전체 목록·순서, 오른쪽 HUD·타이틀 그림, 강화 상한 확장과 사용자용 복원 코드는 후속 과제다. Android 무선 연결·장시간 발열·추가 OS 기기 검사가 남아 있다. 웹·PWA·Tauri Mac은 아직 구현하지 않았다.

[기기별 검사](docs/public/validation.md) · [Android 구현 기록](docs/evidence/android-native-20261106.md) · [트러블슈팅](docs/troubleshooting.md) · [TODO](docs/usability-todo.md)
