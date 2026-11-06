# 변경 이력

## v0.3 — 기기 간 복원 코드와 Quick Play

2026-11-06. [플레이·PWA 설치](https://gelb2.github.io/MachDefense/) · [Quick Play·공유 코드](docs/public/quick-play.md)

- iPhone·iPad·Android·웹/PWA가 같은 MD1 코드로 전체 프로필 5개를 옮긴다.
- 슬롯 화면의 GET CODE·COPY·ENTER CODE·CHECK·YES/NO는 공용 게임 버튼으로 처리한다. 플랫폼은 텍스트 입력·키보드·클립보드를 연결한다.
- 네이티브 복원에 백업·중단 복구·원자적 교체·완료 기록을 연결했다. 복원 이후의 새 진행도 유지한다.
- 80개 미션을 모두 완료한 저장을 복원할 수 있다.
- README에 PWA 안내와 Stage 42·유닛 12종·20,329Cr 공유 코드를 배치했다.

### 확인한 동작

- 공용 C++ 38개 검사, iPhone·iPad 시뮬레이터, Android 에뮬레이터, Chrome·WebKit 회귀 검사 통과.
- 실제 iPhone·iPad·Galaxy에서 코드 생성·복원과 저장을 검사했다. iPad·Galaxy 치트 앱 복원 후 iPhone 원본 42,496바이트가 일치했다.
- Galaxy 정식 앱 저장을 보존했고, Galaxy PWA 오프라인 전투·재실행 후에도 저장이 유지됐다.
- 나는 iPhone 코드로 iPad PWA를 복원해 미션을 클리어한 뒤, PWA의 새 코드로 iPhone에 재복원하고 스테이지 진입을 확인했다.
- 파일 교체·디스크 동기화·롤백·정리 실패를 주입해 기존 저장과 재실행 복구를 검사했다.

### 설치 파일

Android 일반·치트 APK를 제공한다. Galaxy에서 검사한 개발용 서명본이며 앱 내부 버전은 `0.1-android-dev`다. Apple 앱 내부 버전은 `0.1`이며 Xcode로 빌드·서명한다. 소스 릴리스는 `v0.3`, 운영 PWA 빌드는 `d679d7b1061097b5`다. APK SHA-256은 Release의 `SHA256SUMS.txt`에 있다.

전투·경제·이동·무기 계산·음향·게임 자원은 이번 복원 연결에서 변경하지 않았다. iPhone 설치형 PWA 직접 검사, 장시간 성능과 Tauri Mac은 남았다. [검사 결과](docs/evidence/save-transfer-all-platforms-20261106.md) · [전체 TODO](docs/usability-todo.md)

## v0.2 — 웹·PWA 첫 릴리스

2026-11-06. [플레이·설치](https://gelb2.github.io/MachDefense/) · [설치 안내](docs/public/build-and-install.md)

- 같은 C++ 게임과 자원을 WebAssembly로 빌드하고 WebGL2·브라우저 입력·음향을 연결했다.
- 설치형 PWA, 자원 다운로드 표시, 오프라인 실행과 업데이트를 제공한다.
- 진행은 IndexedDB에 저장한다. 슬롯 화면의 GET CODE와 ENTER CODE로 전체 5슬롯을 다른 웹판에 복원한다.
- README에 웹·PWA 실행 링크와 설치 방법을 추가하고 플랫폼·언어 칩을 갱신했다.
- Apple·Android 실행기와 기존 전투 계산은 유지했다. 공용 파일 4개에 선택적 복원 서비스와 메뉴 연결을 추가했다.

네이티브 앱 설치본은 v0.1을 유지한다. Tauri Mac, Apple·Android 복원 메뉴, 실제 모바일 PWA의 추가 검사는 후속 과제다. [웹 릴리스 설명](docs/public/web-pwa-release-draft.md) · [배포 검사](docs/evidence/pwa-release-20261106.md).


## 2026-11-06 문서 갱신

README·개발자의 변·기능·개발 과정·기술 구조·설치·검사·트러블슈팅·크레딧을 갱신했다. 긴급지원은 작성자가 기획한 추가 기능으로 구분하고, 자동 실행과 치트 앱의 검사 절차를 설명했다. 원본 출처와 mesabloo의 선행 복원 시도도 연결했다.

웹·PWA는 개발 브랜치에서 검사 중이다. [다음 릴리스 원고](docs/public/web-pwa-release-draft.md)에 구현·검사 범위를 기록했다. 게임 릴리스 태그는 v0.1을 유지한다.

## v0.1 — iOS·iPadOS·Android 첫 릴리스

2026-11-06. [GitHub Release](https://github.com/gelb2/MachDefense/releases/tag/v0.1) · [소스 태그](https://github.com/gelb2/MachDefense/tree/v0.1) · [설치](docs/public/build-and-install.md)

### 게임

- 원작의 80개 미션을 기준으로 생산·교전·기지와 장비 강화·특수기술·승패·보상·프로필 저장을 연결했다.
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
