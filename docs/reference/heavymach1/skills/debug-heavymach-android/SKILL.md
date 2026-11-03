---
name: debug-heavymach-android
description: HeavyMach Android 일반·치트 APK를 USB 또는 Wi-Fi로 설치하고 실행·입력·로그·종료를 검사한다. Apple 빌드와 게임 규칙 변경에는 적용하지 않는다.
---

# Android 기기 실행

`Android/tools/device.py`와 `Android/README.md`를 사용한다. 앱 패키지는 일반 `com.gelb2.heavymach.restored`, 치트 `com.gelb2.heavymach.validation`이다. 둘의 저장 공간을 분리한다.

1. `python3 Android/tools/device.py devices`로 USB·무선 기기를 확인한다. 이후 모든 명령에 `--serial`을 넣는다. `unauthorized`이면 휴대폰의 USB 디버깅 RSA 승인, 목록이 비면 데이터 케이블·USB 모드·개발자 옵션을 확인한다. ADB 재시작을 반복하거나 Mac 보안 설정을 바꾸지 않는다.
2. `--serial SERIAL diagnose`로 모델·OS·ABI·페이지 크기를 기록한다. 기기가 잠겼으면 실행 검사 전에 잠금을 해제해야 한다. APK 설치와 화면 실행 성공을 구분한다.
3. Android 11 이상 무선 디버깅은 휴대폰의 개발자 옵션에서 켜고 `pair IP:PAIRING_PORT`, `connect IP:DEBUG_PORT`를 사용한다. 페어링 포트와 연결 포트는 다를 수 있다. 페어링 코드는 도구의 비표시 입력에만 넣는다. USB 승인이 끝난 기기는 `--serial USB_SERIAL wireless-usb`도 지원한다. 두 연결의 `ro.serialno` 일치를 검사한다.
4. 최초 구현은 전체 연결을 작성한 뒤 빌드한다. 빌드 이후 오류 수정 범위를 Android 안으로 제한한다. `check_isolation.py`로 기존 Apple·Core·자원 해시를 검사한다.
5. APK 패키지·서명·SHA-256·ABI를 확인하고 `install APK`, `launch --edition normal|cheat`를 실행한다. `-r`로 저장을 유지한다. 설치 실패를 풀려고 앱 데이터 삭제·언인스톨·다운그레이드를 임의 실행하지 않는다.
6. 치트 앱의 단계 선택 후 이동·점프·수동 사격·전체 무기·자동조준·HUD 이동·일시정지·복귀를 검사한다. CPU/GPU 측정은 한 에뮬레이터 또는 한 실기기에서 진행한다. 키 입력·터치·센서 경로와 저장 횟수를 함께 확인한다. 앱 내부 치트 상태만 보고 터치 성공으로 판정하지 않는다.
7. 홈 이동·잠금·오디오 포커스 상실·가로 반전·Surface 재생성을 검사한다. 진행 상태 유지, 입력 해제, 명시적 재개, 음향 복귀를 확인한다. EGL 자원 재생성과 게임 세션 재시작을 구분한다.
8. `capture`, `logs`, `dump`로 증거를 `Android/build/evidence/`에 남긴다. 검사 후 `finish --edition ...`로 검사한 앱을 종료하고 홈으로 돌아간다. 다른 앱을 종료하지 않는다. 무선 성공은 케이블을 뺀 뒤 설치·실행·로그 수집이 모두 끝났을 때 기록한다.

ADB와 Wi-Fi 연결: https://developer.android.com/tools/adb#wireless
기기 실행: https://developer.android.com/studio/run/device

## 확인한 연결 오류

2026-10-30 Galaxy SM-S731N에서 휴대폰의 5555 포트가 열리고 새 TCP 연결도 되는데, 기존 ADB 서버만 `No route to host`를 반환했다. `adb server-status`와 실행 시각을 확인한 뒤 서버를 한 번 재시작하자 연결됐다. 같은 오류가 나면 USB 장치·IP·포트·일반 TCP 연결을 먼저 비교한다. TCP도 실패하면 서버 재시작을 반복하지 않는다. 이 경우 Wi-Fi 경로를 확인한다.

USB 장치 조회는 `ioreg -r -c IOUSBHostDevice -a`를 파싱한다. ADB 목록이 비어 있는 것과 USB 하드웨어 자체가 보이지 않는 것을 구분한다. `device.py diagnose`는 기기 모델·OS·페이지 크기·현재 배터리 값·잠금 상태만 기록한다.

## 검사 명령

- `python3 Android/tools/validate_runtime.py --serial SERIAL --weapons`: 이동·점프·확장 터치·저장·복귀와 20종 무기.
- `python3 Android/tools/validate_platform.py --serial SERIAL --hud-only`: 2340×1080 기기의 HUD 드래그·저장·취소·초기화·재실행·GPU 재생성.
- `python3 Android/tools/validate_platform.py --serial emulator-5554`: 전용 에뮬레이터에서 위 검사와 두 가로 방향·태블릿 화면비. 화면 크기와 회전 설정을 검사 후 복원한다.
- `python3 Android/tools/validate_tilt.py --serial emulator-5554`: 양쪽 가로 방향에서 센서 입력에 따른 전진·후진. 회전 설정을 검사 후 복원한다.
- `python3 Android/tools/validate_audio_timer.py --serial emulator-5554`: 전용 에뮬레이터의 Clock 타이머를 만들어 실제 오디오 포커스 중단·복귀를 검사한다. 만든 타이머는 해제한다.
- `python3 Android/tools/validate_campaign.py --serial SERIAL`: 캠페인 35스테이지 진입, 보스 구간 자동조준, Android 키 입력.

검사 명령은 치트 앱의 shell 권한 전용 BroadcastReceiver를 사용한다. 검사를 실행할 때마다 Activity를 재실행하면 입력·생명주기 결과를 바꾸므로 반복 명령에 `am start`를 쓰지 않는다. 진단 JSON은 매 요청 직전 지우고 새로 생성된 파일만 읽는다. 앱이 죽었는데 이전 JSON으로 통과시키면 안 된다.

`wireless-usb`의 TCP 연결은 USB 승인된 기기를 같은 LAN에서 연결하는 절차다. 기기 재부팅 후에는 다시 설정해야 할 수 있다. Android 11 이상의 무선 디버깅 페어링은 `pair`와 `connect`로 별도 지원한다. 새 페어링 포트와 연결 포트를 구분한다.

## 미션 전환과 성장 검사

스테이지 진입 검사와 별도로 남은 경험치가 다음 레벨 기준을 넘는 저장에서 미션을 연다. `validate_upgrade.py --serial SERIAL --save-file LOCAL_SAVE --slot INDEX`는 이번 오류의 경험치2,415·내부레벨3 조건을 치트 앱에 복사해 검사하고 치트 저장을 복원한다. 일반 앱의 원본 저장은 먼저 별도 보관한다. 강화 선택 중 시간·체력·위치 정지, 실제 터치 선택과 OK, 강화 한 번 적용, 닫힘 후 전투 재개, 중단·복귀를 검사한다. 설치 명령에는 빌드와 같은 JDK17의 `JAVA_HOME`을 사용한다.
