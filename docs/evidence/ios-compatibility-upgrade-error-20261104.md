# 업그레이드 화면의 호환 실행 오류 — 2026-11-04

사용자가 iPhone Air에서 메인게임 진입·유닛 생산·적과의 교전을 확인했다. 업그레이드를 조작하던 중 호환 실행 오류 창이 표시됐다고 보고했다.

## 원인

아이폰 앱의 Documents를 읽어 로그와 MachDefense.sav를 로컬에 보존했다. 로그의 첫 실패는 `src/libc/stdio/printf.rs:321`, `assertion failed: !prepend_sign`이다. 호출 경로는 NSString 문자열 생성 → printf_inner다.

원본 ARMv7 실행 파일의 복귀 주소 `0x3f7f3`와 문자열 객체 `0x5cf80`을 대조했다. 원본 형식은 `%@ : %d(%+d)`다. 업그레이드 수치와 증감량을 함께 표시하는 문자열에서 `%+d`를 사용한다. 호환 실행기는 필드 폭이 없는 정수의 + 부호를 구현하지 않고 assert로 중단했다.

## HeavyMach 1과의 차이

수정 전 printf.rs는 HeavyMach 1에 보관된 호환 실행기 파일과 바이트 단위로 같았다. 두 파일 SHA-256은 `8c15bb470ec0e9519ba9abb64d4c0850234e1e2966d0a3cecd482df360986737`이다. MachDefense 포팅 중 이 파일을 변경하지 않았다.

두 포팅판은 같은 upstream 커밋 `ebf9433e2bfd32071545bf49484b1345d15522a3`, SDL 커밋 `b67f98fe6a147773b6f05d50ae775d9fe16561e7`, Rust 1.99.1, `static,cpu_interpreter` 기능을 사용한다. MachDefense는 원본의 세로 화면을 사용하며 HeavyMach 1의 landscape-right 옵션을 넣지 않았다.

MachDefense에서 추가한 호환 처리는 광고 비표시, 원본 표 해독용 Darwin 난수, UIScreen 프레임 갱신 API, NIB 첫 창 표시, 원본 이름 입력칸 연결, 이미지 합성 및 빈 이미지 처리다. 원본 게임 바이너리와 에셋은 변경하지 않았다.

이번 실패는 기존 호환 실행기에 남아 있던 정수 문자열 기능 누락이다. HeavyMach 1에서 같은 호출을 실제로 실행했는지는 확인하지 않았다. 한 게임의 실행 성공만으로 다른 게임이 요구하는 모든 OS API와 인수 조합의 지원을 확인할 수는 없다.

## 수정과 검사

정수의 부호·출력 폭·정밀도를 함께 처리하도록 수정했다. 양수·음수·0, 정밀도·공백·0 채움의 320개 조합을 Darwin snprintf와 대조해 일치했다. `Compatibility/check-decimal.py`가 실제 런타임 함수 본문을 컴파일해 검사한다. 시뮬레이터 빌드 14와 iPhone Air 빌드는 성공했다.

iPhone Air에서 기지 업그레이드와 네 유닛의 상세 화면을 직접 열었다. 증감값 표시, 유닛 선택 전환, 닫기 동작이 정상이며 호환 오류가 재발하지 않았다. 세 유닛을 생산하고 교전을 확인했다. 게임의 Save & Exit Game으로 저장한 뒤 프로세스를 다시 실행했을 때 기존 슬롯 `1234`, 점수 106, 34 Cr가 유지됐다. 이후 로그에 `assertion failed`, `panicked`, `Unimplemented`가 없었다. 실제 강화 구매는 아직 검사하지 않았다.

## 빌드 서명 재발과 수정

첫 수정본은 Xcode 빌드가 성공했지만 실행 파일에 서명이 없어 설치가 실패했다. 앞서 IPA·설정 파일만 스크립트 출력에 선언한 조치로는 실행 파일을 교체하는 의존 관계를 보장하지 못했다.

Stub 링크 단계를 제거하고 패키지 스크립트를 실행 파일의 유일한 생성자로 바꿨다. 스크립트 출력에 `$(TARGET_BUILD_DIR)/$(EXECUTABLE_PATH)`를 명시했다. 이후 두 번의 증분 빌드 모두 CodeSign 실행과 `codesign --verify --deep --strict` 검사를 통과했다. 두 번째 설치와 잠금 해제 후 실행도 성공했다.

원본의 업그레이드 수치·가격·게임 규칙·저장 형식은 수정하지 않았다.

## 로컬 근거

- `.local/device/upgrade-error-20261104/Documents/touchHLE_log.txt`: 아이폰 원본 실패 로그.
- 같은 폴더의 touchHLE_sandbox 아래 MachDefense.sav: 42,476바이트 저장 사본. SHA-256 `e26adafc396ffa1130e613b8bb71b934288c0a46e25ce8e8af7959d2ca1c5396`.
- `.local/device/upgrade-error-20261104/printf-comparison-before.json`: 수정 전 파일 대조.
- `.local/simulator-build-14.log`, `.local/simulator-upgrade-after.log`: 수정본 빌드·실행.
- `.local/device/upgrade-error-20261104/install-fixed.json`: 서명 누락 설치 실패.
- 같은 폴더의 `install-fixed-2.json`, `launch-fixed-2.json`, `relaunch-after-save.json`: 수정 후 설치·실행·재실행 성공.
- 같은 폴더의 `after/Documents`: 검사 후 로그와 저장 사본. 저장 파일 SHA-256 `4f6a0592ff00d7a8375330f4d3b5c553ce037736d18d2444abfd44f603e51ce9`.

사용자 데이터와 원본 실행 파일은 Git에 넣지 않는다. 다음 단계는 [복원 계획](../restoration-plan.md)의 포팅판 완료 조건을 따른다.
