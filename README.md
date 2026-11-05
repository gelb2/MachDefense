# MachDefense

원본 MachDefense를 현대 기기에서 실행하도록 복원하기 위한 비공개 개발 저장소다. HeavyMach 1에서 사용한 원본 분석·호환 실행·소스 재구현·비교 검증 절차를 적용한다.

위메이드 1.0.3 포팅판을 기준으로 iPhone·iPad 공용 소스를 재개발하고 있다. 포팅판에서 첫 전투·생산·교전·저장·기지 강화 6종을 확인했다. 새 C++ 소스 앱은 일반·치트 빌드가 되었고, iPhone·iPad 시뮬레이터에서 첫 미션·생산·이동을 확인했다. 공통 코드 검사 22개가 통과했다. 전체 미션 대조·일부 연출·화면 크기별 배치는 진행 중이다.

[소스 재개발 계획](docs/source-reimplementation-plan.md) · [현재 검사 결과](docs/current-status.md) · [호환 실행판 빌드](Compatibility/README.md)

- [현재 인계](HANDOVER-CURRENT.md) · [문서 목차](docs/README.md) · [남은 작업](docs/usability-todo.md)
- [트러블슈팅: 오디오 컨텍스트와 기관총 프레임 드랍](docs/troubleshooting.md)
- [복원 계획](docs/restoration-plan.md) · [원본 명세](docs/source-provenance.md) · [스킬과 출처](docs/skills.md)
- [작업 환경 설정](docs/development.md) · [저장소 규칙](docs/repository-management.md)

원본 IPA는 이 저장소와 GitHub에 포함하지 않는다. 비공개 상태는 별도 공개 지시까지 유지한다.

## Android

Android 일반·치트 앱을 Galaxy와 에뮬레이터에서 실행했다. 기존 C++ 게임 코드·자원을 공유한다. [빌드와 설치 파일](Android/README.md), [실행·성능·아이폰 저장 이전 검사](docs/evidence/android-native-20261106.md).
