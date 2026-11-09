# Android 검사의 재실행 기록

첫 APK(923adcaaa38b6f7629a2fb395e649d8130e4c2a6b71da26c14d4d5aaef1f3b9b)의 검사용 JSON은 헬기 파츠의 uint8 parentSocket/zOrder를 문자로 내보냈다. 스카웃 Lv1을 실제 생산한 직후 JSON 파싱이 실패했다. 게임이 중단되거나 텍스처가 누락된 실패로 판정하지 않았다.

최종 숫자 변환 수정과 빌드가 겹쳤다. 컴파일러가 수정 전 파일을 읽었지만 객체 생성 시각은 소스 수정 시각보다 늦어, 후속 증분 빌드가 최신으로 판정했다. 설치 APK와 디스크 APK의 SHA256은 같아 설치 누락은 아니었다. Runtime.cpp를 다시 컴파일하고 Gradle --rerun-tasks로 패키지를 갱신했다. 새 APK는 eb42d761b5e83bddded206245940a0d8ab7a645688d2b7b5de419833f02d2eb5다.

두 번째 실행은 새 TEST 프로필에서 시작했다. 기존 사용자 저장은 별도 백업 이름과 로컬 사본으로 유지했다. 첫 실패의 로그·화면·JSON은 .local/android-aircraft-20261109/level-up-results/에 보존했다. 재실행은 별도 타임스탬프 디렉터리에 기록한다.

초기 TEST 프로필은 저장 전 상태여서 MachDefense.sav가 없었다. adb exec-out cat의 출력에 담긴 오류 문자열을 저장 바이트로 취급한 초기 검사 스크립트의 문제도 수정했다. 실제 원본 저장 백업(42,496바이트)과는 별개다. 이후에는 파일 존재, 길이, 공통 SaveData 디코더를 대조한다.
