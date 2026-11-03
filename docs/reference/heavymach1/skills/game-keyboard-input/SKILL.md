---
name: game-keyboard-input
description: HeavyMach의 키보드 전 조작, 조준, 키 재지정, 입력 해제와 메뉴 탐색을 구현·검증할 때 사용한다.
---

# HeavyMach 키보드 입력

키보드 지원은 전투·조준·이동·점프·모든 무기 선택과 타이틀·슬롯·모드·미션·업그레이드·일시정지·결과 메뉴를 키보드만으로 다루는 범위다. 터치나 마우스로 빠진 단계를 대신한 검사를 키보드 완료로 보고하지 않는다.

- 물리 키와 게임 동작을 분리한다. 플랫폼은 키 누름/해제·연결·포커스를 전달하고 공통 모듈이 액션을 만든다. 유선·블루투스에 별도 게임 규칙을 만들지 않는다.
- A/D 이동, Space 점프 등 익숙한 기본값을 제안하되 조준·발사·정밀 조작·무기 전환·메뉴 키를 모두 표시하고 재지정할 수 있게 한다. 중복 배정은 교환 또는 명시 오류로 해결하며 설정 저장은 변경 확정 시에만 수행한다.
- 키보드만으로 설정을 열고 바꾸고 기본값을 복원할 수 있어야 한다. Escape 취소·포커스 이동·선택 강조·확인을 제공한다. OS 단축키와 문자 입력 중의 게임 조작을 구분한다.
- 아날로그 입력 대체는 가상 기울기·가상 조준점을 공통 코드에서 계산한다. 원작 dead zone·필터·데미지·사거리를 바꾸지 않는다. 일반 키보드는 압력값을 제공한다고 가정하지 않는다.
- OS 반복 keyDown을 프레임 시간이나 반복 점프로 쓰지 않는다. 포커스 상실·키 취소·기기 연결 해제·메뉴 진입에서 누른 키와 발사를 해제한다. 두 반대 방향 동시 입력을 중립 처리한다.
- 동작별 누름/유지/해제, 재매핑, 키 조합, 모든 무기·필살기, 키보드만의 메뉴 경로를 검사한다. API 주입·시뮬레이터·물리 키보드 관측을 구분한다.

근거(2026-10-26 확인): [Xbox XAG107](https://learn.microsoft.com/en-us/xbox/accessibility/xbox-accessibility-guidelines/107), [키 재지정 지침](https://gameaccessibilityguidelines.com/allow-controls-to-be-remapped-reconfigured/), [Apple 키보드 HIG](https://developer.apple.com/design/human-interface-guidelines/keyboards/), [Factorio 공식 위키의 실제 배치·재지정](https://wiki.factorio.com/Controls). 위 출처의 적용 원칙을 정리한 프로젝트 스킬이며 원 제작사의 공식 스킬은 아니다.
