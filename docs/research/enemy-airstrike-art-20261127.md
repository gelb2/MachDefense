# 적 공습기 아트 조사

2026-11-27. 원격 develop `17a3506`의 소스와 위메이드 MachDefense 1.0.3에서 준비한 PNG를 대조했다. 원격 main `7ea9fa4`와 이 조사 기준 사이에 Sources·Resources·Android·Web 파일 차이가 없다. 아래 렌더 절차는 현행 소스판 기준이다. 1.0.3 호환 실행기의 실제 출력과 원작 미술의 제작 프로그램·브러시·3D 작업 여부는 확인하지 못했다.

## 아군 공습기는 어떻게 그리나

1. 특수공격을 발동하면 `World::activateSpecial()`이 무기 정의와 목표 좌표로 `AirStrike`를 만든다. 지면의 `Target.png` 표시는 별도 파티클이다. 기체 외형은 스킬 종류·레벨과 무관하게 `Airplane.png`를 사용한다. [발동 코드](../../Sources/Core/Simulation/WorldCombat.cpp)
2. `World::appendSprites()`는 기체 위치를 화면 위치로 변환하고 기체를 레이어 7에 올린다. `AirStrike::appendSprite()`는 우향 PNG 전체를 정사각형 한 장으로 그린다. 이미지 중앙 `(128, 128)`이 기준점이며 회전각은 비행각을 화면 방향에 맞춰 뒤집은 값이다. 별도 부품, 추가 그림자, 프레임 애니메이션, 런타임 조명은 이 스프라이트 경로에 없다. [월드 그리기](../../Sources/Core/Presentation/WorldPresentation.cpp) · [공습 그리기](../../Sources/Core/Simulation/SpecialAttack.cpp) · [스프라이트 기본값](../../Sources/Core/Contracts/RenderTypes.hpp)
3. 원본 `Airplane.png`는 256×256 CgBI PNG다. 자원 준비는 BGRA와 사전 곱셈 알파를 일반 RGBA PNG로 변환한다. 실제 런타임 소비 파일은 [Images/Airplane.png](../../Resources/Images/Airplane.png)이다. 현재 공습 스프라이트는 Apple·Android·Web 렌더러에서 선형 필터와 일반 알파 합성을 선택한다. 배율·회전 뒤 가장자리에서 이 차이가 보일 수 있으므로 최종 크기에서 비교한다. [자원 준비](../../Resources/README.md) · [이미지 명세](../../Resources/images.json) · [Apple](../../Sources/Platform/Apple/MDRenderer.mm) · [Android](../../Android/app/src/main/cpp/Renderer.cpp) · [Web](../../Web/src/Renderer.cpp)
4. 스프라이트 변의 길이는 `256 × 1.3 × deviceScale`이다. 현행 `GameSession`은 phone 전투 좌표계 `0.5`로 World를 만든다. 따라서 공통 월드 확대 전 사각형은 166.4×166.4 논리 단위다. 회전 전 알파가 0보다 큰 범위는 약 135.2×137.2 논리 단위다. 넓은 화면의 월드 확대와 출력 기기 배율은 이후 적용한다. 이 값은 실제 화면 픽셀 수와 구분한다. [세션 초기화](../../Sources/Core/Session/GameSession.cpp) · [월드 배율](../../Sources/Core/Presentation/WorldPresentation.cpp)

## PNG에서 확인한 질감

원본 공습기는 기수가 오른쪽을 향한 탑뷰다. 날개와 동체의 위쪽 면이 넓게 보인다. 날개는 채도가 낮은 회녹색이다. 날개 뿌리와 동체가 만나는 곳은 어둡고, 기수 양옆과 날개 끝 일부는 밝은 상아색이다. 짧은 밝은 얼룩과 길게 퍼지는 명암이 함께 있다. 외곽은 반투명 픽셀로 이어진다. 표면의 입체감은 이미지 자체에서 보인다.

불투명 픽셀 표본은 날개 중간톤 `#6D7C73` (101, 71), 어두운 연결부 `#333E35` (107, 143), 기수 상아색 `#DAD1BD` (184, 113)이다. RGB 가중 밝기 `0.2126R + 0.7152G + 0.0722B`를 반올림하면 불투명 픽셀의 10·50·90백분위는 69·102·123이다. 도장색을 그대로 전사하는 목표값보다 원작의 중간톤과 명암 폭을 비교하는 참고값으로 쓴다.

좌표는 좌상단 기준이며 범위의 오른쪽·아래쪽 끝은 포함하지 않는다. Pillow로 준비 PNG의 알파를 읽고 파일 SHA를 자원 명세와 대조했다.

| 위메이드 1.0.3 표본 | PNG와 알파 범위 | 참조 용도 |
|---|---|---|
| [Airplane.png](../../Resources/Images/Airplane.png) | 256×256, 알파 > 0 범위 `(34,19)–(242,230)`, 208×211. 알파 ≥ 128은 202×209. 반투명 3,068px, 불투명 18,179px | 기체 시점·광택·색·명암·외곽·회전 기준 |
| [E_Tank_Body_01.png](../../Resources/Images/E_Tank_Body_01.png) | 55×32, 알파 > 0 범위 55×32 | Stage 1 적 탱크. 작은 화면에서 읽히는 장갑 경계와 마모 밀도 |
| [P_Body_18.png](../../Resources/Images/P_Body_18.png) | 64×64, 알파 > 0 범위 `(12,11)–(51,52)`, 39×41 | MachineGun Lv.10 몸체. 대조용 밝은 장갑 면과 어두운 파츠 경계 |

| 표본 | 원본 CgBI SHA-256 | 현행 PNG SHA-256 |
|---|---|---|
| Airplane | `50d513cc152244dd01b982512ad4a5b592404e871b14bdfc938f456821d457f7` | `931bd62d2f15c48b4262fad2ce27b5473ceca1bbd5e7f3d3fde742b65d2fc786` |
| E_Tank_Body_01 | `df945709e9a50f90915939302babdb83912047873681973f5e9a7acc76526bd3` | `415592d9b7df8081b368d8c38df22e30fb4a1e1ffcb0d6da22512cbfe61df49f` |
| P_Body_18 | `9714ad1d7e0d5f9b1b2a029b4e18038b3f9e09304afbd6cbaff245b938325b41` | `b0962cc7192096964019f938606ee5d9e80b598681c9d730a82916e3a2b5918e` |

## F-15 STOL/MTD 적 공습기 제작안

사용자 목표는 럭스루나에 도장의 F-15 STOL/MTD 기체가 원작 공습기와 나란히 보일 때 같은 개발자가 그린 기체처럼 보이는 것이다. HeavyMach 1에서 사용한 참조 방식은 형태·도장·게임 질감을 따로 잡는 것이었다. 그 방식으로 MachDefense 탑뷰 스프라이트를 만든다.

1. **형태.** F-15 STOL/MTD를 형상 기준으로 삼아 주익 앞 카나드 한 쌍, 2인승 캐노피, 사각 흡입구, 쌍미익과 쌍발 꼬리를 잡는다. F-15E는 기본 계열의 비교 자료로 본다. 먼저 우향 탑뷰에서 기수·카나드·주익·꼬리 사이를 분리한 실루엣을 그린다. [미 공군 F-15E 자료](https://www.acc.af.mil/Portals/92/Docs/Fact%20Sheets%20-%202020%20Update/Facts%20Sheets%20-%202023%20Update/F-15E%20Strike%20Eagle_FEB%202023.pdf) · [NASA STOL/MTD 사진](https://www.nasa.gov/image-article/f-15-stol-mtd-arrives-dfrc/) · [NASA F-15B 상면 자료](https://www.nasa.gov/wp-content/uploads/2021/04/88798main_srfcs.pdf)
2. **도장.** 럭스루나에의 짙은 올리브 회색을 큰 면에 쓰고 탁한 금색 선은 기수 옆과 꼬리 일부에 가늘게 넣는다. HeavyMach 1 채택본의 금색 캐노피 반사는 작은 색 강조점으로 시험한다. 표식은 탑뷰에서 읽힐 크기의 독자 도안으로 정리한다. [럭스루나에 공식 자료](https://www.bandainamcoent.com/news/ace-combat-8-wings-of-theve-game-overview)
3. **명암과 재질.** 우선 무채색으로 날개·동체 어깨·흡입구의 세 덩어리를 나눈다. 날개 뿌리와 두 엔진 사이를 어둡게 두고, 상부 패널과 기수에 짧은 밝은 면을 둔다. 도장을 올린 뒤에도 이 명암 폭이 남아야 한다. 패널선은 구조를 가르는 곳에만 두고 금속 마모는 날개 진행 방향을 따라 작은 덩어리로 넣는다. 원작 PNG와 비교해 검은 테두리, 번쩍이는 광택, 균일한 미세 노이즈를 줄인다.
4. **스프라이트 준비.** 첫 시안은 중앙 피벗을 잡은 256×256 RGBA 한 장으로 시험한다. 원작의 약 135 논리 단위 실측 범위를 초기 비교 크기로 쓰고 STOL/MTD 형상의 길이와 폭은 균일 배율로 조정한다. 날개 끝까지 포함한 폭·기수부터 꼬리까지 길이·중앙 동체 폭을 따로 잰다. 작업 원본과 내보낸 PNG, 크롭·피벗·알파 범위, 해시를 함께 보존한다.
5. **화면 비교.** 같은 맵과 화면 배율에서 아군 공습기와 후보를 나란히 놓는다. 회전 0°·90°·180°와 실제 통과 중에 실루엣, 선형 필터가 적용된 가장자리, 밝고 어두운 지형 위 명암을 비교한다. 확대 도안과 실제 크기 캡처를 함께 제시한다. 먼저 외형을 고른 뒤 실제 리소스 로딩·레이어·진입·투하·퇴장을 검증한다.

아군과 같은 일회성 공습이라면 이 한 장짜리 경로가 첫 구현 후보다. 적의 접근 방향·경고·표적 선택을 정의하고, 지금 아군용으로 생성하는 투사체의 피해 진영을 명시해야 한다. 피해 반경이 0이면 현재 충돌 경로는 적 목록만 보고, 범위 피해에서만 `toPlayer`로 대상을 고른다. 격추형을 선택하면 별도 유닛의 체력·표적·격추 처리와 투하 중단을 설계한다. 발동 조건과 격추 여부는 미정이다. 무기·연출 재사용과 피해 기준은 [적 공습기 투두](../usability-todo.md#md-enemy-airstrike-01--적-공습기-구상)를 따른다. [투하 연결](../../Sources/Core/Simulation/World.cpp) · [충돌 검사](../../Sources/Core/Simulation/Projectile.cpp)

이번 단계의 결과는 소스·원본 이미지 대조와 제작안이다. 아래 후속 구현과 검증은 기능 브랜치의 결과다.


## 구현과 검증

기능 브랜치 `feat/enemy-airstrike-20261127`은 develop `17a3506`에서 분기했다. 11스테이지부터 60초 이후에 한 번 출현하는 안을 채택했다. 현재 전방 메크 위치는 조준에만 사용한다. 피해 상한은 전체 생산 표를 읽어 기존 `PlayerLoadout::fromBuild()`로 계산한다. 현재 최저치는 초기 지급되는 `MachineGun_01`의 300 HP다. 강화하지 않은 상태로 후반 및 재도전에 출전할 수 있어 이 최저치가 모든 적용 스테이지에 유효하다.

최대 피해 합계는 첫 공습 스테이지의 75에서 마지막 스테이지의 150까지 선형 증가한다. 기존 폭탄 공습 1레벨의 탄, 폭발과 효과음을 사용한다. 적 공습탄에만 목표 진영과 탄당 피해를 전달한다. 총량을 기존 투하 수로 정수 나누고 나머지를 앞선 투하에 1씩 배정한다. 범위 피해 감쇠를 거친 실제 누적 피해는 이 합계 이하가 된다. [출현·배분](../../Sources/Core/Simulation/SpecialAttack.cpp) · [투하](../../Sources/Core/Simulation/World.cpp) · [착탄](../../Sources/Core/Simulation/Projectile.cpp)

[채택 이미지와 준비 명령](../../Art/EnemyAirStrike/README.md). 적기는 일회성 공습 경로로 렌더한다. 검증용 고정 표적은 게임 제품에 추가하지 않았다.


- 공용 코어에서 80개 스테이지의 발동 여부를 확인했다. 70개 공습 스테이지마다 메크 70개 조합을 최대 피해 위치에 놓고 집속탄 10발의 실제 충돌 계산을 실행했다. phone·pad 조건 모두 최소 잔여 HP가 최대 HP의 50% 이상이었다.
- 실제 `World` 진행으로 80개 스테이지를 검사했다. 11~80에서 경고·기체·투하·착탄이 이어졌고 가장 늦은 착탄은 경고 후 약 4.22초였다. 마지막 스테이지의 기본·강화 편성에는 같은 피해 총량이 나왔다. 일시정지와 스테이지 종료 뒤 취소도 검사했다.
- iPhone Air 시뮬레이터의 제품 렌더러로 같은 전투 프레임에 두 기체를 그렸다. [아군·적군 공습기 화면](../evidence/enemy-airstrike-20261127/ios-simulator.png). 테스트에서는 메크를 지정 위치에 두었다. 실제 기기와 일반 난이도 전체 플레이에서의 균형은 후속 확인 대상이다.
- Release CTest 58개 중 55개가 통과했다. Level 10 장비를 사용한 자동 교전은 80개 미션을 모두 클리어했다. `AircraftAudit` 미사용 파츠, `AircraftCampaign` 몸체 스프라이트, `Level10HUDSweep` 설치 버튼 이미지 실패는 변경 전 `17a3506`에서도 각각 재현됐다.

## 외형 재작업

사용자는 첫 그림의 화풍, 럭스루나에 도장, F-15 STOL/MTD 형상이 모두 맞지 않는다고 지적했다. 첫 생성에는 원작 PNG와 형태·도장 사진을 첨부하지 않았다. 수정 작업에서는 [입력 이미지와 실제 프롬프트](../../Art/EnemyAirStrike/README.md)를 사용했다. 원작 PNG로 명암과 질감을, 공식 편대 사진으로 도장 위치를, NASA 사진 두 장으로 STOL/MTD 형상을 대조했다.

첫 시안의 넓은 호박색 캐노피와 둥근 노즐을 가는 회녹색 캐노피와 사각 노즐로 수정했다. 날개에 둘렀던 금색 선을 빼고 기수 옆의 탁금색 선과 작은 꼬리 표식을 적용했다. 최종 그림의 크롭과 축소 명령을 기록하고 적기 PNG를 교체했다.

[이전 전투 화면](../evidence/enemy-airstrike-20261127/ios-simulator.png) · [수정 전투 화면](../evidence/enemy-airstrike-20261127/ios-simulator-revised.png). 같은 iPhone Air 시뮬레이터 테스트에서 원작기와 수정기를 한 프레임에 그렸다. 테스트 번들의 수정 PNG를 소스와 대조했다. 전투 배율에서 기수 옆의 가는 도장선은 보이지만 작은 꼬리 표식은 읽기 어렵다.

## 검은색과 금색 도장 재수정

사용자가 전투 화면에서 검은색과 금색을 알아보기 어렵다고 지적했다. 이번 편집에는 헤비멕1 채택 F-22·MiG-31을 도장 기준으로 직접 첨부했다. MachDefense의 `Airplane.png`는 무광 표면과 부드러운 명암을 맞추는 데 사용했다. STOL/MTD의 카나드·가는 복좌 캐노피·사각 노즐을 남기고 동체를 검은 차콜색으로 바꿨다. 금색 유리와 기수 양옆 도장선의 명도와 면적을 높였다. [참조와 실제 프롬프트](../../Art/EnemyAirStrike/README.md#검은색과-금색-도장-수정).

[회녹색 수정본 캡처](../evidence/enemy-airstrike-20261127/ios-simulator-revised.png) · [검은색·금색 캡처](../evidence/enemy-airstrike-20261127/ios-simulator-black-gold.png). 같은 iPhone Air 시뮬레이터 전투의 320×480 프레임을 비교했다. 검은 동체와 금색 유리·선이 배율을 확대하지 않아도 보인다. 흰색 `055`는 원본 그림에서 읽히며 전투 프레임에서는 작다. 캡처의 픽셀 변화는 적기 영역에만 있다. 변경한 PNG가 빌드 및 테스트 앱 번들에 들어간 것을 바이트로 대조했다.

## 빨간 공습 표적

아군 스킬의 `Target.png`에서 녹색 신호값을 투명도로 옮겨 32×32 빨간 조준점을 만들었다. 적 공습이 표적 좌표를 확정하면 투하 전에 그 위치에 표시한다. 적기가 표적 위를 지날 때도 보인다. [빨간 조준점 PNG](../../Resources/Images/TargetEnemyRaid.png) · [표시 코드](../../Sources/Core/Presentation/WorldPresentation.cpp)

iPhone Air 시뮬레이터에서 치트판의 TEST 기능으로 11스테이지를 열었다. 투하 전 적 표적에는 빨간 조준점, 아군 스킬 표적에는 녹색 조준점이 보였다. 메크가 이동하고 적기가 표적 위를 지나가는 동안에도 빨간 조준점은 최초 표적에 남았다. 공용 코어 검사 4개와 치트판 렌더 검사 1개가 통과했다. [투하 전](../evidence/enemy-airstrike-20261127/ios-cheat-enemy-red-reticle-before-bombs.png) · [메크 이동 후 적기 통과](../evidence/enemy-airstrike-20261127/ios-cheat-enemy-air-raid-and-allied-airstrike.png)

치트판에서 발동 0.109초와 0.656초 프레임을 비교했다. 빨간 조준점의 스프라이트 폭은 84.28에서 24.15px로, 녹색은 96.34에서 24.18px로 줄었다. 같은 구간에 두 조준점 모두 약 189° 회전하고 옅어졌다. 녹색은 3초, 빨간색은 5초 표시되므로 빨간색이 더 천천히 옅어진다. [치트판 프레임 비교](../evidence/enemy-airstrike-20261127/ios-cheat-reticle-motion.png)

## 반복 출격 표

지상 적의 `ESpawn_*.tsv`에는 시간 지정 규칙과 난이도별 규칙이 있다. `World::start()`가 시간 지정 규칙을 읽고 생성 웨이브를 계산한다. 난이도별 규칙 선택과 한 웨이브 안의 개별 출현 간격은 전투 중에도 계산한다. 적 공습은 시간표 방식으로 연결했다. [지상 적 출격 코드](../../Sources/Core/Simulation/Spawning.cpp)

11~80스테이지마다 첫 60초와 후속 출격 31회의 절대 시각을 기록했다. 제작 시 20~45초 정수 간격 32개를 무작위로 뽑았다. 마지막 간격은 다음 주기의 첫 출격까지 이어진다. 전투 코드는 스테이지 시작 시 읽은 한 행을 소비한다. 출격 간격을 정하기 위해 난수를 호출하거나 저장 파일에 기록하지 않는다. [출격 표](../../Resources/Tables/EnemyAirRaidSchedule.tsv) · [표 읽기](../../Sources/Core/Simulation/World.cpp) · [시각 소비](../../Sources/Core/Simulation/SpecialAttack.cpp)

공용 코어 검사 5개가 통과했다. 70개 행을 각각 세 주기 검사했다. 11스테이지 전투에서는 첫 32회와 다음 주기의 첫 출격까지 표와 일치했다. 아군 메크가 60초에 없던 재도전은 첫 회차를 건너뛰고 81초와 119초에 출격했다. iPhone Air 시뮬레이터 치트판의 첫 출격과 표적 표시 검사도 통과했다. [전투 검사](../../Tests/WorldSpecialTests.cpp) · [치트판 렌더 검사](../../Apple/Tests/MenuRenderTests.mm)

### 플랫폼별 치트판 대조

iPhone Air와 iPad Air 시뮬레이터, Android API 36 에뮬레이터, Chromium과 WebKit 브라우저, 오프라인으로 다시 연 설치형 PWA, macOS Tauri 검증 앱에서 11스테이지의 60·81·119초 출격과 빨간 조준점을 확인했다. macOS 검증 앱은 웹 치트 빌드를 독립 검증 번들에 넣어 실제 WKWebView에서 실행했다. [Apple 전투 검사](../../Apple/Tests/MenuRenderTests.mm)

아군과 적군 공습기는 같은 `AirStrike` 갱신을 사용한다. 진입 방향의 세로 성분은 반대다. 초기 속력은 같은 배율에서 같고, 이후 매 시뮬레이션 틱에 1%씩 증가한다. 이 가속은 투하 뒤 퇴장까지 이어진다. 같은 난수와 무기로 두 경로를 시작해 퇴장까지 위치와 속력을 틱마다 비교했다. [공습 코드](../../Sources/Core/Simulation/SpecialAttack.cpp) · [비행 대조 검사](../../Tests/SpecialAttackTests.cpp)

### 여러 대 순차 출격과 기체당 피해

출격표의 기존 시각은 그대로 사용한다. 11~80스테이지의 각 회차에 1~3대를 같은 확률로 뽑아 기록했다. 스테이지를 열 때 표의 대수를 사용해 후속 기체의 진입 시각을 1.5초 간격으로 준비한다. 기체마다 진입 시점의 전방 메크를 표적으로 삼고 빨간 조준점을 표시한다.

기체 한 대의 피해 상한은 출전 가능한 메크의 최저 최대 체력에서 80%를 계산해 소수점 아래를 버린 값이다. 현재 최저 체력은 300으로, 기체 한 대의 최대 피해는 11스테이지 120부터 80스테이지 240까지 선형 증가한다. 2·3대가 출격하면 각 기체에 같은 기준을 적용한다.

공용 코어에서 80개 스테이지와 메크 70개 조합의 실제 충돌 경로를 검사했다. iPhone Air 시뮬레이터의 치트 빌드에서 11스테이지의 81·82.5·84초와 119·120.5초에 기체가 한 대씩 추가되는 것을 확인했다. [전투 검사](../../Tests/WorldSpecialTests.cpp) · [Apple 렌더 검사](../../Apple/Tests/MenuRenderTests.mm).
