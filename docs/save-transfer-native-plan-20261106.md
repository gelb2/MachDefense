# 복원 코드 — iPhone·iPad·Android 동시 연결 계획

2026-11-06. main `ec628e3`와 같은 트리인 develop `52e1e6c`를 기준으로 조사했다. 작업 브랜치는 `research/web-reference-save-transfer`다. 이번에는 소스 대조와 계획을 작성했다. 앱 기능은 변경하지 않았다.

## 결론과 현재 상태

HeavyMach1과 같은 구조로 구현할 수 있다. MachDefense에는 이미 **공용 MD1 코덱, 원본 5슬롯 저장, C++ 복원 메뉴**가 있다. PWA는 사용 중이고 Apple·Android는 연결이 빠져 있다. 코덱을 플랫폼별로 다시 만들지 않는다.

| 현재 파일 | 역할·상태 |
|---|---|
| `Modules/SaveTransfer/Codec.hpp` | encode/decode, PackBits·CRC32·Base64url. MD1 접두사, 입력 120,000자 상한 |
| `Modules/SaveTransfer/Validate.hpp` | 실제 자원 목록과 미션·유닛·강화·스킬·프로필 값을 대조 |
| `Sources/Core/Session/SaveData.*` | 42,496바이트, 원본 5슬롯. 슬롯 시작 0x408, 간격 0x2064. 미확인 바이트 보존 |
| `Sources/Core/Presentation/Menu.cpp` | 원작 SysMenu_Btn과 게임 글꼴로 GET CODE / ENTER CODE·복원 확인을 표시 |
| `Sources/Core/Contracts/Game.hpp` | supportsSaveTransfer 기본 false. transferText 기본 취소. 기본 restoreSave는 writeSave만 호출 |
| `Sources/Platform/Apple/MDApplication.mm` | 복원 서비스 미연결. iPhone·iPad는 이 구현을 함께 사용 |
| `Android/app/src/main/cpp/Runtime.*` | 복원 서비스 미연결 |
| `Web/src/Main.cpp` | 공용 복원 메뉴를 웹 텍스트 입력·저장에 연결 |
| `Distribution/Web/storage.js` | 복원 전 백업과 새 저장을 같은 IndexedDB 트랜잭션에 기록. revision 충돌 확인 |

현재 PWA 텍스트 입력창에는 DOM CANCEL/OK 버튼이 있다. 게임 안의 GET CODE/ENTER CODE와 다른 구현이다. 이번 연결 과제에서 입력 완료·취소도 공용 게임 버튼으로 고친다. 플랫폼별 대화상자의 OK/Cancel을 재사용하지 않는다.

## HeavyMach1에서 재사용할 구조

HeavyMach1의 `Modules/SaveTransfer/SaveTransfer.*`, `FileStore.hpp`, Apple `HMTransfer.inc`, Android `Transfer.inc`, 공용 `HMGameButtons.hpp`를 대조했다. 해당 파일들은 조사 checkout과 origin/main 사이에 차이가 없었다.

HeavyMach1은 코드 형식·검증·복원 상태 전이를 공용 모듈에, 파일과 텍스트 입력 연결을 호스트에 두었다. 복원 전 백업과 pending 파일, 재읽기 대조, 시작 시 중단된 복원 처리도 분리했다. MachDefense도 이 순서를 따른다. HeavyMach1의 3슬롯·3,624바이트 규격은 MachDefense의 5슬롯·42,496바이트 규격으로 대체한다.

같은 아키텍처는 같은 저장 코드를 뜻하지 않는다. HeavyMach 코드와 MD1 코드는 서로 받지 않는다. MachDefense의 iPhone·iPad·Android·PWA 사이에서는 같은 MD1 코드를 받는다.

## 옮길 데이터

프로필·선택 슬롯·80개 미션 진행과 클리어 횟수·누적 점수·크레딧·유닛 이름/순서/강화/생산 등록·필살기 이름/순서/강화/잔량/등록·기지 강화 6항목과 나머지 원본 저장 바이트를 함께 옮긴다.

생성·입력은 프로필 선택 화면에서만 한다. 마지막으로 저장된 게임을 옮긴다. 전투 중 셀 잔량, 이동 중 유닛, 발사체는 원본 슬롯 저장에 없는 실행 상태다. 이를 저장 규격에 임의로 추가하지 않는다. 전체 5슬롯 복원을 첫 범위로 맞추고, 단일 슬롯 선택과 향후 설정 확장은 별도 항목으로 둔다.

## 공용 처리와 플랫폼 연결

공용 모듈은 해독→전체 검증→복원 전 백업→pending 기록→저장 교체→재읽기 대조→pending 정리를 담당한다. 다음 실행 때 pending이 있으면 완료 여부를 대조해 재개한다. 기존 세션이 옛 데이터를 다시 덮어쓰지 않도록 복원 적용과 세션 재로드를 같은 절차로 묶는다.

공용 Menu가 COPY / CHECK / RESTORE / CANCEL / CLOSE를 원작 에셋·글꼴·입력 판정으로 그린다. 코드 표시·미리보기·확인·오류도 공용 화면에 둔다. 호스트는 텍스트 필드, 키보드, 선택·붙여넣기·클립보드, 파일 읽기·쓰기를 제공한다. 표시 문구는 영어다.

Apple의 NSDataWritingAtomic과 Android의 temp→fsync→rename→directory fsync 경로를 활용한다. Android는 rename 뒤 directory fsync 실패처럼 파일은 바뀌고 실패가 반환되는 경우도 있으므로, 반환값만 보고 옛 메모리를 유지하지 않고 pending과 파일 재읽기로 판정한다. 현재 복원 확인 문구에 백업 안내가 있으므로 백업을 구현하기 전에 네이티브 기능 플래그만 켜서는 안 된다.

Apple·Android 이름 입력의 기존 생명주기 요청 ID와 실행 큐를 참고하되, 이름 입력의 31바이트 한도를 복원 코드에 적용하지 않는다. 화면 종료·회전·백그라운드 복귀 후 오래된 입력 결과를 무시한다. 오디오 비동기 처리와 전투 tick의 저장 시점은 유지한다.

## 구현 순서와 완료 기준

1. develop에서 기능 브랜치를 만든다. 현재 앱과 기존 MD1 코드·5슬롯 바이트를 기준으로 고정한다. 사용자 저장은 무시된 로컬 폴더에 복사해 보관한다.
2. 공용 복원 저장 절차와 공용 게임 입력 패널을 보강한다. 기존 코덱과 원본 저장 형식은 유지한다.
3. Apple 어댑터 하나로 iPhone·iPad를 연결하고 Android JNI·Kotlin에는 OS 서비스만 연결한다. PWA 입력창의 액션 버튼도 공용으로 옮긴다.
4. 같은 입력을 네이티브와 Wasm에서 생성해 코드 일치를 검사한다. 전체 42,496바이트와 5슬롯·미확인 바이트를 대조한다.
5. 정식·치트 앱에서 아래 검사를 하고, **iPhone·iPad·Android 모두 통과한 뒤** 기능 완료와 배포를 판단한다. 한 플랫폼만 끝난 상태를 전체 완료로 보고하지 않는다.

| 검사 | 방법·기대값 |
|---|---|
| 데이터 왕복 | 빈 저장·높은 진행도·5슬롯·순서·강화·잔량을 코드로 내보내고 되받아 바이트 비교 |
| 형식 오류 | 다른 게임·지원하지 않는 버전·잘림·손상·초과 길이·잘못된 자원값을 저장 전에 거부 |
| 중단·실패 | 백업 실패·공간 부족·교체 도중 종료·재시작을 주입하고 기존 저장 보존/복구 확인 |
| 입력 | 실제 버튼 터치·복사·붙여넣기·취소·키보드 닫기·화면 전환·배경 복귀 확인 |
| 플랫폼 교차 | iPhone→iPad→Galaxy→PWA 및 역방향. 같은 코드로 같은 슬롯을 복원하고 재실행 |
| 게임 연결 | 복원 후 미션 진입·유닛 생산·강화·승리·저장, 배경음·발사음·생산음 확인 |
| 기존 기능 | 조회·복사·취소는 저장 0회. 전투 tick에 복원용 저장이 생기지 않는지 기존 프레임·오디오 검사와 대조 |

기존 SaveTransferTests·TransferMenuTests에는 왕복·잘못된 코드·취소·쓰기 실패·복원 후 재출력·100 tick 추가 저장 없음이 있다. 실제 네이티브 텍스트 입력, 복원 중 종료, 백업 재개는 추가한다.

[기존 PWA 근거](evidence/web-pwa-20261106.md)에는 아이폰 저장을 읽어 PWA에서 복원하고 원본 42,496바이트와 일치시킨 기록이 있다. HeavyMach1의 Apple/Android 버튼 검사 근거도 참고한다. 기존 기록을 새 네이티브 연결의 실행 결과로 대신하지 않는다.

[전체 웹·Tauri 계획](save-transfer-web-tauri-plan.md) · [TODO](usability-todo.md) · [문서 정본](canonical.md)
