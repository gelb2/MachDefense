---
name: port-machdefense-android
description: 검증된 MachDefense 코어를 Android에 연결할 때 사용한다. 최초 원본 규칙 분석과 구분한다.
---
# port-machdefense-android

docs/design.md, runtime-policy.md와 validation-plan.md를 읽는다. 기존 정상 Apple 코드·코어를 먼저 공통화한다고 수정하지 않는다. Android/에서 NDK 코어 연결, Kotlin OS 입력, EGL context/Surface 복구, Audio Focus, 파일 저장을 구현한다. 게임 버튼과 메뉴는 게임 렌더러에서 처리한다. Surface 소실/Activity 재생성/프로세스 종료를 분리해 검사한다. 패키지·일반/치트 저장·ABI를 확인하고 전체 교전·진행·저장을 검사한다. HeavyMach 1 이식 원본은 docs/reference/heavymach1/skills/port-heavymach-android에 있다. 그 게임의 50Hz 등 수치를 옮기지 않는다.
