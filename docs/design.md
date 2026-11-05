# 설계

원본 비교 실행기는 Compatibility/ios에 있다. Web·Desktop·Distribution 및 native macOS 경로는 예정 책임이다. Android는 2026-11-06 일반·치트 APK를 구현하고 Galaxy와 에뮬레이터에서 검사했다. [Android 책임과 실행 근거](evidence/android-native-20261106.md)를 따른다. 소스 재개발판은 C++20 코어·공용 자원 번들·Objective-C++ Apple 실행기로 작성 중이다. Apple/MachDefense.xcworkspace에 일반·치트 앱과 Modules 프로젝트를 연결했다. GameSession에 메뉴·전투·강화·보상·저장을 연결했다. 현재 위메이드 1.0.3 포팅판과 기능·표시를 대조하고 실기기 검증을 진행한다. 전체 완료 범위는 [현재 상태](current-status.md), 작업 순서는 [소스 재개발 계획](source-reimplementation-plan.md)을 따른다.

| 경로 | 책임 |
|---|---|
| Sources/Core | MachDefense 게임 로직·상태·수치·원본 데이터 해석 |
| Modules | C++ 코어 라이브러리·공용 에셋 번들·외부 라이브러리 |
| Resources | 패키지에서 분리해 준비한 에셋·데이터와 명세 |
| Compatibility | 원본 비교 실행 도구. IPA·실행 바이너리는 외부에 보관 |
| Apple | 현재 iPhone/iPad 유니버설 앱과 Modules 프로젝트를 연결한 workspace. native macOS 앱 미구현, Catalyst 제외 |
| Android | Gradle·NDK·Kotlin OS 연결과 플랫폼 생명주기 |
| Web | 웹 런타임·저장·입력·GPU 복구 |
| Desktop/Tauri | 웹 빌드를 소비하는 데스크톱 실행기 |
| Distribution | 배포 포장·PWA 등 배포 구성 |
| Tests | 기능·동등성·플랫폼·도구 검사 |
| docs/evidence | 공개 가능한 검사 요약·해시. 원본 및 개인정보 제외 |

원본 비교판의 구성과 빌드는 [Compatibility](../Compatibility/README.md)를 따른다. ARMv7 명령은 touchHLE 인터프리터, 그래픽은 기존 OpenGL ES 호환 경로, 음향은 기존 OpenAL 호환 경로를 사용한다. 이미 정상인 플랫폼이 생긴 후에는 새 플랫폼 때문에 그 소스·공용 동작을 임의 변경하지 않는다.

신규 Apple 앱은 iPhone/iPad 유니버설 구성을 목표로 한다. 공용 에셋 원본·메타데이터를 플랫폼들이 소비하며, [자원 사전조사](universal-assets-plan.md)에 따라 픽셀 해상도와 게임 좌표를 분리한다.

## 2026-11-05 공용 모듈·음향·후속 플랫폼 계획

[공용 모듈 진단과 플랫폼 확장 계획](module-audit-20261105.md)에 현재 파일별 책임, HeavyMach 분기·병합 이력, OpenAL 구현 차이, 예정 폴더·빌드 대상과 분리 전후 검사를 정리했다. 목표 플랫폼은 iPhone/iPad·Android 네이티브·웹/PWA·Tauri Mac이다. 원본 음향·텍스처 대조 수정은 `f623eba`, Core·Audio 분리는 `193ed2f`에서 완료했다. 분리 전후 비교와 기기별 결과는 [모듈화 실행 기록](evidence/module-work-20261105.md)을 따른다.

## 긴급지원 모듈 — 2026-11-05

`Simulation/EmergencySupport`는 전투 시간과 현재 셀 용량·연구값을 받아 30/45/60초 반복 지원을 계산한다. `Session/GameSession`은 수령액을 기존 슬롯 크레딧과 World의 셀에 반영한다. `Presentation/SupportDialog`는 원작 CommonUI·SysMenu_Btn·Common_Btn_CLOSE·SpriteFont·Button으로 창을 그리고 입력을 처리한다. Apple 전용 UI나 새 저장 파일은 없다. 지원 상태는 전투가 끝나면 사라지며 수령한 크레딧은 기존 저장 지점에서 보존한다. [산식](emergency-resupply-economy.md) · [검사](evidence/emergency-resupply-20261105.md).

## Android 착수 경계 — 2026-11-06

기존 Core·Audio의 NDK 컴파일과 OpenAL Soft 링크를 통과했다. Android 실행기·렌더러·오디오 출력·저장·생명주기는 `Android/`에 추가한다. Apple·공용 소스·자원·루트 빌드 파일을 변경하지 않는 조건으로 진행한다. [HeavyMach 비교와 실제 검사](android-readiness-20261106.md).
