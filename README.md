# MachDefense

[![Release v0.1](docs/public/media/badges/version.svg)](https://github.com/gelb2/MachDefense/releases/tag/v0.1)
[![iOS, iPadOS, Android](docs/public/media/badges/platforms.svg)](docs/public/build-and-install.md)
[![arm64](docs/public/media/badges/arch.svg)](docs/public/architecture.md)
[![C++20, Objective-C++, Kotlin](docs/public/media/badges/code.svg)](docs/public/architecture.md)

MachDefense를 iPhone·iPad·Android에서 실행하도록 다시 만들었다. 메크와 포탑을 생산하고, 기지와 장비를 강화하며, 특수기술로 적 기지를 공격하는 세로 화면 전략 게임이다. 위메이드판의 80개 미션과 원작 에셋을 사용한다.

**[v0.1 — iOS·iPadOS·Android 첫 릴리스](https://github.com/gelb2/MachDefense/releases/tag/v0.1)** · [버전별 변경](CHANGELOG.md) · [설치](docs/public/build-and-install.md) · [조작과 게임 기능](docs/public/gameplay.md)

## MachDefense를 다시 만든 이유

나는 캐나다와 미국 앱스토어 계정으로 MachDefense를 구매해 플레이했다. HeavyMach 1을 복원한 뒤, MachDefense도 같은 절차로 다시 만들기로 했다. 먼저 원본을 실행해 동작을 확인하고, 새 소스와 에셋을 연결한 앱을 만들어 둘을 비교했다.

나는 무명이나, 예술가들은 불멸이다. 나는 그저 불씨를 다시 지폈을 뿐이다.

[개발자의 변](docs/RESTORATION.md)

## 원작의 전투와 성장

오른쪽 생산 목록에서 유닛을 고르면 셀을 사용해 생산한다. 메크는 전장으로 나가 적과 교전한다. 포탑은 생산 항목을 고른 뒤 설치할 위치를 누른다. 전장을 스크롤하며 아군과 적의 위치를 확인하고, 기지·장비 강화와 특수기술 보충에 크레딧을 쓴다.

기지 기관포·방어력·생산 시간·크레딧 및 셀 연구·셀 용량을 강화할 수 있다. 특수기술은 폭격·미사일·핵폭탄 등 원작의 공격 규칙과 보유 횟수를 사용한다. 미션을 마치면 결과와 보상을 저장하고 다음 미션으로 이동한다.

![Android에서 실행한 Stage 33](docs/evidence/images/android-20261106/normal-iphone-stage33.png)

![보유 장비와 강화 단계](docs/evidence/images/android-20261106/normal-iphone-upgrades.png)

## 복원판에서 추가한 기능

iPhone과 iPad 화면에 맞춰 전장과 HUD 표시 영역을 확장했다. Android는 같은 C++ 게임 코드와 자원을 사용한다. 게임 버튼·메뉴·HUD도 공통 코드에서 그린다.

기존 자금 버튼에는 **EMERGENCY RESUPPLY**를 연결했다. 전투 중 크레딧은 30초, 셀은 45초, 혼합 지원은 60초마다 준비된다. 받을 지원이 있으면 버튼이 빛나며, 메뉴에서 CLAIM을 눌러 수령한다. 일시정지 중에는 지급 시간도 멈춘다. 광고와 인앱결제는 넣지 않았다.

일반 앱과 치트 검증 앱을 각각 설치할 수 있다. 치트 앱에서는 미션·유닛·자금·강화 조건을 바꿔 게임을 검사한다. 두 앱의 저장 공간은 분리돼 있다.

## 설치와 실행

| 플랫폼 | 실행 방법 | 현재 확인 |
|---|---|---|
| iOS·iPadOS | Xcode에서 `Apple/MachDefense.xcworkspace` 실행 | 실제 iPhone·iPad 일반/치트 앱, 시뮬레이터 |
| Android | Android Studio에서 `Android/` 실행 또는 Releases의 APK 설치 | Galaxy SM-S731N, API 36 arm64 에뮬레이터 |
| 웹·PWA·Tauri Mac | 후속 개발 | 아직 구현하지 않음 |

[설치 문서](docs/public/build-and-install.md) · [공용 모듈과 기술 구성](docs/public/architecture.md) · [기기별 검사](docs/public/validation.md)

## 복원 중 고친 문제

기지 기관총 발사 때 나타난 프레임 저하, 백그라운드 복귀 후 효과음 소실, 생산·보상 아이콘의 방향, 가격 표시와 메뉴 동작을 원본과 대조해 수정했다. Android에서는 Surface·오디오 재개와 선택적 이미지 처리 차이를 수정했다.

[트러블슈팅](docs/troubleshooting.md) · [Android 개발과 아이폰 저장 이전](docs/evidence/android-native-20261106.md)

## 남은 과제

- 원형 구조물과 밀집한 포탑 주변의 유닛 이동 정체.
- 생산 HUD의 7개 제한과 전체 유닛 접근·순서 편집.
- iPhone의 셀 표시 위 월드 누락과 타이틀 경계선. HUD 에셋은 현재 모습을 유지한다.
- 기지·포탑·특수기술의 강화 상한과 단계별 외형·효과 연구.
- 사용자용 복원 코드, 웹·PWA·Tauri Mac.
- Android 장시간 발열·메모리, 추가 기기·실제 오디오 중단·무선 연결 검사.

[전체 TODO](docs/usability-todo.md) · [문서 목차](docs/README.md) · [개발 진입](AGENTS.md) · [현재 인계](HANDOVER-CURRENT.md)
