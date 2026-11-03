# HeavyMach 시리즈 복원 인계

2026-11-03. HeavyMach 1에서 확인한 복원 순서와 확보 자료를 다음 게임의 조사에 연결한다. HeavyMach 2·HD·Defense의 소스 복원·현대 기기 실행은 아직 완료하지 않았다.

## 확보 자료

[후속 타이틀 조사](related-titles-20261026.md), [파일별 해시·패키지 검사](research/series-acquisition-20261030.json)를 먼저 읽는다.

- 출처: https://archive.org/details/heavy-mach-archive_202406
- 로컬 원본: `/Users/sokol/Documents/Codex/Originals/HeavyMach/heavy-mach-archive_202406`.
- 19개 IPA, 858,465,819바이트. 2026-11-03에 19개 모두 파일 크기와 SHA-256이 보존 명세와 같은지 다시 확인했다. ZIP CRC·Info.plist·Mach-O 검사는 2026-10-30 기록이다.
- HeavyMach 2 네 판본과 HD 두 판본이 있다. 2.0.1은 iPhone 계열, HD는 iPad 계열로 패키지에 표시된다. 서로 다른 자원과 게임 규칙을 같다고 가정하지 않는다.
- Mach Defense 두 판본과 WeMade Heavy MACH: Defense 한 판본이 있다. 번들 ID·자원·규칙 차이를 비교하고 복원 기준을 정한다.
- Assault는 HeavyMach 1의 비교 자료였다. 2·Defense에 그 판본의 Life·광고·결제·맵 규칙을 섞지 않는다. 암호화 표시가 있는 파일의 분석 가능 여부는 확보 기록과 실제 도구 결과로 확인한다.

## 기존 MachDefense 작업

- 로컬: `/Users/sokol/Git/hm-defense`.
- 2026-11-03 확인: `feature/add_android_build`, HEAD `022d6911679aa2f0792d915cc490530c0714a9e9`, 미추적 `.claude/`가 있다.
- [사용자 포크](https://github.com/gelb2/hm-defense), [Mesabloo 원본](https://github.com/Mesabloo/hm-defense). 현재 Kotlin/LibGDX의 core·desktop·android 작업과 기존 APK를 먼저 조사한다. 새 프로젝트를 만든다는 이유로 버리지 않는다.
- 과거 TODO에는 터렛·기지 포격·필살기·GL context 복구·atlas 해제·지형 통과 등이 있다. 최신 동작은 재빌드와 실행 전까지 미확인이다.
- 첫 단계는 기존 소스·에셋·커밋·저장·APK 보존과 실행 범위 확인이다. HeavyMach 1의 C++ 게임 로직으로 일괄 교체하지 않는다.

## REA와 기존 증거

- 설치 스킬: `/Users/sokol/.agents/skills/reverse-engineer-anything/SKILL.md`, `/Users/sokol/.codex/skills/reverse-engineer-anything/SKILL.md`. 두 경로의 파일 존재를 확인했다. 다른 환경에서는 현행 스킬 목록과 도구 제공 상태를 다시 확인한다.
- HeavyMach 저장소에는 위 전역 REA 스킬 자체가 들어 있지 않다. [REA 활용 기록](public/rea-restoration-method-20261025.md)과 [증거 색인](public/rea/inventory-summary.json)이 사용 이력의 입구다.
- 함수 목록·어셈블리·의사 코드·원본 SHA와 Evidence ID 연결을 읽는다. [원본 함수 목록](public/rea/original-procedures.csv), [실패 기록](public/rea/failed-results.csv), [구현 감사](public/rea-implementation-audit-20261026.md).
- 분석 도구의 옛 세션 ID·경로를 새 게임의 분석 결과로 재사용하지 않는다. `tools/export-rea-all.py`, `export-rea-metadata.py`, `export-rea-headers.py`, `export_resources.py`는 HeavyMach 1용 구현이다. 입력 형식·주소·구조체 가정을 읽고 새 대상에 맞는 별도 도구를 만든다.
- [2026-10-26 도구 실행 확인](rea-availability-20261026.md)은 당시 성공 기록이다. 새 세션의 도구 실행 성공을 대신하지 않는다.

## 복원 순서와 단계별 산출물

| 순서 | 수행할 일 | 남길 결과 |
|---|---|---|
| 1. 기준 판본 | 버전·번들 ID·기기·바이너리·자원·암호화 상태를 대조하고 원본을 별도 보존 | 원본 경로·SHA·선정 이유, 변형 금지 원본 목록 |
| 2. 비교 실행 | 가능한 호환 실행기를 조사해 원작 화면·입력·저장·전투 기준을 확보 | 실행 명령·기기·영상·저장 복사본·재현 조건, 실행 불가 원인 |
| 3. 코드·데이터 분석 | REA로 함수·호출·문자열·메타데이터·데이터 reader를 조사하고 불명확한 부분은 어셈블리와 대조 | 함수 주소→게임 규칙→구현 파일 대응, 데이터 포맷과 실패 증거 |
| 4. 게임 구현 | 원작 게임 루프·렌더 명령·좌표·부품 조립·입력·충돌·탄도·AI·성장·저장·음향을 소스로 구현 | 공용 게임 코드와 플랫폼 어댑터, 자원 명세, 원작과의 차이 목록 |
| 5. 같은 조건 비교 | 같은 미션·무기·시작 상태로 원작 비교판과 새 앱을 비교 | 장면·수치·이벤트·저장·소리·시간 차이, 수정 전후 증거 |
| 6. 기능 검증 | 일반·치트 앱에서 전체 무기, 이동·점프·사격·피격·보스·클리어·레벨업·업그레이드·저장·복귀를 확인 | 자동 검사와 직접 조작 결과, 시뮬레이터/실기기 구분, Instruments 등 측정 |
| 7. 플랫폼 확장 | 한 플랫폼의 검증된 게임 코어·자원을 재사용하고 새 플랫폼 입출력·생명주기·GPU 복구·음향·저장을 구현 | 보호 경로 해시, 플랫폼별 빌드·실행·회귀 결과 |
| 8. 배포 | 새 clone과 준비 자원만으로 빌드하고 릴리스·설치·기여자·문서를 대조 | 소스/빌드/태그/릴리스 연결, 설치 안내, 배포 후 확인 |

HeavyMach 1은 호환 실행판과 소스 복원판을 비교하며 진행했다. 후속작도 비교 기준을 확보하는 목적은 같다. 호환 실행기의 재사용 가능 여부는 게임별로 조사하며, 실행이 안 된 단계를 완료로 처리하지 않는다. 원본 개발 소스와 디컴파일 결과, 그 결과를 참고한 새 구현을 구분한다.

## 재사용할 정책과 새로 조사할 부분

재사용할 것은 원본 보존·증거 연결·동일 조건 비교·플랫폼 분리·일반/치트 검증·문서 위계다. [복원 계약](restoration-contract.md), [좌표 정책](coordinate-policy.md), [렌더 정책](rendering-policy.md), [수치 정책](game-calculation-policy.md), [실행 정책](runtime-policy.md), [전체 무기 검증](weapon-validation-procedure.md)을 참고한다.

새 게임의 해상도·좌표계·프레임 시간·맵 구조·자원 포맷·저장 슬롯·유닛 AI·교전 규칙은 그 게임에서 읽는다. HeavyMach 1의 480×320, 저장 3,624바이트, 무기·슬롯 구조를 후속작의 상수로 복사하지 않는다. 기존 게임의 배치·수치·외형과 신규 기능은 별도 확장으로 관리한다.

인게임 UI는 해당 게임의 원작 에셋·버튼 상태·렌더·입력 절차로 만든다. 플랫폼 UI로 화면을 중복 구현하거나, 발사·피격 이벤트마다 저장하는 새 경로를 만들지 않는다. 아트 추가는 실제 채택 에셋과 원작의 부품·좌표·화풍을 대조한다.

## 저장소와 착수 범위

HeavyMach 1과 Unity는 각 저장소에 유지한다. HeavyMach 2는 새 독립 게임 저장소, Defense는 기존 포크 보존을 기준으로 저장소 구성을 결정한다. 허브는 링크와 공통 작업 규칙만 소유한다. 새 게임의 구현을 HeavyMach 1 소스 폴더에 섞지 않는다.

이 문서는 재개 준비다. 새 게임 구현·원격 저장소 생성·기존 Defense 브랜치 교체를 수행한 기록은 아니다. 후속 사용자 지시가 오면 대상 게임·기준 판본·최초 플랫폼을 정하고 1단계부터 진행한다.
