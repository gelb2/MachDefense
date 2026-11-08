# 재작업 후속

AH-10 생성 성공 및 로컬 연결: [최신 기록](../../../../docs/evidence/aircraft-rework-20261108.md). 아래 실패 기록은 이전 시점이다. Cheyenne은 이번 재시도도 실패했다.

# 사용자 제공 도안 재제작 — 2026-11-08

- AH-10: IMG_1166의 좌측 상면도. 생성 도구의 출력 단계 moderation_blocked/violence로 실패했다. 새 AH-10 그림은 없다. 이전 AHeavyBody를 새 도안 적용본으로 취급하지 않는다.
- Beluga: IMG_1167의 좌측 상면도. `beluga-sheet-v1.png`는 고정 몸체와 부품 시트다. 최초 로터 크롭에 옆 그림 조각이 남아 `beluga-rotor-v2.png`를 별도 생성했다.
- 사용자가 “제공한 이미지의 비율을 지킨다”고 답했다. 포장에서는 몸체·무장에 같은 균일 축소율 13.666667%를 사용한다. 독립 로터는 원래 시트의 약 80픽셀 크기로 균일 축소한다. 회피·무기 수치를 변경하지 않는다.
- 실제 연결: `tools/pack_beluga_user.sh`, `Sources/Core/Simulation/AircraftModels.hpp`. 몸체·중앙 로터·기관포 2개·미사일 2개로 조립한다. 기존 무장 정의와 게임 식별자 AircraftTransport/OrcaBomber를 유지한다.
- 이미지 생성 도구: 내장 image_gen. 제공 도안의 실루엣·색·장갑·시점을 보존하고 종이·글자·화살표를 제거하며 독립 로터·무장을 분리하도록 요청했다. 전체 프롬프트는 `prompts.txt`에 보존한다.

- Cheyenne: IMG_1168 좌측 상면도. 입력 단계 moderation_blocked/violence로 실패했다. 새 그림은 없다. 기존 ADuctBody를 제공 도안 적용본으로 보고하지 않는다.

## Cheyenne 후속 생성과 Beluga 로터

Cheyenne 최종 시트는 `cheyenne-sheet-v2.png`다. v1의 몸체 고정 포신을 이미지 편집으로 제거했다. `cheyenne-runtime-v2.json`과 `tools/pack_cheyenne_user.sh`에 크롭·균일 배율·연결점·총구를 기록했다. 원작 P_Body_01·13·34를 질감 참조로 사용했다. Beluga 로터는 기존 그림의 허브 중심을 기준으로 재포장했다. [실제 웹 조립과 검사](../../../../docs/evidence/aircraft-resume-20261108.md).
