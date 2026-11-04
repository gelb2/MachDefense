---
name: apple-wireless-device
description: MachDefense iPhone·iPad의 USB·무선 설치·디버깅·검사·종료에 사용한다.
---
# apple-wireless-device

docs/design.md, runtime-policy.md, validation-plan.md를 읽는다. 현재 호환판 타깃은 MachDefenseCompatibility, 번들 ID는 com.gelb2.machdefense.compatibility, 표시 이름은 MachDefense Port다. 소스 재구현 제품은 MachDefense (`com.gelb2.machdefense.source`)와 MachDefenseCheat (`com.gelb2.machdefense.source.cheat`)다. 각각 별도 저장 컨테이너를 사용한다. 포팅판과 혼동하지 않는다. devicectl에서 transport를 검사하고 localNetwork일 때만 무선이라고 쓴다. 기존 페어링/저장/서명을 보존한다. 설치→실행→기능 조작→증거 저장→입력 해제→앱 종료→홈 화면 확인을 수행한다. 프레임 계측은 디버거 부착 여부를 구분한다. 원본 절차는 docs/reference/heavymach1/skills/apple-wireless-device에 있다. 그곳의 앱 이름과 기기 값으로 실행하지 않는다.

실기기 XCTest는 기존 MachDefenseCheat를 호스트로 사용한다. CLI 서명이 `errSecInternalComponent`로 실패하면 기존 Xcode GUI 서명·빌드 절차를 사용한다. 키체인 권한을 임의로 바꾸지 않는다. 검사 후에는 Clean Build Folder와 일반·치트 스킴 빌드로 테스트 플러그인이 없는 앱을 만든다. 결과·실행 시간·기기·디버거 부착 여부를 기록한다.

Device Hub의 AX와 화면은 읽히지만 좌표 입력만 `noWindowsAvailable`로 실패하면, 현재 AX에 있는 `Switch to compact window`로 기기 단독 창을 열고 새 화면 좌표로 다시 검사한다. 2026-11-05 실제 iPad에서 이 전환 후 게임 터치가 복구됐다. 창 읽기 실패만으로 Mac 잠금이나 기기 잠금으로 단정하지 않는다.
