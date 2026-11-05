# 문서와 코드 대조 — 2026-11-06

요구·정책은 목표 동작을 정하고, 코드·검사·기기 기록은 현재 구현을 설명한다. 문서를 코드에 맞춘다는 이유로 미해결 버그의 요구를 삭제하지 않는다. 아래는 현행 소비 경로이며 과거 분석 문서의 파일 목록은 당시 경로다.

| 기능 | 요구·정책 | 구현 | 검사·실행 근거 | 남은 일 |
|---|---|---|---|---|
| Core·Audio 분리 | [설계](design.md), [모듈](../Modules/README.md) | [Core 목록](../Modules/core-sources.cmake), [모듈 빌드](../Modules/project.yml) | [분리 전후](evidence/module-work-20261105.md), `tools/check_modules.py` | 웹/PWA·Tauri 실행기 미구현 |
| Android 연결 | [설계](design.md), [빌드](../Android/README.md) | [실행기](../Android/app/src/main/cpp/Runtime.cpp), [Activity](../Android/app/src/main/java/com/gelb2/machdefense/GameActivity.kt) | [기기·저장·프레임 검사](evidence/android-native-20261106.md) | 무선·장시간 발열·추가 OS 기기 |
| 월드·HUD 표시 | [좌표](coordinate-policy.md), [렌더](rendering-policy.md) | [WorldPresentation](../Sources/Core/Presentation/WorldPresentation.cpp), [GameHUD](../Sources/Core/Presentation/GameHUD.cpp), [FrameLayout](../Sources/Core/Contracts/FrameLayout.hpp) | [ViewportSweep](../Tests/ViewportSweep.hpp), [화면 확장](evidence/viewport-20261105.md) | HUD 위 월드 누락, 7개 생산 제한 원본 대조. 그림 변경 보류 |
| 타이틀 | [렌더](rendering-policy.md) | [Menu](../Sources/Core/Presentation/Menu.cpp), [자원 출처](../Resources/Generated/TitleOutpaint.json) | [MenuTests](../Tests/MenuTests.cpp), [수정 이력](evidence/hud-title-20261105.md) | 후속 사용자 확인에서 흰 선 잔존 |
| 피해·이동·표적 | [계산 정책](game-calculation-policy.md) | [WorldCombat](../Sources/Core/Simulation/WorldCombat.cpp), [WorldMovement](../Sources/Core/Simulation/WorldMovement.cpp), [WorldTargeting](../Sources/Core/Simulation/WorldTargeting.cpp), [WorldTerrain](../Sources/Core/Simulation/WorldTerrain.cpp) | [핵폭탄](evidence/nuclear-damage-20261104.md), [지형](evidence/terrain-motion-audit-20261104.md) | 이동 정체·미션3 설치 대조, 화면 밖 반격 보류 |
| 강화·경제 | [계산 정책](game-calculation-policy.md) | [Economy](../Sources/Core/Simulation/Economy.cpp), [WorldEconomy](../Sources/Core/Simulation/WorldEconomy.cpp), [GameSession](../Sources/Core/Session/GameSession.cpp) | [강화 효과](../Tests/BaseUpgradeEffectSweep.hpp), [증가율](evidence/base-upgrade-rates-20261105.md) | 실제 포팅판 생산량 대조, 신규10레벨 확장 미구현 |
| 반복 긴급지원 | [기획](game-design.md), [산식](emergency-resupply-economy.md) | [EmergencySupport](../Sources/Core/Simulation/EmergencySupport.cpp), [SupportDialog](../Sources/Core/Presentation/SupportDialog.cpp), [GameSession](../Sources/Core/Session/GameSession.cpp) | [Sweep](../Tests/EmergencySupportSweep.hpp), [기기·저장](evidence/emergency-resupply-20261105.md) | 지원 유무 난이도 비교 |
| 효과음·음악·복귀 | [수명주기](runtime-policy.md) | [OpenALEffects](../Sources/Audio/OpenALEffects.cpp), [MDAudio](../Sources/Platform/Apple/MDAudio.mm), [Apple 실행기](../Sources/Platform/Apple/MDApplication.mm) | [세션 경고](evidence/audio-session-20261105.md), [장치 복귀](evidence/effects-resume-20261105.md) | 원작 녹음·실제 사진 촬영 등 중단 대조 |
| 저장·진행 | [수명주기](runtime-policy.md), [화면 전환](stage-transition-contract.md) | [SaveData](../Sources/Core/Session/SaveData.cpp), [Settlement](../Sources/Core/Session/Settlement.cpp), [GameSession](../Sources/Core/Session/GameSession.cpp) | [SaveFailureTests](../Tests/SaveFailureTests.cpp), [전체 구매·진행](evidence/full-upgrade-weapon-device-20261104.md) | 최대 단계 초과 저장 입력 처리, 한 줄 복원 코드 미구현 |

현행 Core는41개 C++ 구현 파일이며 CMake 목록과 일치한다. OpenAL Soft 원본317파일의 해시는 보존됐고 Core에는 플랫폼 SDK 헤더가 없다. Apple은 현재 arm64 iOS15 이상 설정의 iPhone/iPad 유니버설 타깃이다. 일반·치트의 게임 코어는 같고 저장 경로와 검사 진입은 분리한다.

새 기능을 구현할 때 요구→이 표의 소비 코드→검사→실행 근거를 갱신한다. 함수가 존재하거나 과거 검사가 통과했다는 사실만으로 새 조건의 동작을 완료 처리하지 않는다. [현재 상태](current-status.md)와 [TODO](usability-todo.md)에서 미확인 범위를 확인한다.
