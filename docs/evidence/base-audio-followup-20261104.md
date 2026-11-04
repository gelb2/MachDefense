# 기지 강화·오디오 복귀 후속 검사

2026-11-04 23:40. 실제 iPad Air M2에서 치트 앱에 포함한 XCTest 3개가 통과했다. 임시 프로필을 사용했고 기존 사용자 저장에는 쓰지 않았다.

| 검사 | 결과 |
| --- | --- |
| Base Upgrade | phone/pad 108레벨 조건·456구매·40크레딧 수집·40셀 수집·20용량 상한·30외형 전환, 1.745초 |
| 오디오 복귀 | 종료 알림 없는 중단/복귀 3회·복귀 뒤 지연 suspension 알림·미디어 재설정 후 음악 복원과 재생 시간 증가, 2.386초 |
| 핵폭탄 | 경계 70조건·실제 공습 10회·일반 체력 적의 파괴/생존, 0.144초 |

전체 테스트 시간 4.275초, 설치·도구 준비 포함 21.115초. `.local/base-nuclear-audio-ipad.xcresult`와 같은 이름의 로그에 보존했다.

## 기지 강화

CrResearch는 크레딧 드롭 수량을 1~3배로 늘린다. CellResearch는 시간당 셀 생성량을 1~2.2배로 늘린다. 광물 수집량·수집 반경을 늘리는 효과는 아니다. Cannon·Defense·BuildTime·CellStorage의 실제 피해·회복·생산 완료·상한도 검사했다.

원본 DlgUpgradeBase::setUpgrade(0x344cc)는 구매 직후 체력·무장을 갱신하고, GobWorld::setStage(0x1627c)는 다음 미션에서 차체·레이더를 다시 구성한다. 새 World::refreshBaseUpgrades는 차체·레이더도 즉시 바꿨다. 무장만 교체하도록 수정했다. 수정 전 외형 검사 실패, 수정 후 통과를 확인했다.

## 오디오 복귀

복귀 후 AVAudioSession 활성화에 성공해도 늦게 도착한 `AVAudioSessionInterruptionWasSuspendedKey` 알림이 `_audioInterrupted`를 다시 켰다. 게임 입력은 동작했으나 모든 음향은 비활성 상태로 남았다. 이 순서로 실패를 재현했다.

현재 화면이 활성 상태인 경우 과거 suspension 알림에서 세션을 다시 활성화하고 성공 시 중단 상태를 해제한다. 백그라운드로 갈 때는 OpenAL을 중단한 뒤 세션을 비활성화한다. 미디어 서비스 재설정에서는 음원을 다시 준비하고 기존 음악·볼륨을 복원한다.

재생 호출 횟수에 더해 실제 OpenAL 재생 시간 증가를 확인했다. iPhone의 실제 사진 촬영 후 출력음 녹음 비교는 아직 남아 있다. 사용자 플레이 중인 iPhone 정식 앱은 이번 후속 수정본으로 교체하지 않았다.

[원본 기능 대조 기록](source-ui-audio-lifecycle-20261104.md) · [핵폭탄 검사](nuclear-damage-20261104.md) · [절차 점검](restoration-process-audit-20261104.md)

## 피격·지속 피해음과 성능 — 23:48

World::damage의 원본 피격음 SFX_HitBody01~03 호출 누락을 복구했다. 사망 시 폭발·드롭·잔해 수명 계산 뒤 원본 순서로 난수를 한 번 사용한다. 지속 피해의 원본 코드는 SFX 번호를 조회하지 않고 OpenAL 핸들 15를 직접 사용한다. WeMade 포팅판의 음원 초기화 순서와 OpenAL Soft 할당자를 대조한 결과 Missile06.wav에 해당해 그 파일을 연결했다. 옛 iOS OpenAL 구현의 핸들까지 대조한 결과는 아니다.

DamageAudioSweep의 생존·사망·드롭·지속 피해·EMP·일시정지·난수 순서 72조건을 iPad에서 통과했다(0.063초). 효과음 네 파일은 원본과 바이트가 같다. 원본 공간 좌표를 전달하는 3D 음향은 추가 대조가 남았다.

같은 iPad에서 1800프레임·효과음 410회·저장 쓰기 0회를 기록했다. 기본 무장·최고 무장·유닛 추가 구간 각각 화면 갱신 간격 p95 16.67ms, 25ms 초과 0회. CPU p95는 순서대로 2.36/2.55/3.40ms였다. 결과 `.local/final-audio-performance-ipad.xcresult`. 최신 전체 CTest 33/33 통과(184.48초), `.local/av-audit-20261104/core-tests-hit-audio.log`.

## iPad 직접 조작 — 2026-11-05 00:11

테스트 플러그인이 없는 최종 치트 앱을 Xcode GUI에서 새로 빌드하고 iPad에 설치했다. 서명 검사 통과. 실제 IPADTEST 프로필의 Stage2에서 포탑 선택→Tap to build 안내→위치 지정→생산→배치→사격을 확인했다. 홈 화면으로 나갔다 돌아오자 System Menu가 열렸고, 셀 412에서 멈춰 있었다. Resume Game을 누르자 메뉴가 닫히고 셀 증가·적 이동·포탑 사격이 재개됐다. 복귀 뒤 특수기술 패널과 Menu도 직접 눌렀다. Save & Exit로 타이틀 복귀를 확인하고 프로세스를 종료했다. iPad 홈 화면도 확인했다.

Device Hub 전체 창의 좌표 입력 오류는 작은 기기 창으로 전환한 뒤 해소됐다. 재현 조건과 복구 방법을 apple-wireless-device 스킬에 기록했다. 이번 화면 검사는 오디오 녹음 검사를 포함하지 않는다. 사진 촬영 후 실제 출력음 비교는 남아 있다. iPhone 정식 앱은 변경하지 않았다.
