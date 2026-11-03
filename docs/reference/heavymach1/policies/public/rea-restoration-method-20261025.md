# HeavyMach의 REA 활용 내역

2026년 10월 25일 · HeavyMach 1.13 복원 기록 · 설치 제품 코드 `72e8c68e`

Codex는 HeavyMach 1.13의 ARMv6 실행파일에서 게임 규칙을 읽고, C++·Objective-C++로 다시 구현해 현재 아이폰에서 빌드·실행했다. 나는 설치본의 모든 미션을 직접 플레이했고 크래시·오동작·적 이동 이상은 보이지 않았다.

분석에 사용한 도구와 복원한 코드를 작업 순서대로 정리했다. 전체 함수 목록과 증거 색인은 부록에 있다.

## 1. 실제로 사용한 스킬·도구

사용한 REA 스킬의 이름은 **reverse-engineer-anything**이다. 여러 분석 기능을 안내하는 하나의 스킬이며 두 설치 위치에 같은 번들 판본을 두었다. 프로젝트에서 쓰인 패키지는 `rea-agents 6.7.0`, 스킬 판본은 `42`, 네이티브 분석 엔진은 `Ghidra 12.1.4`였다. 스킬 판본과 패키지 버전은 다른 번호다. 먼저 받은 저장소 main의 스킬 판본46은 백업한 뒤 설치 패키지와 맞는 판본으로 교체했다.

| 구성 | 프로젝트에서 맡은 일 |
|---|---|
| REA SKILL.md | IPA/네이티브 분석 경로 선택, 원본 식별, 도구 준비와 증거 해석 규칙 |
| connection-and-recovery | 스킬 설치, MCP 등록, 현재 세션 연결, 분석 엔진 준비를 분리해 진단 |
| native-and-artifacts | IPA 목록·추출, Mach-O/Objective-C, 함수 의사 코드와 어셈블리 대조 |
| evidence-workflows | Evidence ID·입력 해시·분석 한계·원본과 파생본 구분 |
| runtime-observation | 정적 코드와 실행 관측을 분리하는 기준. 이 게임의 ARM 실행 관측은 별도 GDB 도구로 수행 |
| REA CLI·별도 MCP 클라이언트 | 실제 패키지·함수 분석 명령 실행, 결과 저장, 세션 열기/닫기 |
| Ghidra | ARM/Thumb 함수 분석, 디컴파일, 어셈블리와 Objective-C 정보 제공 |
| Python·otool·원본 바이트 파서 | 자원 비교, 데이터 포맷 복원, 지원되지 않는 메타데이터 보완 |
| touchHLE 호환 실행판 | 내가 정상 동작을 확인한 원본 ARM 실행 기준, 런타임 표본 관측 |
| CMake·Xcode·XCTest·Instruments·Device Hub | 복원 소스 빌드, 자동 검사, 성능 계측, 아이폰 설치·직접 조작 |

Android/JADX, JavaScript/Electron, 펌웨어 등 스킬의 다른 경로는 이 HeavyMach 복원에 사용하지 않았다. 조회된 도구는 207개였다. 실제 사용한 분석 작업과 세션 제어는 아래 목록과 부록에 정리했다.

출처 저장소는 [morluto/rea](https://github.com/morluto/rea)다. 이 문서의 버전 설명은 당시 설치 기록과 로컬 package.json, 보존 Evidence를 기준으로 한다.

## 2. REA 설치·설정

Codex는 처음에 스킬만 설치했다. 내가 REA 본체와 MCP 도구가 필요하다고 지적하자 Codex가 `rea-agents@6.7.0`을 설치하고 Codex MCP를 등록했다. 설정 전에는 dry-run으로 변경 경로를 확인했고, 설정 백업과 REA 외 항목 비교를 남겼다.

CLI 버전, doctor, 별도 MCP의 initialize→tools/list→binary_session을 확인했다. 서버는 REA6.7.0과 도구207개를 반환했다. 당시 대화에 REA 도구가 직접 노출되지는 않아 등록 설정을 읽는 별도 MCP 클라이언트와 CLI로 분석을 계속했다.

Ghidra에는 Homebrew JDK21.0.12.1을 연결했다. Ghidra12.1.4 배포본의 SHA-256을 확인했으며, Mac ARM64용 decompile 실행파일은 포함 소스로 빌드했다. 호스트 아키텍처와 맞는 실행파일인지 doctor로 확인했다. 기존 JBR17과 시스템 Java 선택은 유지했다. REA 서버에만 Ghidra·JDK·실행 경로를 지정했다.

직접 만든 작은 ARM64 Mach-O 함수로 디컴파일을 시험했다. 첫 선택자 `rea_probe`는 실패했다. `nm`으로 실제 Mach-O 이름 `_rea_probe`를 확인한 뒤 다시 실행해 소스의 분기·산술과 대조했다. 이후 실제 원작의 충돌 함수 `0x5a88`을 MCP로 디컴파일했다.

이 준비를 마친 뒤 원본 함수 분석과 증거 저장을 진행했다.

## 3. 원본 식별·추출과 두 판본 비교

Codex는 내가 지정한 Heavy Mach1.13과 Assault를 모두 조사했다. Assault의 실제 앱 버전은1.0.3이며 아카이브 이름의 Clutch2.0.4는 도구 표기였다.

| 대상 | 구조 | 실행파일 SHA-256 |
|---|---|---|
| Heavy Mach1.13 | ARMv6·32비트 Mach-O | `97166198d701dd929243196a85e81182d7716915cdd50dec2acdc1918d10deeb` |
| Heavy Mach Assault1.0.3 | ARMv7·32비트 Mach-O | `37827b677f739c48a55b6d4c5362f61fccbe92bf713727e9f0526e4eff779762` |

원작 IPA의 SHA-256은 `477c672f92110c6020be3f744dfa7a3456739e6f970c66715e3b7b3cd32b7776`, Assault IPA는 `71368f0167061e629deb2db2d7a53b7338e03239740caa2e5771a9ac77fb00a8`이다. 내려받은 파일의 SHA-1도 아카이브 메타데이터와 대조했다. 원본 분석 전후 해시는 유지됐다.

REA의 artifact 목록과 추출 결과를 읽고, 임시 추출물을 별도 보관 위치로 복사한 뒤 파일별 해시를 비교했다. 패키지 추출 기록은779개, 원작 앱 내부 일반 파일 비교는776개로 집계 범위가 다르다. 원작에는 이미지695개, WAV31개, 지도43개, MCH3개가 있었다.

두 판본의 같은 상대 경로 파일744개는 바이트까지 같았다. 같은 이름의 변경 파일15개, 원작 전용17개, Assault 전용48개를 분리했다. 이미지674개와 WAV31개는 같았고 WeaponInfo160개·PartsInfo294개 레코드도 같았다. 맵7개, Boss08 부품, 터치, Life 구매, Game Center·광고에는 차이가 있었다.

Assault는 보조 비교 자료로 사용했다. 원작 reader에서 불명확한 구조를 같은 데이터 형식과 Assault reader로 대조하되, Assault의 Life·결제·멀티터치·후반 적 배치를 원작 기획에 섞지 않았다. 구매→Life20 지급→저장, 계속하기의 Life1 소비는 정적 호출 경로까지 조사했다. 실제 결제·복원 서비스 실행은 이 작업에 포함되지 않았다.

## 4. 실행한 REA 도구

보존된 JSON을 다시 색인한 결과는 [inventory-summary.json](rea/inventory-summary.json)에 있다. 분석 폴더, 최초 아카이브 분석, 타이틀 추가 분석, 자원 Evidence의 네 범위를 읽었다. 현재 색인에는 JSON2,067개와 고유 Evidence ID1,872개가 있다. 같은 Evidence가 여러 파일에 보존된 경우 ID로 묶었다. ID 수와 도구 호출 횟수는 다르다.

| 보존된 operation | 고유 Evidence 수 | 사용 목적 |
|---|---:|---|
| inspect_artifact | 2 | 패키지 구조와 산출물 관계 확인 |
| inventory_artifact | 2 | 두 IPA 파일 목록·해시 확인 |
| extract_artifact | 2 | 실제 파일 추출 |
| inspect_plist | 1 | 앱 버전·실행파일 등 메타데이터 확인 |
| binary_overview | 2 | 네이티브 대상 개요 |
| read_binary_inventory_summary | 1 | 분석 목록 요약 |
| list_procedures | 2 | 원작·Assault 전체 함수 목록 |
| list_strings | 2 | 정의 문자열 목록과 기능 단서 |
| search_procedures | 1 | 특정 기능 함수 찾기 |
| search_strings | 1 | 기능 관련 문자열 찾기 |
| analyze_function | 1 | 선택 함수의 분석 정보 |
| get_objc_classes | 1 | Objective-C 클래스 정보 |
| get_objc_protocols | 1 | 프로토콜 정보 |
| procedure_pseudo_code | 951 | 함수 의사 코드. 시험 함수·두 판본·추가 분석 포함 |
| procedure_assembly | 902 | 실제 ARM/Thumb 명령 대조 |

`open_binary`, `binary_session`, `close_binary`, MCP initialize/tools/list, CLI doctor/setup은 세션·환경 기록으로 별도 보존했다. `inspect_macho` 실패 등 Evidence ID가 생성되지 않은 결과는 [failed-results.csv](rea/failed-results.csv)에 연결했다. [artifact-index.csv](rea/artifact-index.csv)는 모든 대상 JSON의 상대 이름·해시·연결 Evidence를 제공한다. [evidence-index.csv](rea/evidence-index.csv)는 ID별 operation·대상 SHA·보존 위치를 제공한다.

## 5. 함수 목록과 분류

원작 목록의1,133개 항목은 함수 본문·thunk·외부 import로 나뉜다.

| 분류 | 수 | 의미 |
|---|---:|---|
| 파일에 본문이 있는 함수 | 658 | 게임 함수와 함께 런타임·오디오·컴파일러 코드 포함 |
| 파일에 있는 thunk | 186 | 다른 함수로 연결하는 작은 진입 코드 |
| 분석기가 만든 외부 thunk | 56 | 파일 본문 없는 합성 항목 |
| 외부 import | 233 | 외부 라이브러리 함수 참조 |

총900개 로컬 목록 항목을 내보내려 했고, 합성 thunk56개에서는 어셈블리 본문이 없다는 결과가 남았다. 이56개는 별도 실패 목록에 보존하고 파일 주소 매핑으로 분류했다.

[original-procedures.csv](rea/original-procedures.csv)에1,133개 전부의 이름·주소·분류·함수군·의사 코드/어셈블리 상태와 Evidence ID를 넣었다. [assault-procedures.csv](rea/assault-procedures.csv)는 비교 판본의1,802개 목록을 담는다.

`export-rea-all.py`는 입력 SHA를 확인하고 같은 원작 Evidence만 재사용한다. 함수별 JSON과 읽기용 `.pseudo.c`·`.asm`을 분리하고 진행 중 manifest를 기록해 중단 후 이어갈 수 있게 했다. 외부 import에는 본문을 만들지 않는다. 내보낸 의사 코드는 함수 동작을 읽는 분석 자료로 사용했다.

## 6. Objective-C·ARM32 구조 분석

REA 결과와 원본 Mach-O 바이트에서 Objective-C 클래스37개, 메서드553개, ivar354개를 복원했다. 디스패치 메타데이터의 이름·타입 인코딩·원래 주소·Thumb 비트를 보존했다. 원본 목록의 이름 탐지557개와 메타데이터 선언553개는 수집 방식이 달라 동일 집계로 합치지 않았다.

`export-rea-metadata.py`는 segment/section, 가상 주소→파일 offset, class_ro, method/ivar/property 목록을 읽었다. superclass가0으로 보인다고 NSObject로 추정하지 않고 `otool -rv`의 외부 relocation으로 연결했다. EAGLView의 외부 부모는 UIView였다.

`export-rea-headers.py`는 OriginalClasses/OriginalTypes 선언과 ARM32 디스크 레이아웃을 생성했다. 선언만 있는 메서드를 빈 몸체로 채우지 않았다. 원작32비트 offset은 분석 근거이며 ARM64 객체 메모리에 그대로 적용하지 않는다. 저장 구조는 포인터 없는 고정 크기 레코드로 분리했다. 무기 항목28바이트, 슬롯1,208바이트, 전체 저장3,624바이트를 정적 단언과 실제 저장 검사로 연결했다.

## 7. 데이터와 자원 복원

MAP/MCH를 `declared_size/type/data_size → payload → child_count → children` 형태로 읽었다. 두 판본의92개 파일을 끝까지 파싱하고 잔여0바이트를 확인했다. 선언 크기가 실제 하위 길이와 다른 사례가 있어 해당 숫자만으로 다음 노드로 건너뛰지 않았다.

WeaponInfo160개, PartsInfo294개, MachInfo720개를 원작 reader·setter와 대조했다. 무기 속도·분산·연사·중력·수명·효과 번호, 부품 socket과 atlas, 기체 HP/EXP/타입·재귀 부품, 지형 선분·배치 적을 각각 typed data로 연결했다. effectTileNo처럼 의사 코드에서 빠진 값은 어셈블리 load와 호출 인수에서 확인했다. 원작 reader의 불완전한 switch를 보조 판본으로 해석한 부분은 별도로 남겼다.

이미지·WAV 원본과 비교용 파생본을 구분했다. CgBI/alpha, premultiplied 색, POT 패딩, atlas, RGB565 변환과 샘플링 규칙을 복원했다. Icon은 `.png` 확장자로 저장된 JPEG여서 파일 헤더를 읽어 디코더를 선택했다. 현재 제품은 OriginalImages를 소비하고, 원본695개 파일은 IPA와 SHA가 같다. 메뉴 확장 그림은 별도 Extensions 자원이다.

## 8. 함수군 25개의 구현 범위

아래는 당시 source-coverage의 모든 함수군이다. 원문 상태·대표 주소·목록 연결은 [family-audit-snapshot.json](rea/family-audit-snapshot.json)에 보존했다. 그 파일의 CADisplayLink·AVAudioPlayer·CPU 렌더와 미검사 문구는 당시 상태이며 후속 변경은9~12절을 따른다.

| 함수군 | 복원·대체한 책임 | 주요 소비 코드 |
|---|---|---|
| geometry | 벡터·선분·sweep·충돌 반경·입력 보존 | Geometry |
| data_records | 무기·부품·기체·지도 reader | GameData |
| global_progress_save | EXP·레벨·인벤토리·진행·저장 | GlobalInfo, SaveData, GameSession |
| machine_motion | 지상/공중 이동·부스터·기체 부품·출구 | Motion, GameSession |
| player | 피격·습득·강화·터치·패시브 | Player, GameSession, HUD |
| enemy | 적 생성·AI·파괴·드롭·EXP | Enemy, GameSession |
| combat | 재장전·조준·연사·탄환·피해·효과 | Combat, BulletVisual |
| particles_parts | 파티클과 부품 좌표·애니메이션 | Visuals |
| trails | 잔상·레이저 mesh·감쇠·바람 | Trail, 렌더 연결 |
| items | 아이템 튕김·획득·Survival 흡인 | ItemBox, Player |
| world | 맵·카메라·보스 경계·결과·Survival | World, GameSession |
| title | 타이틀 시간·변환·건너뛰기·조건부 RNG | TitleState, HMGameView |
| menus | 슬롯·모드·미션·미리보기 | HMMenus |
| hud | HP/EXP/장전·강화·메뉴·결과 | HMGameHUD |
| screen_routing | 닫힘→화면 교체→열림·음향 순서 | LoadingTransition, HMGameView |
| camera_quake | 화면 흔들림·가시성·조준 연결 | CameraShake, GameSession |
| base_visual_font | 경계·자식 순서·이미지·숫자 glyph | 전용 소비 코드로 부분 대체 |
| buttons | 눌림·해제·이동량·입력 상태 | HMButtonState, 메뉴/HUD |
| containers_dispatch | 목록·레이어·입력 전달 | C++ 컨테이너와 전용 전달 경로 |
| graphics_resources | 텍스처·tile·blend·최종 표시 | HMMedia, HMGLRenderer |
| audio | 원본31음원·재시작·감쇠·거리 | HMOpenALAudio, AudioContract |
| ios_platform | 창·프레임·센서·수명주기 | Application, HMGameView |
| linked_runtime_audio | 링크된 오디오·STL 런타임 식별 | 원작 목록 보존, 필요한 호스트 구현 재사용 |
| compiler_entry_helpers | ARM 진입·switch helper | 현대 컴파일러/플랫폼 진입으로 대체 |
| analysis_fragments | 분리된 익명 코드 조각의 소유자 연결 | Combat·Enemy 등 해당 함수 |

앱에서 필요한 행동을 전용 C++/iOS 소비 코드로 연결했다. 코드가 있는 범위, 연결된 범위, 검사한 범위는 당시 감사 JSON의 ported/wired/tested/remaining 필드로 구분했다.

## 9. 디컴파일 결과 해석 오류와 수정

| 문제 | 추가 확인 | 코드에 반영한 내용 |
|---|---|---|
| CGPoint·CGRect와 생략된 인수 | ARM 호출 레지스터·어셈블리·타입 인코딩 | float로 잘못 보이는 state/인수와 구조체 반환을 실제 ABI에 맞게 해석 |
| 부동소수 계산 순서 | 명령 순서·bit pattern·경계 입력 | binary32 중간 반올림, `-ffp-contract=off`, signed zero·NaN 처리 보존 |
| EXP의 고레벨 변환·합산 | VFP 변환 helper와 호환 실행기 의미 | 정수 포화·32비트 wrap·한 호출당 최대1레벨·gauge 경계 반영 |
| 데미지 강화 | `tickBullet 0xa444` | `D+(D/8)*G`의 정수 나눗셈 순서, Attack의 float1.5 후 절삭, Defense의 정수 절반 |
| 화면 밖 탄환·부품 | 등록·가시성·draw의 부작용 비교 | 충돌 가시성과 시각 등록을 구분하고 RNG·효과 갱신 호출을 복원 |
| HUD 이벤트 순서 | HP/장전/강화/메뉴 원본 함수 | World 이벤트를 HUD tick 전에 전달하고 HUD를 화면 재진입에도 유지 |
| Survival 시계 | 원작 World의 객체 수명·무기 시간 비교 | 세션 교체 때 world time 유지, 메뉴 중 시간 추가 없음 |
| 최종 미션 진행 | setNextStage와 결과 버튼 분기 | 정상35맵과 데이터만 존재하는 직접 지정6맵을 구분하고 마지막 clamp 보존 |
| nil BaseParts | 원본 ARM 실행의 반환 버퍼·누적 경계 관측 | nil을 무조건0구조체로 설명하지 않고 관측 표본과 실제 계산 결과를 비교 |

REA 어셈블리로 위치를 정한 뒤 touchHLE의 GDB RSP에 전용 관측 도구를 연결했다. 별도 관측 앱에서 Stage01을 정상 UI로 시작하고, 한 번에 하나의 trap을 설치·원복했다. 첫 연결은 시작 `+` 인사 누락으로 실패했고 이를 수정했다.12행의 getter 전후 버퍼와15행의 root 누적 관측을 얻었으며, 세 표본의 root 경계는 `(-20,0,20,40)`이었다. 기체720종의 이름별 최종 경계 일치는 아직 별도 범위다.

## 10. 호환 실행판 비교와 버그 수정

REA는 원본의 규칙을 읽는 근거를 제공했다. Codex가 처음 만든 호스트 구현에는 원본과 다른 선택이 있었다. 내가 정상 IPA 호환판과의 차이를 비교하라고 반복해서 지시했고, 그 비교가 후속 수정의 기준이 됐다.

초기 UIKit/CPU 그림 합성은 슬롯·사격 표본에서 평균20.187ms·19.506ms를 기록했다. 게임 표시를 OpenGL ES1.1로 복원하고 원본 텍스처·blend·UV·자원 수명을 맞췄다. 음향 재시작의 AVAudioPlayer 호출은 한 측정에서 평균52.887ms였고, 호환판 OpenAL Soft1.22.2를 재사용한 뒤 같은 호출 비용이0.007053ms가 됐다.

시간 누적 보충 계산은 한 표시 프레임에서0회 또는2회 게임을 진행시킬 수 있었다. 원작 renderScene의 고정 상수와 호환판 예약 규칙에 따라 한 프레임 한 tick·일회용 NSTimer 기한 계산으로 맞췄다. 탄약·피격·EXP 변경을 파일 저장에 연결한 오류는 원작 네 저장 지점으로 제한했다.180프레임 사격 추가 쓰기0회, 메뉴 복귀1회를 확인했다.

시뮬레이터에서 Time Profiler·Allocations/Leaks·Metal System Trace를 수집했다. CPU 표시 비용과 게임 tick을 분리했고 누적 할당과 살아 있는 메모리를 구분했다. 실제 아이폰의 별도127.77초 기록은 주 화면 갱신59.71회/초, p95 간격16.67ms였다. 그 구간의 열 상태는 Serious였고 장시간 발열 비교는 후속 과제다.

화면·입력·범위 수정에서도 원본 기준을 사용했다. 틸트 반전 보정을 복구했고, 누락된 무기 전환음 호출과 JPEG 이름을 바로잡았다. 승인된 넓은 캔버스에서 필살기 해제의 가짜 좌표, 클리어 그림 밖 월드 노출, 항공기 보스의 조기 활성, 메뉴 경계의 검정 텍스처 여백 혼입을 수정했다.

## 11. 앱 구현·빌드·설치

편집 가능한 C++ 코어, Objective-C++ 호스트, CMake 검사와 Xcode target을 직접 작성했다. 소스 앱은 원본 ARM 명령을 실행하는 호환판과 별도다. 원본 ARM 호환 실행기는 비교 기준으로 보존했다.

일반·치트 앱은 다른 bundle과 Documents를 사용한다. 치트 패널은 무기·부품·EXP·미션·시작 위치·방향 입력을 설정하되 실제 게임의 이동·사격·성장·저장 함수를 호출한다. 조건 설정과 게임에서 발생한 결과를 구분한다.

Xcode 서명, 아이폰 설치, 잠금 상태, 원격 화면 조작의 인증은 다른 단계였다. CLI 서명 오류가 있었어도 Xcode GUI에서 서명 빌드를 성공한 경로를 사용했다. 앱 설치 전에는 기존 저장을 백업했다. 별도 도구 서버의 오래된 버전 때문에 생긴 화면 조작 장애는 공식 갱신 후 실제 아이폰 터치까지 확인했다. 이 복구는 REA 분석 엔진의 성능과 관계없는 실행 환경 작업이다.

내 승인 이후 추가한 넓은 시야, 자동 일시정지·120초 미사용 정지, 치트 방향 입력, 생성 배경은 원작 복원과 구분해 설계했다. imagegen으로 만든 좌우 그림은 중앙 원본과 합성하고 실제 앱에서 확인했다.

## 12. 검사 결과

| 층위 | 실행 근거 | 확인한 결과 |
|---|---|---|
| 원본 보존 | IPA·실행파일·자원 해시 | 입력과 원본 자원 보존 |
| 정적 분석 | 함수 의사 코드·어셈블리·원시 메타데이터 | 규칙·자료형·호출 순서의 근거 |
| 코어 | 실제 데이터·단위/통합·ASan/UBSan 검사 | 기하·전투·진행·저장과 특정 경계 동작 |
| 원본 실행 비교 | touchHLE 표본·동일 상태·RNG/저장/draw 비교 | 정상 기준과 차이 분리 |
| 실제 iOS 소비자 | HMGameView·HMMenus·HUD·GL XCTest | 입력→게임→화면 연결과 회귀 |
| 시뮬레이터 화면 | 직접 메뉴·전투 조작과 GL 캡처 | 표시·버튼·확장 영역 동작 |
| 아이폰 자동 검사 | 전체20무기360조건·1,080발, 최종 메뉴26/26 | 정한 조건의 목표·연사·탄약·배경 검사 |
| 아이폰 직접 조작 | 이동·점프·사격·전환·메뉴·정지/재개 | 실제 조작 경로 |
| 직접 한 전체 미션 플레이 | 현재 설치본 내 플레이 기록 | 크래시·오동작·적 이동 이상 미관측 |

과거 보스48개 처치 검사, 정상 조건의 독립35맵 검사, 나머지10맵 치트 검사, 최신 직접 한 전체 미션 플레이는 서로 다른 조건의 기록이다. 자동 조작 중 사망·시간초과와 실제 제품 오류를 구분하고 각 실행 결과를 보존했다. 모든 함수·플랫폼·픽셀·오디오 출력의 완전한 원본 동등성 검사는 남아 있다.

## 13. 분석·실행 오류와 처리 결과

| 실패/미확정 | 원인 또는 확인 범위 | 처리 |
|---|---|---|
| 스킬만 설치 | 서버·엔진과 별개 | REA 본체/등록/엔진을 각각 준비 |
| 현재 대화에 도구 미노출 | 등록 성공만으로 세션 연결이 생기지 않음 | CLI와 별도 MCP 사용 |
| Mac ARM64 decompile 부재 | 배포본에 해당 실행파일 없음 | 포함 소스 빌드·호스트 검사 |
| `rea_probe` 조회 실패 | 실제 심볼은 `_rea_probe` | nm으로 이름 확인 후 재실행 |
| `inspect_macho` 실패 | native macOS dyld_info가 구형 ARMv6에 unknown platform | 실패 보존, Ghidra·otool·원본 바이트 파서 사용 |
| Assault setFire 의사 코드 실패 | Overlapping input varnodes | 어셈블리 보존, 해당 완전 동등성 미확정 |
| 합성 thunk56개 본문 없음 | 파일에 코드가 없는 분석기 항목 | 함수/파일 offset 분류에 반영 |
| 원작 reader switch 불완전 | 일부 타입 의미가 의사 코드에서 부족 | 동일 파일 구조·Assault reader 대조, 추정 표시 |
| ARM ABI·nil 구조체 반환 | 의사 코드만으로 실제 인수/버퍼 확정 불가 | 명령·메타데이터·원본 실행 표본 대조 |
| 픽셀 차이·16.67ms 성능 기준 실패 | 렌더/표시 단계와 환경별 비용 차이 | 최초 차이·원시 결과 유지, GPU/오디오/시간 경로별 수정 |
| 자동 조작 사망·전환 정지 | 조작 전략·하네스 순서와 제품 조건 혼재 | 실제 HUD 순서·출구/보스 조건 대조, 원인 없는 난이도 변경 금지 |
| CLI 서명·폰 잠금·CUA 인증 | 빌드/설치/실행/조작의 서로 다른 실패 | 성공 경로 확인, 저장 보존, 공식 환경 복구 |

## 14. 전체 색인과 재현 방법

본문은 방법과 결과를 설명하고 부록은 개별 항목을 찾는 데 사용한다.

- [원작 함수1,133개](rea/original-procedures.csv): 주소·분류·함수군·export 상태·Evidence.
- [Assault 함수1,802개](rea/assault-procedures.csv): 비교 대상 전체 목록.
- [Evidence1,872개](rea/evidence-index.csv): operation·대상 해시·보존 위치.
- [JSON 산출물2,067개](rea/artifact-index.csv): 상대 이름·SHA·Evidence 연결.
- [실패 응답 색인](rea/failed-results.csv): Evidence ID가 없는 실패도 포함.
- [함수군25개 당시 감사](rea/family-audit-snapshot.json): 구현·연결·검사·당시 미확정 범위.
- [관련 문서 전체 목록](rea/document-index.csv): 조사·구현·실행 기록을 파일 단위로 추적.
- [집계 기준과 수치](rea/inventory-summary.json): 중복 제거와 입력 범위.

공개 부록은 주소·이름·해시·연결 메타데이터다. 원시 분석 파일은 기존 로컬 증거로 보존했다. 분석 재실행에는 같은 해시의 입력 파일과 당시 도구 환경이 필요하다.

부록 생성기는 `HeavyMach/tools/export_rea_public_report.py`다. 기존 증거 위치를 `--archive-root`, `--title-root`에 전달하면 메타데이터만 다시 만든다. 새로운 디컴파일이나 앱 실행은 하지 않는다. 새 분석이 필요할 때는 원본 SHA를 먼저 확인하고 별도 출력 위치에 저장한다.

후속 작업은 게임 기획의 요구 ID, 분야 정책, 소비 코드, 검사와 앱 관측을 함께 갱신한다. REA로 함수·데이터를 분석했고, 분석 결과를 바탕으로 앱 코드를 작성하고 검사했다.
