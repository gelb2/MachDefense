# WeMade 1.0.3 메뉴 배치 조사

원본 SHA-256: `882fe8efa9a0c3d51c23da190ee3d30376a1b6766e16cc6b87daf09b98dc92b1`.
외부 로컬 분석의 `functions/0x*.asm`와 `Resources/images.json`을 대조했다. ARM 어셈블리에서 정수·실수 인자를 확인했다. 디컴파일된 부동소수점 인자는 레지스터 복원 오류가 있어 그대로 사용하지 않았다.

## 좌표와 이미지

`QobBase::calcWorldPos`(0x3e9b0)는 자신의 위치에 부모 worldPos를 더한다. 부모 회전·스케일을 자식 위치에 곱하지 않는다. `QobImage`(0x40918)는 타일의 중심을 기준으로 좌우·상하 절반 크기의 경계를 갖는다. 카메라가 있으면 screenPos는 worldPos − cameraPos + halfScreen − cameraOffset이다. fixedPos 1은 정수 위치, .5는 반 픽셀 위치로 반올림한다.

원본은 좌하단 원점, y가 위로 증가한다. 재구현 RenderTypes의 좌상단 원점에서는 중심 `(x,y)`와 크기 `(w,h)`를 `(x-w/2,480-y-h/2,w,h)`로 변환한다.

`getTileForRetina`(0x47f38)는 iPad(deviceType 1)를 제외한 기기에서 forRetina를 켠다. iPhone 논리 크기는 픽셀 크기의 절반이다. `getUniTile`(0x47a34)은 iPad에는 `iPad_`를, iPhone에는 `iPon_`를 붙이며 Retina에서는 `@2x`를 우선한다. 고해상도 이미지에만 절반 크기를 적용한다.

## 타이틀

`GuiTitle::init` 0x2ca6c, `GameMain::setScreen_Title` 0x7744.

| 요소 | 원본 이미지 | iPhone 중심 | 논리 크기 |
|---|---|---|---|
| 배경 | Title.jpg 640×960 | 160,240 | 320×480 (scale .5) |
| TAP TO START | Main_TapToStart.png 128×32 | 160,208 | 128×32 |

TAP TO START는 title child(0,-32)다. iPad에서는 배경 scale1.2, child y=-64다. processTitle(0x2cecc)는 2초 주기 중 0~.2초에 무작위 점멸, .2~1.2초에 표시, 1.2~2초에 숨긴다. 초기시간 추가값은0이다. title 내부 초기 bgScale/rotate 필드는 배경 QobImage의 실제 scale과 구별한다.

## 프로필 선택

`GuiSelectSlot::init` 0x2b180, refreshSlot0x2b7a4, handleButton0x2c684.

BG는 `iPon_SelSlot_BG@2x.jpg`640×960을320×480으로 그린다. row는 `iPon_SelSlot_Sel@2x.png`512×512의1×2세로타일이다. 타일별 논리 크기는256×128이며 실제 클릭 경계는256×60이다. 선택 타일0, 비선택1이다. active 슬롯만 생성한다.

슬롯 i의 중심은 `(160,340−80*i)`다. 0x2c2cc의 `mvn r0,#0x4f`, 0x2c2d4의 `muls`, 0x2c2d6의 `add #0x154`로 확인했다. i=4는 y20이므로 하단 버튼과 겹치는 값이 원본에 있다. 이 수치를 60간격으로 추측해 바꾸지 않는다. iPad에서는 `(width/2,740−120*i)`다.

| 하단 버튼 | 원본 strip | iPhone 중심 | 클릭 경계 |
|---|---|---|---|
| BACK | Common_Btn_BACK.png128×128,1×2 |80,30|원본 타일 경계|
| DEL | Common_Btn_DEL.png128×128,1×2 |160,30|원본 타일 경계|
| NEW | Common_Btn_NEW.png128×128,1×2 |240,30|원본 타일 경계|

하단 타일은 논리64×32다. default0/release1, pushDepth/releaseHeight/releaseDelay0이다. 프로필 행을 누르면 선택하고, 이미 선택한 행을 다시 누르면 진입한다. NEW는 빈 슬롯을 만들어 이름을 받는다. DEL은 확인을 요청한다. 원본은 선택 시 수정시간을 기록한다.

행 안 텍스트는 TrebuchetMS-Bold다. 프로필 이름의 원본 texture256×32,font20,Retina=true는 iPhone논리128×16,font10이다. 행 중심 기준 검은 그림자는(-24,20), 색(160,255,255)의 본문은(-24,21)이다. 점수는 texture256×32,font18,align2,중심(40,20). 국기는(-100,23),16×16이다. 생성날짜(-90,-2),저장날짜(-90,-20)는 NumSet01b.png128×8을16×1로 분할하고 pitch6,align0으로 그린다. 스테이지 숫자는(74,-1),credits는(88,-21),pitch6,align1이다. 날짜 format은 `%04d/%02d/%02d %02d:%02d:%02d`다.

## 미션 선택

`GuiSelectStage::init`0x3bc90,tick0x3cd68,handleButton0x3d078.

BG는 슬롯 BG와 같다. foreground는 `iPon_SelStage_Fore@2x.png`640×960,논리320×480,layer9이다. 버튼부모는 BG의 자식,layer5이다.

row i의 local중심은(0,-32*i),화면중심은 `(160,240+buttonBaseY−32*i)`이다. `SelStage_Btn_Stage.png`512×256의1×4타일이며 논리256×32다. 해제됨0/누름1,잠김2,선택overlay3이다. 생성 row수는 min(currentStage+11,80)이다.

텍스트 `Stage n`은 원본 font32,Retina=true여서 iPhone논리16이다. texture256×42→논리128×21,중심local(-10,0),align0. 해제됨RGB(10,255,255),잠김(10,10,10). MapList ImageFile을 읽어 `tBG_xx_01.png`와 `tBG_xx_02.png`를 표시하며 중심x38,86,각48×24이다. 완료표시는 SelStage_CompleteMark.png,중심x160,논리64×16이다.

BACK(80,30),SUPPORT(160,30),OK(240,30)의 hit경계는80×40이다. BACK/SUPPORT는1×2,OK는1×4타일이다. SUPPORT는 원본 nMailview를 연다. 원본 광고 banner는 사용자 광고금지 지시로 재생성하지 않는다.

camPos 초기값=currentStage×32. topLimit120,btmLimit125. tick에서 dragVel이 .1보다 크면 10분의1씩0으로 감속하고 camPos에 더한다. camPos가 topLimit보다 작으면5분의1씩복귀한다. 아래 한계는 currentStage<5일 때125,그외currentStage×32이다. buttonBaseY는camPos까지5분의1씩이동한다. 원본 코드가 위아래 경계를 순서대로 처리하므로 범위를 임의로 clamp하지 않는다.

선택된 row를 다시 누르거나 OK를 누르면 슬롯+0x34에 선택한 미션을 기록하고 다음 화면4로 진입한다. 미해제row는 진입하지 않는다.

## 남은 대조

이 문서는 메뉴의 원본 수치와 자원 참조를 기록한다. 화면 전환 도어의 원본 함수와 애니메이션은 아직 대조하지 않았다. SUPPORT 메일의 현대 플랫폼 연결은 별도 host 기능이 필요하다. 원본 마지막 슬롯 겹침은 실화면으로 추가 확인한다.

## 구현과 검사

Menu.hpp/.cpp는 위 iPhone 논리좌표의 타이틀·프로필·미션 목록을 그린다. 원본 프로필 경계와 strip 타일을 사용한다. 이름 입력의 OS 키보드는 HostServices로 요청한다. 원본 alertView handler(0x2c374)가 1~10글자만 허용하므로 영어 이름 1~10글자를 검증한다. 저장 실패 시 생성·삭제·프로필 선택 전의 메모리 상태를 되돌린다.

타이틀의 점멸은 DarwinRandom과 .0166초 tick을 사용한다. 0~.2초 구간에서 첫 random()%10이5이하일 때만 두 번째 random()의 하위 비트를 읽는다. 게임 전체의 원본 random() 상태와 연결한 검증은 남아 있다. 현재 메뉴 객체가 별도 RNG 상태를 갖는다.

NumSet01b의 마침표10,슬래시11,콜론12,빼기13,퍼센트14,별표15는 QobImageFont::setText(0x41704)에서 확인했다. setText가 마지막 문자부터 child를 만들고 tick이 child를0,-pitch순서로 배치한다. align0은 첫 문자 중심이 지정좌표,align1은 마지막 문자 중심이 지정좌표다.

`Tests/MenuTests.cpp`는 실제 준비 리소스로 실행했다. 타이틀 점멸 구간,잘못된 이름,프로필 생성·선택·삭제취소·삭제완료,잠긴 미션,스크롤 후 취소,저장실패 복구,날짜의 slash/colon 타일번호,숫자 정렬,사용 이미지 존재와 crop경계를 확인했다. 타이틀·메뉴의 연속 tick에서 저장 호출은 없고 프로필 변경 시에만 발생했다. clang++ C++20 `-Wall -Wextra -Werror`와 테스트가 통과했다.

미구현 항목: 원본 도어 전환 대조,메뉴 BGM 시작·정지 정책,원본 SUPPORT 메일의 현대 host 연결,버튼 release시간에 따른 애니메이션의 전 범위. 현재 SUPPORT를 누르면 원본 MessageBox 자원으로 이 빌드에서 메일 기능을 사용할 수 없음을 표시한다. 실제 시뮬레이터 화면 대조는 별도 검증이 필요하다.

## 도어 전환 구현 — 후속 확인

GameMain::start0x6028/tick0x67e4/changeScreen0x768c/refreshScreen0x76d4를 대조해 Transition.hpp/.cpp로 구현했다. Loading_Left/Right.png는각256×512,Top/Bottom.png는각512×512,Center.png는256×256이다. iPhone에서 Retina절반에left/right scale2,top/bottom scale1.5를 적용한다. Center는bottom의자식이며y+84에독립scale을적용한다.

iPhone시작좌표(topY,bottomY,leftX,rightX)는(672,-192,-128,448),닫힌좌표는(304,176,96,224)다. left가80에이를때좌우닫기단계에서상하닫기단계로넘어간다. 좌우는차이/5,상하는차이/6만큼매tick이동하며.1이내에서목표값으로맞춘다. Center는1.5에서.9로차이/5씩줄여1미만일때1로맞춘다. 이때다음tick에화면을교체한다. 다시Center를1.5로확대하고1.4를넘으면도어모두시작점으로차이/10씩연다.

폰에서state2가13tick,state3이60tick,state4가69tick,화면교체가70tick,state7이78tick,완전열림state8이157tick이다. Close사운드2회,Open1회다. state7이상에서다음화면전환요청을받는원본규칙도구현했다. Tests/TransitionTests.cpp로좌표·자원·타이밍·소리순서·iPad수치·중간재진입을검증했다.

메뉴BGM은CFString0x6bbfc=Intro.mp3,0x6baec=UI_BGM.mp3를확인하고HostServices로연결했다. 메뉴버튼의pushDepth/pushDelay/releaseHeight/releaseDelay는QobButton::init0x3f7f4에서모두0이다. Slot에서도0을명시하고Stage는변경하지않으므로메뉴에추가변위애니메이션을넣지않는다. 눌림상태releaseTileNo전환이원본draw0x3fd00과대응한다.


## 원본 특수공격·진형·공격 장비 화면

`GameHUD`는 `DlgSpecialAttack::refreshButtonsWithType:`(0x30de0)와
`DlgFormation::refreshButtons`(0x3dd94)의 원본 위치를 쓴다. 두 패널의 루트는
(327,604), 첫 행 로컬 좌표는 (-35,-176)이다. 절대 위치는 (292,428),
행 간격은 51이다. 원본 리터럴 0x318e4와 0x3e110의 iPhone X 값은 모두 -35다.
특수공격 아이콘은 AttackSet의 Icon%05d.png이며, 장착한 항목 순서대로 표시한다.
진형은 FormationIcon_01.png부터 05.png까지다. 두 패널의 행 선택 ID는 0x40e다.
UI는 수량·비용·레벨을 전달받아 표시하고 값 변경을 요청하는 동작만 반환한다.

`GuiGame::init`(0x25748)의 실제 세 번째 하단 버튼은 0x40c와 MachButton 타일6이다.
`handleButton:`(0x29000)은 이 ID로 300,000cr/$0.99 구매 확인을 연다.
진형 ID 0x40a의 처리 함수는 남아 있지만 이 버튼에 연결돼 있지 않다.
재구현은 진형 패널 API를 제공하며 0x40c를 진형으로 임의 변경하지 않았다.
장비 열기·닫기는 0x3fe/0x3ff, 기지 업그레이드 열기·닫기는 0x413이다.

공격 장비 창은 원본의 (-8,48) 루트, 즉 화면 (152,288)에 놓는다.
`refreshAttackButtons`(0x3778c)의 5열 간격은 46, 행 간격은 62다.
공격 상세는 `initWithAttackSet:`(0x39f64)의 UpgradeMachBG.png와 원본
UpgradeBtn_Upgrade/Buy/Install/Uninstall.png를 쓴다.
선택 0x412, 강화 0x403, 1개 구매 0x404, 장착 0x405, 해제 0x406을 반환한다.
강화 비용과 구매 가능 여부는 정산 모듈이 계산하고 UI는 다시 계산하지 않는다.

실제 Resources를 읽는 GameHUDTests와 UpgradeDialogTests에서 특수공격 선택,
진형 5개 선택, 공격 구매·강화·장착·해제, 비용 부족 버튼 차단,
화면 갱신 사이의 누름 유지, 모든 사용 이미지의 자르기 영역을 검사했다.
이 검사는 독립 C++ 화면·입력 검사다. 전체 게임 및 기기 동작은 통합 검사 대상이다.

### 통합 검사에서 수정한 버튼 전달

장비 창은 전체 화면 입력을 막지 않는다. 창 밖 터치를 GameHUD에 전달한다.
`turnOffButtons:`(0x2817c)는 장비 창에서 시스템·기지 버튼만 끄고,
하단 패널 선택과 오른쪽 슬롯을 유지한다. 기지 창에서는 오른쪽 슬롯과
하단 패널 선택도 끈다. 이 구분은 `baseUpgradeOpen` 상태로 전달한다.

오른쪽 위 0x3fd는 크레딧을 쓰는 영구 CellStorage 강화와 다르다.
원본 `handleButton:`(0x29000)은 전투 중 셀을 소비해 임시 셀 레벨을 올린다.
`refreshCellUpgradeCost`(0x28b84)의 비용 문구는 `%d cell`, 최대 5단계에서는
`FULL`이다. HUD는 `cellUpgradeCost`와 `cellUpgradeLevel`을 받아
(304,457.5)에 원본 14px 글자를 절반 크기로 표시한다.
