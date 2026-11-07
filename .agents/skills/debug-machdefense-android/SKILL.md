---
name: debug-machdefense-android
description: MachDefense Android USB·무선 연결과 설치·실행·검사·종료에 사용한다.
---
# debug-machdefense-android

실제 adb serial과 transport를 조회한다. 기존 기기나 다른 앱을 초기화하지 않는다. 무선 페어링/연결 포트를 구분하고 비밀을 로그에 쓰지 않는다. 빌드 패키지·SHA·저장 백업을 확인해 일반/치트 앱을 각각 설치·실행·조작한다. 실패 로그를 보존하고 입력 해제·일시정지·앱 종료·홈 복귀를 검사한다. 일반 앱은 com.gelb2.machdefense.restored, 치트 앱은 com.gelb2.machdefense.validation이다. Android/README.md와 docs/evidence/android-native-20261106.md에서 현재 빌드·검사 도구를 확인한다. 게임 저장은 각 앱 files/MachDefense.sav다. 저장 이전은 기존 대상 저장을 백업하고 앱을 종료한 뒤 수행하며, 복사 후 바이트와 게임 진척을 대조한다. 명령 예시는 docs/reference/heavymach1/skills/debug-heavymach-android에서 참고하되 실제 MachDefense ID로 검증한다.

## Galaxy 기본 USB 용도와 무선 연결

사용자가 파일 전송 기본값을 요청하면 연결된 기기에 `adb -s <serial> shell svc usb setScreenUnlockedFunctions mtp`를 적용한다. `dumpsys usb`의 screen_unlocked_functions와 current_functions를 확인한다. 케이블 재연결까지 검사하기 전에는 재연결 검증 완료라고 쓰지 않는다.

무선 디버깅은 기기에 표시되는 현재 연결 포트를 쓴다. 페어링 포트와 혼동하지 않는다. USB를 연결한 채 검사한다면 IP:port를 명시해 실제 무선 transport로 설치·실행했는지 확인한다. `No route to host`는 인증 성공이나 연결 성공이 아니다. 네트워크 오류를 해결하려고 방화벽·격리·보호 기능을 임의로 끄지 않는다. ADB 전체 상태에는 키 정보가 포함될 수 있으므로 출력은 필요한 필드만 남긴다.
