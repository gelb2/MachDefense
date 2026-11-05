# MachDefense 판본 비교와 WeMade 실행 근거

2026-11-04. 조사 대상은 외부 원본 보관소의 indieapps 1.0.3·1.0.5, WeMade 1.0.3이다. 전체 파일 목록·SHA-256·Objective-C 메서드·복호화한 테이블을 대조했다. 로컬 산출물은 `.local/version-audit/comparison.json`이다. 추출 실행 파일과 테이블은 외부 빌드 폴더 `analysis/version-audit`에 있다.

## 판본 선택

|항목|indieapps 1.0.3|indieapps 1.0.5|WeMade 1.0.3|
|---|---:|---:|---:|
|패키지 내부 파일|1,264|1,266|1,295|
|PNG / JPG|936 / 153|942 / 152|948 / 161|
|데이터 테이블 `.tble`|84|84|94|
|MapList 미션 행|70|70|80|
|빌드 SDK|iPhoneOS 4.2|iPhoneOS 4.3|iPhoneOS 5.0|
|지원 기기 선언|iPhone·iPad|iPhone·iPad|iPhone·iPad|

WeMade판은 71~80번 적 생성 테이블과 BG26·27 배경을 추가했다. 확보한 판본 중 재개발 기준은 WeMade 1.0.3으로 정한다. 기존 indieapps 1.0.5 포팅판은 비교용으로 보존한다. 두 배포자의 버전 숫자만으로 출시 순서를 판단하지 않는다.

indieapps 1.0.3과 1.0.5의 84개 테이블·3개 mcb는 모두 같다. WeMade와 1.0.5는 공통 1,263개 중 1,251개가 같고, 공통 84개 테이블 중 82개가 같다. 77개 WAV·3개 MP3는 세 판본에서 같다.

WeMade의 MachList에서 Boss_01은 HP 25,000→33,000, EXP 900→30,000, Spd 0→0.4, Rad 100→48로 바뀌었다. Tanker_03의 `1.0`→`1`은 같은 값이다. MapInfo.mcb의 바이트 차이는 확인했지만 전체 필드 해석은 남았다. WeaponInfo·PartsList·BaseUpgrade·MachBuild_Name·MachBuild_Parts와 ESpawn_01~70은 같다.

## 언어와 에셋

세 판본에 Description_kr/us 테이블과 언어별 HelpScr 이미지 48개가 모두 존재한다. 유닛 이름은 테이블, 도움말은 언어별 이미지로 처리한다. 앞선 제한된 파일명 검색으로 언어 파일이 없다고 판단해서는 안 된다.

WeMade GlobalInfo init `0x19b8c`와 setLocaleCode `0x1a468`을 추적했다. NSLocaleCountryCode가 `kr`이면 한국어, 나머지는 `us`를 선택한다. indieapps도 같은 선택 구조다. 영어 선호 언어만 강제한 시뮬레이터에서 지역 KR을 유지하면 한글 이름이 남는 것을 확인했다. 미국 지역 실험은 아직 별도로 완료하지 않았다.

공통 PNG 939개 중 934개가 같은 WeMade판에도 별도 고해상도 유닛 세트는 발견하지 못했다. 대표 차체는 25×18·55×32픽셀이다. 배경은 512×1024 및 iPon 256×512, 도움말은 iPad 768×1024·iPon 320×480·@2x 640×960으로 나뉜다. 화면 확대와 필터링이 자글거림에 미치는 영향은 추가 비교한다.

## WeMade 시작 오류

1. `DefaultAppDelegate startAnimation`이 `setFrameInterval:0`을 호출한다. ARMv7 `0x3b30`에서 r2=0, `0x3b42`에서 objc_msgSend다. touchHLE의 `frameInterval >= 1` assertion으로 시작이 중단됐다.
2. 동일 iPhone 시뮬레이터의 네이티브 CADisplayLink를 호출했다. 기본값1, 2설정→2, 0설정→1, -1설정→1, 1설정→1이다. 프로브는 `.local/version-audit/displaylink-probe.m`이다. 호환 실행기에서 최소1로 정규화했다. 당시 구형 iOS의 관측으로 확대하지 않는다.
3. 프로필 선택 뒤 GuiBanner.initgb `0x4ed38`가 postwebobj 요청을 시작했다. 미구현 NSOutputStream/요청 객체를 거쳐 `GuiBanner.m` assertion과 잘못된 객체 수신 오류로 멈췄다. 로그는 `wemade103-iphone.log`다.
4. 광고 금지 지시에 따라 **WeMade 번들·GuiBanner 클래스의 initgb/tick/ClickBanner/onTap:State:ID:**만 요청·표시·광고 링크를 실행하지 않게 했다. 일반 객체 생성·해제 및 다른 네트워크/게임 객체 처리는 유지한다. 수정 후 미션 선택과 첫 전투 화면에 진입했다. 로그는 `wemade103-iphone-noads.log`다.

## 저장 형식 차이

indieapps 1.0.5는 42,476바이트(`0xa5ec`), 슬롯 간격 `0x2060`이다. WeMade 1.0.3은 42,496바이트(`0xa600`), 슬롯 간격 `0x2064`다. WeMade openDataFile `0x1b48e`, slotData `0x1b3ec`에서 확인했고 새 프로필의 저장 파일 크기·체크섬도 일치했다. 두 판본 저장을 그대로 교환하지 않는다.

WeMade DlgUpgradeBase.setUpgrade `0x345ac`/`0x345ea`가 슬롯+`0x3c`의 Cr을 읽고 차감한다. 테스트 도구는 판본별 크기와 슬롯 간격을 구분한다. 100만 Cr 시험 저장은 새 테스트 프로필의 Cr 및 체크섬만 바꾼 별도 파일이다. 사용자 아이폰 저장은 읽거나 수정하지 않았다.

## 추가 판본 검색

[시리즈 모음](https://archive.org/details/heavy-mach-archive_202406)의 MachDefense 파일은 현재 3종이다. [별도 1.0.5 항목](https://archive.org/details/Mach_Defense_1.0.5_ios_4.1_1781114582)은 현재 1.0.5와 SHA-1·크기가 같았다. [TouchArcade의 HD Universal 표기](https://toucharcade.com/games/mach-defense)는 별도 HD 패키지의 존재를 증명하지 않는다. 조사 범위에서 별도 MachDefense HD 파일은 찾지 못했다.

[2011-05-11 위메이드 발표](https://www.inven.co.kr/webzine/news/?news=35584), [2011-10-21 위메이드 발표](https://www.newswire.com/news/heavy-mach-assault-and-battle-games-for-ios-devices-get-major-updates-and-91125)는 인수·재출시 맥락을 설명한다. 미확보 빌드의 부재나 모든 출시판의 확보를 뜻하지 않는다.

## iPhone 시뮬레이터 직접 조작 결과

iPhone Air iOS 26.5, 별도 `com.gelb2.machdefense.audit.wemade103`에서 검사했다.

- 타이틀 → 문 전환 → 프로필 `1234` 생성 → Stage 1 → 도움말 → 전투 진입.
- 네 생산 버튼, 에너지 차감, 출격·이동·사격·적 파괴·크레딧 획득, 전장 스크롤 확인.
- Save & Exit Game 뒤 프로세스를 종료하고 다시 실행했다. 점수 274·164 Cr·프로필이 화면과 저장 파서에서 유지됐다.
- 원본 시험 저장을 보관한 뒤 credits만 1,000,000으로 바꾼 별도 fixture를 사용했다. 기지 강화 6종을 1회씩 UI에서 구매했다. 모든 표시 레벨이 2가 됐고 600+600+700+500+800+700=3,900 Cr가 차감돼 996,100이 됐다. 업그레이드 호환 오류는 재현되지 않았다.
- 생산 버튼의 가격 누락, KR 지역의 한글, 이름 입력칸 아래 이동은 남아 있다. 자글거림과 전체 미션·승패 전환 검사는 후속 항목이다.
- 화면 증거는 `.local/version-audit/wemade-combat.png`, `wemade-saved-slot.png`, `wemade-upgrades.png`다. 사용자 실기기 앱·저장에는 접근하지 않았다.
- `package-version.py`로 별도 위메이드 시뮬레이터 앱 패키징·서명 성공. 누락 runtime을 사전 검사하도록 보강했다. `save_fixture.py`의 두 형식 왕복·손상 체크섬 거부 검사 통과.

강화 후 Save & Exit → 프로세스 종료 → 실행 → 슬롯 선택 → 첫 전투 재진입 → 강화 메뉴 재개방에서 6종 레벨 2와 996,100 Cr를 확인했다. 증거: `.local/version-audit/wemade-upgrades-reloaded.png`.
