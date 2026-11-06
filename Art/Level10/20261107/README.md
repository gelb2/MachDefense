# 추가 레벨 외형 시안

포탑 무장 6종 Lv.6–10과 기지 몸체·기관총·레이더 Lv.8–10, 39개 부품에 공용 포탑 몸체 Lv.6–10 5개를 더해 총 44개 부품 시안을 만들었다. built-in image_gen으로 원작 PNG를 참조해 만들었다. 원본 이미지는 수정하지 않았다. 게임 연결 전 시안이며 현재 Resources는 기존 최종 레벨 그림을 쓴다.

**2026-11-07 몸체 추가:** 누락됐던 공용 몸체 Lv.6–10 5개를 제작했다. [몸체와 무장 조립 시안](https://heavymach-dev-report-20261025.hiphopxy.chatgpt.site/machdefense.html#turret-body-concepts-20261107)에는 원작 Lv.5와 신규 5단계를 6종별로 나란히 표시했다. 원작 무장·신규 무장·몸체 단독 전환과 무장 회전을 제공한다. 게임 자원은 변경하지 않았다. [원작 대조](../../../docs/evidence/turret-body-scale-20261107.md).

[통합 비교 보고서](https://heavymach-dev-report-20261025.hiphopxy.chatgpt.site/machdefense.html#level10-effects-art-20261107) · [프롬프트·출처·해시](prompts.json) · [제작 스킬](../../../.agents/skills/extend-machdefense-art/SKILL.md)

| 부품 | 단계 | 시안 |
|---|---|---|
| rifle | 6, 7, 8, 9, 10 | [rifle-v1.png](rifle-v1.png) |
| missile | 6, 7, 8, 9, 10 | [missile-v1.png](missile-v1.png) |
| vulcan | 6, 7, 8, 9, 10 | [vulcan-v1.png](vulcan-v1.png) |
| ion | 6, 7, 8, 9, 10 | [ion-v1.png](ion-v1.png) |
| laser | 6, 7, 8, 9, 10 | [laser-v2.png](laser-v2.png) |
| plasma | 6, 7, 8, 9, 10 | [plasma-v1.png](plasma-v1.png) |
| base-body | 8, 9, 10 | [base-body-v2.png](base-body-v2.png) |
| base-gun | 8, 9, 10 | [base-gun-v1.png](base-gun-v1.png) |
| base-radar | 8, 9, 10 | [base-radar-v2.png](base-radar-v2.png) |

기지 몸체 v1은 빨간 착륙 표식 누락과 배경 때문에 제외했다. 레이저·레이더 v1도 원작 최종 형상·색 비중을 대조해 v2로 수정했다. 탈락본은 이력을 위해 보존했다.

후속 연결에서는 부품별 투명 여백·논리 크기·회전 중심·총구·레이더 독립 회전·장비 미리보기·생산 아이콘을 원작 조립 경로에서 검사한다. 현재 보고서의 같은 크기 비교만으로 실제 게임 조립 검사가 끝났다고 보지 않는다.

특수공격 5종의 추가 레벨은 기존 최종 아이콘·탄환·폭발·음원을 재사용한다. 이미 Lv.10 외형이 있는 메크와 탱커도 유지한다.

## 몸체와 조립

| 단계 | 몸체 원본 PNG | 변화 |
|---|---|---|
| 6 | [몸체 6](turret-body-6-v1.png) | 장갑판 구성 |
| 7 | [몸체 7](turret-body-7-v1.png) | 통풍구 두 곳 |
| 8 | [몸체 8](turret-body-8-v1.png) | 분할 상부 덮개와 점검판 |
| 9 | [몸체 9](turret-body-9-v1.png) | 세로 보강 구조 |
| 10 | [몸체 10](turret-body-10-v1.png) | 밀집 패널과 상부 덮개 |

몸체 5개도 built-in image_gen으로 원작 Lv.3·4·5를 참조해 생성했다. [전체 프롬프트·참조·출력 해시](body-prompts.json), [조립 설정](body-composition.json), [미리보기 좌표 측정](body-preview-calibration.json), [보고서 렌더 스크립트](body-composition-preview.js)를 보존했다.

미리보기는 원작 Lv.5의 캔버스와 연결점을 사용한다. 생성 그림의 장착 링 위치를 지정하고 균등 배율로 원작의 그림 범위 안에 놓았다. 높이·너비를 따로 늘리지 않았다. 35개 신규 그림의 범위 검사, 36개 조립 화면, 무장 선택·몸체 단독·45도 회전·초기화, 393px 모바일 표시를 Chrome에서 확인했다. 무장 파일명과 보고서 WebP 이름이 달라 처음 로딩이 실패한 부분은 경로를 고쳐 재검사했다. 실제 게임 연결·미세 도트 보정·사격 검사는 후속 단계다.
