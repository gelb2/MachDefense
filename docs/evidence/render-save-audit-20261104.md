# 렌더링·저장 호출 검사 · 2026-11-04

사용자가 HeavyMach1에서 지적했던 발사마다 저장, UIKit으로 게임 그림을 그리는 구현이 재발했는지 검사했다. 기준은 기능 커밋31395fb와 인계 커밋bca4a5e 이후 소스다.

## 확인 결과

- 제품 코드에 SQLite·Core Data·NSUserDefaults 저장은 없다. Apple의 실제 저장 쓰기는 MDApplication.mm의 Services::writeSave 한 곳이며 NSDataWritingAtomic으로 저장 파일을 교체한다.
- 호출자는 Menu의 프로필 생성·선택·삭제, GameSession의 결과 처리·Save & Exit·명시적 TEST SAVE·실패한 결과 저장 재시도다.
- World의 발사·피해·파괴·재화 획득은 메모리 상태를 바꾼다. SaveData::writeWord도 bytes_ 배열을 바꾸며 파일을 열지 않는다. 기지 파괴로 미션이 끝날 때는 결과를 한 번 저장한다.
- World::appendSprites와 BattleModels::appendSprites가 부품 그림을 Frame에 넣는다. MDRenderer가 원본 이미지를 텍스처로 읽어 glDrawArrays로 출력한다. 텍스처와 이미지 크기는 캐시한다. 유닛·HUD·게임 버튼에 UIButton·UILabel·UIImageView·UIGraphics·drawRect를 사용하지 않는다.
- UIKit 사용은 앱 창·GL 표시용 뷰·터치 전달·생명주기와 프로필 이름 입력창이다. 텍스트 입력창의 OK/Cancel은 UIKit 버튼이다.
- 음원은 prepareSounds에서 읽어 OpenAL 버퍼에 보관한다. 발사 때는 버퍼를 재생하며 음원 파일을 다시 읽지 않는다.

## 찾은 문제와 수정

GameSession::drawReward가 특수공격 보상을 그릴 때 SpAttackList.tsv를 다시 읽고 파싱했다. 보상 아이콘이 여러 개면 한 프레임에도 반복됐다. 기존 AttackCatalog를 재사용하도록 수정했다.

GameSession::rebuild는 오류창을 그릴 때 글꼴 표를 다시 읽고 SpriteFont를 만들었다. Session 초기화 때 만든 글꼴을 재사용하도록 수정했다.

FrameIOTests에서 수정 전 오류창60프레임의 데이터 읽기60회, 보상창60프레임의 읽기180회를 재현했다. 수정 후 두 값은0회다. 전투1200tick에서는 생산이 완료됐고 저장 쓰기0회·표/글꼴 읽기0회였다. 새 이미지의 최초 텍스처 로딩과는 별도 검사다.

관련 FrameIO·GameSession·SaveFailure·ValidationTools·Campaign·NaturalCampaign·SummaryDialog 7종이 통과했다. 로그는 `.local/frame-io-before.log`, `.local/frame-io-after.log`다. 80개 자동 교전과 진행·보상·저장 검사도 다시 통과했다.

## 당시 측정 도구와 기기

기존 코어 검사에서 ASan·UBSan을 사용했고, 시뮬레이터 지연은 CPU sample로 조사했다. Apple 렌더러 검사는 텍스처 재사용과 그림 순서를 확인한다. Instruments Allocations·Leaks로 실기기 메모리·누수를 측정한 결과는 아직 없다.

iPad Air 13-inch M2의 localNetwork 연결, paired, 개발자 모드 활성화를 확인했다. 일반·치트 앱의 실기기 서명·설치·실행 검사는 별도 실행이 필요하다. 현재는320×480 원본 논리 화면이며 전체화면 대응은 후속 단계다.

## 후속 iPad 측정

이후 치트 앱을 iPad에 설치하고 Time Profiler·Allocations·Leaks 기록을 수집했다. 효과음마다 오디오 출력을 시작·중단하는 지연을 수정하고 14,912프레임을 재측정했다. [트러블슈팅](../troubleshooting.md)에 원인과 측정값을 기록했다. 메모리 기록의 누수 판정, 일반 앱과 전체 무기 검사는 남았다.
