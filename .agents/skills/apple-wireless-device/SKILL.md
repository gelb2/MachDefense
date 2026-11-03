---
name: apple-wireless-device
description: MachDefense iPhone·iPad의 USB·무선 설치·디버깅·검사·종료에 사용한다.
---
# apple-wireless-device

docs/design.md, runtime-policy.md, validation-plan.md를 읽는다. MachDefense 타깃·번들 ID·일반/치트 저장은 아직 미정이다. 실제 프로젝트에서 먼저 확인한다. devicectl에서 transport를 검사하고 localNetwork일 때만 무선이라고 쓴다. 기존 페어링/저장/서명을 보존한다. 설치→실행→기능 조작→증거 저장→입력 해제→앱 종료→홈 화면 확인을 수행한다. 프레임 계측은 디버거 부착 여부를 구분한다. 원본 절차는 docs/reference/heavymach1/skills/apple-wireless-device에 있다. 그곳의 앱 이름과 기기 값으로 실행하지 않는다.
