# 정식 앱 피드백 수정 — 2026-11-04

사용자는 새 정식 앱에서 포탑 설치 안내·생산 불가 딤 표시·생산 완료음이 빠졌고, 촬영 후 앱에 복귀하면 Resume와 소리가 동작하지 않는다고 보고했다. 스테이지 사이의 전투 일러스트도 확인하도록 지시했다.

## 원인과 변경

| 항목 | 원작 근거 / 새 코드 원인 | 수정 |
| --- | --- | --- |
| 포탑 설치 안내 | DlgBuildMach::buildMach: 0x2f270 → TapToTurretPos, GuiGame::setMessage:BG:AutoHide: 0x287bc | 원본 MessageBox.png, 영어 문구 “Tap to build.”, 상단 위치·등장 진동·닫힘 축소를 GameHUD에 연결. SFX_OnMessage.wav도 재생 |
| 생산 불가 딤 | DlgBuildMach::tick 0x2ef70, refreshButtons 0x2e090 | BuildMach_Cover.png로 셀 부족·대기열 7개·유닛별 한도·전투 상태를 표시. 강화 열람은 허용 |
| 생산 완료음 | BuildItem::tick 0x30644의 sfxID 17. GlobalInfo::loadSFX 0x1a080의 +0x44, CFString 0x6aedc = BuildMachComplete.wav | World 생산 완료 콜백에서 원본 음원 1회 재생. 사전 로드한 OpenAL 버퍼·소스 사용 |
| 촬영 후 복귀 먹통 | MDGameView에서 오디오 중단 플래그가 화면 갱신과 터치까지 막음. 종료 알림 없는 복귀에서 플래그가 남을 수 있음 | 앱 활성화 시 AVAudioSession 재활성화. 게임 입력·화면 갱신을 오디오 중단 상태와 분리. 전투는 멈춤 메뉴 유지 |
| 스테이지 로딩 그림 | DlgSummary 0x35e64 → GobWorld::setNextStage 0x16b74 → GuiMapLoading 0x2acb4/0x2ae94. CFString 0x6badc = StageClear01.png | Next/Retry에서 원본 전투 그림을 페이드 인, 화면을 덮은 동안 미션 교체, 페이드 아웃. 새 구현의 문 전환 호출을 해당 경로에서 교체 |

분석 대상은 위메이드 1.0.3 ARMv7 실행 파일 SHA-256 `6ec47ea094abaa31bab10058fe47bb25d46c6038b2e88a7da2629899f84af112`이다. 원본과 분석 덤프는 저장소 밖 로컬 보관소에 있다. 원작의 로딩 코드·이미지를 확인했으며 이번 검사에서 포팅판을 직접 조작해 해당 장면을 재생한 것은 아니다.

## 자동 검사

- 코어 5개 검사 통과, 6.19초. `.local/five-fixes-core-tests.log`.
- 시뮬레이터 3개 검사 통과, 5.843초. 포탑 안내·딤 GPU 캡처, 생산 220조건·운반봇 60배치·특수공격 54조건, 실제 MDGameView의 중단 종료 알림 없는 복귀 3회를 검사했다. `.local/five-fixes-render.xcresult`.
- 생산 완료 사운드는 완료 전 호출하지 않고, 각 생산 완료 때 한 번 호출하며, 운반봇이 포탑으로 바뀔 때 중복 호출하지 않는지 검사한다.
- 로딩 복구 후 80개 미션의 실패·재도전·승리·다음 미션·저장 재읽기 검사 통과. `.local/loading-80-missions.xcresult`. 첫 재도전의 StageClear01 화면을 GPU 캡처로 확인했다.
- 기존 검사에서 생산·저장 결과가 맞는 것만으로 개별 안내·사운드·전환까지 확인할 수 없었다. 위 항목을 별도 회귀 조건으로 추가했다.

## 실기기

iPad Air 13-inch M2/iPadOS 27.0.1에서 회귀 검사 4개가 모두 통과했다. 전체 실행 구간은 49.928초다. 생산·배치·특수공격 4.258초, 80미션 26.710초, 실제 MDGameView 중단·복귀 1.273초, 포탑 안내·딤 GPU 검사 0.699초였다. `.local/ipad-five-fixes-r2.xcresult`와 `.local/device-sweep-final/five-fixes-summary.json`에 결과를 보존했다. 시험 저장은 임시 디렉터리를 사용했다.

iPhone·iPad 정식 앱, iPad 치트 앱에 수정본을 설치했다. iPhone 정식 앱의 기존 저장은 설치 전 백업했다. iPhone 치트 앱은 무료 개발 서명의 3개 앱 한도로 설치하지 못했다. 기존 포팅판의 저장을 백업했고, 삭제 여부에 대한 사용자 답변을 기다린다. 정식 앱·HeavyMach 앱을 임의로 삭제하지 않는다.

최종 기기 빌드는 `.local/five-fixes-apps/`다. 일반·치트 앱의 서명을 검사했고 XCTest 플러그인이 포함되지 않은 것을 확인했다. 설치·실행 로그는 `.local/device-sweep-final/*five-fixes*`에 있다. iPhone 정식 화면과 iPad 정식 타이틀을 확인했으며 iPad에서는 포탑 선택→안내 표시→위치 지정→홈 화면→앱 복귀→Resume 터치를 직접 수행했다. 멈춤 메뉴가 닫히고 셀 증가·적 이동·생산이 재개됐다. 검사 앱 두 개를 종료하고 홈 화면을 확인했다. 촬영 앱까지 포함한 iPhone 사용자 경로 재확인은 남았다.


## 사용자 테스트 중 상태

22:14 정식 빌드와 22:15 치트 빌드에 위 다섯 수정이 포함됐다. 이후 원작의 안내창 글자가 배경과 함께 축소되도록 세부 보정을 추가했고 코어 2개 검사 1.55초 통과, 별도 GPU 검사도 수행했다. 22:22 치트 빌드를 iPad에 설치했다. 최종 보정 빌드 경로는 `.local/final-ui-apps/`다. iPhone은 사용자가 테스트 중이므로 추가 재설치하지 않는다. 테스트가 끝난 후 글자 축소 보정을 정식 앱에도 반영한다. 코드·문서는 `feat/source-reimplementation`의 미커밋 변경이며 push·merge는 하지 않았다.

## 후속 UI·효과음·지형 수정

스테이지 선택 화면은 원작이 없는 `CompleteMark.png`를 요청한다. 새 코드는 이를 `SelStage_CompleteMark.png`로 임의 대체했고, 원작 부모 좌표까지 더해 화면 오른쪽에 잘린 도장을 그렸다. 포팅판에서도 해당 도장이 없는 것을 확인하고 대체 연결을 제거했다. 해금·선택 표시는 유지한다. iPad 치트 앱의 미션 선택 화면에서 직접 확인했다.

로딩 일러스트의 `Title_TitleIn.wav` 호출을 복구했다. 원본 `GobWorld::setNextStage`(0x16b74)는 Next/Retry 전환 시작 때 이 음원을 한 번 재생한다. 패널 전환음, 생산 대기열 등록음 `BuildMach.wav`, 기지·유닛·특수기술 강화와 구매·장착 효과음도 원작 호출에 맞췄다. 특수기술 선택은 원작처럼 손가락을 누르는 시점에 처리한다.

하단 세 버튼은 누르는 동안 원본 밝은 타일로 바뀐다. 20프레임 누름 유지·취소·선택 검사와 실제 GPU 픽셀 밝기 검사를 통과했다. 원작 QobButton은 누름 해제 지연이 0이다. 짧게 탭하면 밝은 프레임이 화면에 잡히지 않을 수 있으나 이번 검사에서 사용자 관측의 글로우 누락을 재현하지 못했다. 버튼 유지 시간을 임의로 늘리지 않았다.

같은 편 지상 유닛 간 겹침 해소 호출 누락도 복구했다. [지형·이동 대조](terrain-motion-audit-20261104.md). 미션 4 정체 해결 여부와 미션 3 구멍 위 포탑 설치의 포팅판 실행 대조는 남아 있다.

### 후속 검사와 설치

- 코어 전체 33개 중 32개가 첫 실행에서 통과했다. Campaign 검사는 로딩 그림 복구 후 기존 160틱 대기가 짧아 실패했다. 시험 코드가 실제 로딩 종료까지 기다리도록 고친 뒤 해당 검사도 통과했다(22.37초).
- 시뮬레이터 버튼·로딩 검사 1개 통과(1.095초).
- iPad Air 13-inch M2: 버튼·로딩, 메뉴, 복귀, 생산 딤·설치 안내 4개 검사 통과(3.285초). 실제 출력음 녹음 비교는 수행하지 않았다.
- 23:05 치트 빌드 서명과 테스트 플러그인 미포함을 확인한 후 iPad에 설치·실행했다. iPhone 정식 앱은 사용자 테스트 중이므로 종료·재설치하지 않았다.
- 로컬 근거: `.local/av-feedback-core-tests.log`, `av-feedback-campaign-tests.log`, `av-feedback-ipad.xcresult`, `av-feedback-ipad-images`, `av-feedback-ipad-install.log`.

전체 음향 대조는 계속 진행한다. 피격음 HitBody01~03, 플라스마 지속 피해음, 원본 공간 음향의 좌표 전달은 추가 확인이 필요하다. 버튼 검사 통과를 전체 그래픽·음향 대조 완료로 기록하지 않는다.
