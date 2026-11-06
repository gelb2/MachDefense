# GitHub 새 클론의 iOS·Android 빌드 검사

2026-11-06. 공개 main `45d91b9b7911aa7b38215237d74dd979b67b9760`, 태그 v0.3을 검사했다.

## 받은 파일

GitHub HTTPS 주소를 인증 정보 없이 새 디렉터리에 clone했다. 기존 제작 폴더의 소스·자원·빌드 결과·사용자 저장·원본 IPA를 가져오지 않았다. 설치된 Xcode·SDK·NDK·JDK·CMake·Python은 빌드 도구로 사용했다.

자원 준비 목록의 이미지 1,109개·음원 80개·데이터 98개가 모두 존재하며 SHA-256이 같다. 변환된 표 94개와 글꼴 아틀라스 등 생성 자원 6개도 Git에 들어 있다. 게임에서 사용하는 자원은 총 1,387개다. 미다운로드 LFS 포인터와 외부 경로 심볼릭 링크는 없다. [자원 대조 결과](clean-clone-resources-20261106.json).

원본 IPA·원본 실행 바이너리는 빌드 입력이 아니다. `prepare_resources.py`와 `prepare_font.swift`를 실행하지 않고 저장소의 준비된 파일을 사용했다. v0.3 푸시의 전체 Git 이력 검사도 원본 패키지·바이너리와 비밀값이 없음을 확인했다.

## iOS

추적된 `Apple/MachDefense.xcworkspace`를 그대로 사용하고 새 DerivedData에서 일반·치트 앱을 빌드했다. XcodeGen으로 프로젝트를 재생성하지 않았다. OpenAL Soft는 저장소에 포함된 소스를 pre-build 단계에서 새로 컴파일했다.

환경은 Xcode 27.1, iOS Simulator SDK 27.1, CMake 4.4.4, Python 3.14.6이다. 일반·치트 시뮬레이터 빌드가 성공했다. 두 앱의 `MachDefenseAssets.bundle`에 자원 1,387개가 전부 있으며 clone 파일과 SHA-256이 같다.

새 iPhone Air 시뮬레이터에서 빈 프로필을 만들고 Stage 1에 들어갔다. 전장·기지·HUD를 표시하고 MachineGun 두 대의 생산·출격·이동과 Save & Exit를 확인했다. 실제 iPhone·iPad용 iphoneos Release 일반앱도 서명 없이 컴파일했으며 자원 1,387개가 모두 일치했다.

치트 앱에서는 TEST 메뉴·빈 프로필·1,000Cr 추가·Stage 1 진입을 직접 확인했다. 자동검사 2개에서 기지 강화 6종·유닛 4종·특수공격 3종의 버튼 구매·저장과 WAV 준비·재사용·중단 복귀를 확인했다. 검사 앱과 전용 시뮬레이터를 종료했다.

## Android

새 `GRADLE_USER_HOME`에서 Gradle 배포본과 의존성을 내려받았다. 기존 Gradle 빌드 캐시·APK·가공 자원을 복사하지 않았다. 일반·치트 APK 빌드는 2분 46초에 완료됐으며 83개 작업을 실행했다. SDK 36·NDK 26.3.11579264·CMake 3.22.1·JDK 17을 사용했다. 일반·치트 APK의 실제 자원 1,387개와 resource-manifest의 SHA-256이 clone 파일과 모두 같았다.

새 API 36 arm64 에뮬레이터의 정식 앱에서 빈 슬롯에 FRESH 프로필을 만들고 Stage 1에 들어갔다. MachineGun 생산·이동·Save & Exit와 42,496바이트 저장을 확인했다. 치트 앱은 유닛 생산 1회, 음원 버퍼 77개, 재생 9회, 음향 샘플 진행을 확인했다. game error와 audio error는 없었다. 두 앱을 종료해 홈 화면을 확인한 뒤 전용 에뮬레이터도 종료했다.

`FileStore.hpp`의 signed/unsigned 비교 경고가 두 flavor에서 남았다. 에뮬레이터의 ranchu 오디오 HAL에는 `pcm_writei` I/O 경고가 있었으며, 음향 버퍼·샘플 검사를 실제 스피커 음질 검사로 간주하지 않았다. 빌드 실패나 누락 파일은 없었다.

## 설치 문서 보완

iOS 설치 안내에 CMake와 Python 3가 빠져 있었다. OpenAL의 첫 빌드에 필요한 도구로 추가했다. XcodeGen은 프로젝트 재생성 때만 필요하다는 점, 자원과 글꼴이 이미 포함돼 있어 원본 변환 도구를 실행할 필요가 없다는 점도 명시했다.

Apple·Android 빌드 로그 5개에 기존 제작 폴더나 IPA를 참조한 경로가 없었다. 제품 소스·게임 자원·빌드 설정은 변경하지 않았다.

로컬 검사 로그와 화면은 `Documents/Codex/2026-11-06/machdefense-clean-clone/`에 보존했다. 사용자 앱·저장을 사용하는 검사는 수행하지 않았다.

[설치 안내](../public/build-and-install.md) · [검사 결과](../public/validation.md)
