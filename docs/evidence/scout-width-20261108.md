# Scout Heli 폭 조정 — 2026-11-08

사용자 지시 순서에 따라 먼저 AH-10·Beluga·Cheyenne의 현행 크기·디자인을 `ce58bd0`에 보존 기록으로 커밋했다. [보존한 이미지 해시·배율](../../Art/Aircraft/runtime/preserved-three-20261108.json). 그다음 Scout Heli만 수정했다.

북쪽 방향·로터 각도 0·월드 2배율에서 몸체·로터·무장을 둘러싸는 사각형을 그렸다. 투명 여백과 그림자는 제외했다. 기존 Scout 114×156px에 가로·세로 같은 118/114 배율(약 1.03509)을 적용했다. 최종 Scout는 **118×162px**, Cheyenne은 **118×158px**다. 폭은 정확히 같고 길이는 기존 비율을 유지한다. 화면 픽셀 반올림은 1px 미만이다. 다른 로터 각도에서는 외곽 폭이 달라진다.

첫 측정에서는 원본 PNG 폭을 이용한 118/116 배율이 실제 출력 116px에 그쳤다. 실측 기준 114px로 보정하고 다시 빌드해 118px를 확인했다. 첫 결과는 `.local/scout-width-20261108/first-measurement.json`에 보존했다.

[사각형 비교](scout-width-20261108/comparison.png) · [실측 수치](scout-width-20261108/bounds.json) · [측정 코드](scout-width-20261108/measure.js)

## 보존과 확인

- AH-10 230×171, Beluga 164×173, Cheyenne 118×158px로 이전 화면과 같다.
- [보존 검사](scout-width-20261108/preservation.json): 전체 PNG 1,020개, 세 기체 조립 배율·모든 부품 정의, 무장·HP·속도·가격 테이블이 그대로다.
- [C++ 검사 8개](scout-width-20261108/core-tests.txt) 통과. 전 레벨·화면 배율의 크기·총구와 그림자 간격을 포함한다.
- [Safari 실제 전투](scout-width-20261108/combat-safari.json): 이동·일시정지·피해·발사음 통과. 추가 저장·준비 후 추가 텍스처 업로드 0회.

[네 기체 현행 사양과 논의 항목](../research/aircraft-specs-20261108.md)을 정리했다. 전투 수치는 변경하지 않았다. 게임 main·운영 PWA·기기 설치는 갱신하지 않았다.
