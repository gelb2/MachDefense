# 몸체·라이플 새 부품 시안

원작 몸체와 라이플 Lv.1~5의 반복 구성을 읽고 Lv.6~10 부품을 새로 설계했다. 사용자는 원작 부품의 재조합에 한정하지 말고, 같은 디자인 문법으로 필요한 새 부품을 만들라고 지시했다.

- 원작: Resources/Images/Turret_b_01..05.png, Turret_gun_01..05.png.
- 새 몸체: 분리 장갑, 3분할 상부 덮개, 쌍 냉각부, 매립 창, 계단식 장갑.
- 새 라이플: 갈라진 덮개, 통풍구 덮개, 열린 지지대, 분할 장갑, 단차가 있는 외피.
- 비교: review/의 원작·부품·조립 그림. 64×64 캔버스, 균일 축소. 별도 가로/세로 늘리기는 하지 않았다.
- measurements.json: 원작 해시, 생성 파일 해시, 자른 범위, 배율, 실제 크기, 밝기.
- generation.json와 revisions.json: 원작 참조와 생성·수정 프롬프트.

몸체 높이 60px와 라이플 길이 61px는 맞췄다. 몸체 폭은 52~55px, 라이플 높이는 41~46px로 원작의 52px·43px와 차이가 남았다. 새 몸체의 상부 경사면과 링은 원작보다 매끈하고, 새 라이플은 밝은 세부 구조가 적다. 이번 결과는 디자인 검토용이다. Resources와 앱에는 연결하지 않았다.

body-first.png와 rifle-first.png는 첫 생성 결과다. body-dark-rejected.png는 질감 수정 뒤 지나치게 어두워져 제외했다. rifle-proportion-pass.png는 중간 비례 수정본이다. 최종 비교에 쓰는 파일은 body.png와 rifle.png다. 흰 배경·포신 수 오류가 난 중간 라이플 출력은 revisions.json에 로컬 생성 출처를 남겼고 채택하지 않았다.

원작 1~5단계의 서로 다른 연결점은 original-anchors.json에 기록했다. prepare_turret_design_review.py가 이를 읽어 원작 조립본을 만든다.
