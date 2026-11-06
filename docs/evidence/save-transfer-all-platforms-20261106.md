# iPhone 저장으로 플랫폼 간 복원 검사

2026-11-06. 작업 브랜치 `feat/save-transfer-all-platforms`, 기준 develop `52e1e6c`. 구현 커밋은 `61291c3`이다.

## 구현

Apple·Android·웹이 같은 MD1 코덱과 5슬롯 저장 형식을 사용한다. 슬롯 선택 화면에서 GET CODE → COPY로 내보내고, ENTER CODE → CHECK → YES로 복원한다. 원작 `SysMenu_Btn.png`, YES/NO/OK 이미지와 게임 글꼴로 버튼·안내를 그린다. 텍스트 필드·키보드·클립보드만 플랫폼에서 제공한다.

| 변경 위치 | 처리 |
|---|---|
| `Modules/SaveTransfer/FileStore.hpp` | 네이티브 복원 전 백업·pending·원자적 교체·재읽기·commit 기록·시작 시 복구 |
| `Modules/SaveTransfer/Validate.hpp` | 80개 미션을 모두 끝낸 저장도 허용 |
| `Sources/Core/Presentation/Menu.*` | 공용 복원 입력 화면, 복사·검사·취소, 재시작이 필요한 실패 안내 |
| `Sources/Core/Contracts/Game.hpp`, `Session/GameSession.cpp` | 텍스트 필드와 클립보드 요청 전달 |
| `Sources/Platform/Apple/MDApplication.mm` | iPhone·iPad UITextView와 클립보드·공용 파일 복원 연결 |
| `Android/app/src/main/` | EditText·JNI·게임 큐·클립보드·공용 파일 복원 연결 |
| `Web/src/Main.cpp`, `Distribution/Web/` | 복원 입력을 일반 textarea로 연결하고 액션 버튼은 공용 게임에 맡김 |

전투·경제·이동·무기 계산, 음향, 이미지 자원, 원본 SaveData 파일 형식은 변경하지 않았다. 기존 일반 저장 시점도 유지했다.

## 데이터와 검사

사용자의 현재 iPhone 정식 앱 저장을 읽기 전용으로 복사했다. 전체 42,496바이트를 비교하므로 프로필·미션·유닛 순서/강화·필살기 잔량·기지 강화·자금과 아직 이름을 붙이지 않은 원본 필드도 대조된다. 원본과 복원 코드는 Git에서 제외한 `.local/save-transfer/`에 보관한다.

| 검사 | 결과 |
|---|---|
| 공용 C++ | CTest 38개 통과. 기존 캠페인·전투·경제·저장 검사 포함 |
| 코드 | 5슬롯 왕복·잘못된 코드 403건·현재 iPhone 원본 왕복 통과 |
| 공용 메뉴 | 복사·취소·쓰기 실패·복원·잘못된 강화/유닛·80미션 완료·화면 크기 변경 통과. 대기 100tick 추가 저장 0회 |
| FileStore | 백업, 세 중단 지점의 재실행 복구, 완료 journal 뒤 최신 진행 보존, 손상 코드와 쓰기 실패 검사 통과 |
| Apple 시뮬레이터 | iPhone Air·iPad에서 각 2개 어댑터 검사 통과. 키보드·긴 코드·입력 영역·크기 변경·비활성·닫기·종료·파일 복구 |
| Android 에뮬레이터 | 현재 iPhone 저장 복원 후 전체 바이트 일치. 입력·잘못된 코드·취소·회전/Activity 재생성·배경 복귀·세대 번호·복사·재실행 검사 통과 |
| Web Chrome·WebKit | 정식·치트판에서 같은 iPhone 저장의 복원·재출력·미션 진입·GPU 복귀·오프라인 재실행·오래된 창 쓰기 거부·DB 실패 검사 통과 |
| Web 입력 화면 | 실제 포인터 입력, 네 화면 크기, 키보드를 고려한 높이 변경·취소·원본 유지. Chrome 클립보드 바이트 일치 |

iPhone 정식 앱에서 GET CODE·COPY·ENTER CODE·붙여넣기·CHECK·확인 취소를 직접 검사했다. 진행·무장·자금은 유지됐으며, 슬롯 최근 접속 시각만 바뀌었다. 최초 원본 백업은 그대로 보존했다.

iPad 치트 앱에서 같은 코드를 실제 복원했다. 결과 파일 42,496바이트가 최초 iPhone 백업과 일치했고, 자동 생성한 restore-backup도 복원 전 iPad 파일과 일치했다. 복원된 미션의 전투·해금 유닛 HUD·저장 후 재실행을 확인했다.

iPhone·iPad 정식 앱에 최종 공용 메뉴 수정본을 설치했다. iPad 정식 앱은 기존 저장을 백업한 뒤 GET CODE·CLOSE를 검사했고 설치 전후 파일 전체가 일치했다. iPhone·iPad 검사 앱을 종료하고 홈 화면으로 돌아왔다.

Galaxy SM-S731N(Android 17)을 USB로 연결해 최신 정식·치트 앱을 설치했다. 기존 두 앱의 저장을 먼저 백업했다. 치트 앱의 9개 복원 조건을 통과했고, 복원 직후 파일 42,496바이트가 iPhone 원본과 일치했다. 복원한 Stage 41에서 유닛 생산 1회·음향 호출 248회·강화창 표시를 확인했다. 정식 앱은 기존 Galaxy 저장으로 GET CODE·COPY·ENTER CODE·CANCEL을 검사했고 저장 SHA-256이 유지됐다.

모바일 PWA의 직접 검사는 아래 운영 배포 결과에 기록한다. WebKit 데스크톱 검사와 iPhone/iPad Safari 실기기 결과는 구분한다.

## 검사 중 수정

Android에서 UTF-8 상한을 넘는 입력을 받았을 때 이전의 유효 코드가 남던 경로를 고쳤다. 새 입력을 거부할 때 캐시를 비워 CHECK가 예전 코드를 사용하지 않게 했다.

복원 파일 교체 뒤 journal 삭제가 실패한 경우, 다음 실행이 더 최근 게임 저장을 덮지 않도록 완료 기록을 추가했다. 디스크 상태가 불확정하면 후속 저장을 차단하고 게임 화면에서 재실행을 안내한다. 백업·교체 실패 후 정상적으로 되돌린 경우에는 재시도를 허용한다.

전체 미션을 완료한 진행 값 80을 검증기가 거부하던 경계를 원본 저장 범위에 맞췄다. 코덱이나 게임 미션 계산은 바꾸지 않았다.

Android 에뮬레이터에서 새 복원 화면의 단색 덮개가 기존 슬롯 제목을 가리지 못하는 현상을 확인했다. 복원 화면에서는 슬롯 화면을 함께 그리지 않도록 수정했다. 최종 Android·Web 캡처에서 겹침이 사라졌으며 렌더러 파일은 변경하지 않았다.

## 재검사

- 공용: `cmake --build build/save-transfer -j6`, `ctest --test-dir build/save-transfer --output-on-failure`
- Web: `Web/tests/regression.mjs`, `Web/tests/transfer-ui.mjs`. 개인 저장과 코드는 검사 인자의 로컬 `iphone/` 폴더에 둔다.
- Android: `Android/tools/save_transfer_smoke.py`
- Apple: `Apple/Tests/SaveTransferAdapterTests.mm`

FileStore의 추가 실패 검사를 마쳤다. macOS의 임시 디렉터리에서 테스트 파일의 POSIX 호출을 가로채 오류를 만들었다. 제품 모듈에는 검사 코드를 추가하지 않았다. 합성 저장과 실제 iPhone 저장 복사본에서 아래 결과가 같았다.

- pending·실제 저장·commit 파일의 rename 직후 디렉터리 fsync 실패 3조건: 기존 저장으로 되돌리고 실패를 반환했다.
- 저장 교체 후 rollback의 rename도 실패: 예외를 반환하고 백업·pending을 유지했다. 재실행 시 새 저장을 완성했다.
- 성공 후 pending 삭제 실패: 이후 게임 저장이 생겨도 다음 실행이 그 진행을 덮지 않았다.
- 첫 설치에서 복원 도중 실패: 원래처럼 저장 파일이 없는 상태로 되돌렸다.

검사 근거는 `.local/save-transfer/file-store-followup-test.log`, `file-store-iphone-faults.log`다. 검사 전후 iPhone 백업의 SHA-256도 같았다.

[설계](../save-transfer-native-plan-20261106.md) · [사용법](../public/gameplay.md) · [남은 과제](../usability-todo.md)

## PWA 운영 배포

- 주소: https://gelb2.github.io/MachDefense/
- 정식 웹 빌드: `d679d7b1061097b5`
- gh-pages 배포 커밋: `86cef6e790e541cbe23cf1a83f85a23840a6c3dc`
- GitHub Pages 상태 `built`, 운영 build.json도 같은 빌드 번호로 확인했다.
- 운영 Chrome·WebKit에서 iPhone 저장 복원·재출력·미션 진입·오프라인 재실행·저장 충돌/실패 검사를 통과했다.
- Mac Chrome에서 새 PWA 설치·독립 창 실행·슬롯 화면·오프라인 재실행을 확인했다.
- Galaxy Chrome 운영 주소에서 ENTER CODE·CHECK·YES를 눌러 iPhone 저장 전체를 복원했다. INSTALL GAME의 Android 설치 절차를 완료하고 독립 PWA 창(`standalone=true`)에서 같은 저장을 확인했다.
- Galaxy PWA 안에서도 GET CODE 결과가 원본 코드와 같았고 ENTER CODE·CHECK·YES로 다시 복원한 42,496바이트가 일치했다.
- Galaxy 증거는 `.local/save-transfer/galaxy-device/`의 `smoke.log`, `chrome-restore.json`, `pwa-restore.json`과 화면 캡처에 보존한다. iPhone 설치형 PWA의 직접 복원 결과는 아직 확정하지 않았다.
- 개인 복원 코드는 브라우저 로컬 저장 검사에만 사용했다. 배포 파일 허용 목록에 개인 저장·코드는 없으며 원본 패키지 검사도 통과했다.

이 검사 직후에는 기능 브랜치에 보존했다. 후속 사용자 지시로 develop→main 병합과 v0.3 릴리스를 진행한다. [변경 이력](../../CHANGELOG.md).

## 사용자 확인 — iPhone와 iPad PWA 왕복

사용자가 iPhone 정식 앱에서 생성한 코드를 iPad 설치형 PWA에 입력해 복원한 뒤 한 미션을 클리어했다. 이어서 iPad PWA에서 새 코드를 생성하고 iPhone 앱에 재복원한 다음 스테이지 진입을 확인했다. 이 결과는 사용자의 직접 플레이 확인이다. 위 자동검사와 별도로 기록한다.

Galaxy PWA의 오프라인 새 실행·전투·재실행 검사를 마쳤다. 독립 창에서 42,496바이트를 읽고 전투 HUD·생산 버튼 7개를 확인했다. 전투 중 ticks는 545→702, 음향 호출은 13→67로 증가했고 저장 횟수는 1회로 유지됐다. 오프라인 재로딩 뒤 전체 저장 바이트가 같았다. Wi-Fi·모바일 데이터·비행기 모드는 검사 전 상태로 복구했다. 근거는 `galaxy-device/pwa-offline.json`과 전투·재실행 캡처다.

중간 검사에서 잠금 화면을 확인했다. 잠금 해제 뒤에도 검사기가 정식 빌드에 없는 치트 전용 `time` 필드를 요구해 실패했다. 정식 빌드가 제공하는 ticks·음향 호출 증가와 실제 전투 화면으로 검사 조건을 고쳤다. 제품 코드는 변경하지 않았다.

## 공개용 Quick Play 코드

사용자가 자신의 iPhone 진행도 코드를 GitHub에 공유하도록 지시했다. 2026년 11월 6일 10:39 KST에 정식 앱 저장을 새로 읽었다. Stage 42 진입 가능, 20,329Cr, 보유 유닛 12종이다. 최신 저장으로 만든 1,443문자 코드를 재해석한 42,496바이트가 원본과 일치했다.

공개 파일은 `docs/public/saves/iphone-stage42.txt`이며 [Quick Play](../public/quick-play.md)에서 사용법과 함께 안내한다. 원본 저장 파일과 검사 복사본은 `.local/save-transfer/iphone-quickplay-20261106/`에 보존한다.
