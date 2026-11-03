---
name: game-keyboard-input
description: MachDefense에 키보드·포인터 조작을 연결하고 플랫폼별 입력을 검증할 때 사용한다.
---
# game-keyboard-input

원본 동작 목록과 docs/coordinate-policy.md를 먼저 읽는다. 명령을 게임 입력으로 모으고 OS는 이벤트 전달만 한다. 키 매핑은 실제 원작 행동과 사용자 요구로 정한다. HeavyMach 1 이동/점프 키를 그대로 적용하지 않는다. 눌림/반복/해제/포커스 손실을 검사하고 저장은 설정 변경 시점에만 수행한다. 터치와 키보드가 같은 게임 동작을 호출하는지 검사한다.
