# 헬기 강화 파츠 통합

벨루가 `629cd49` 위에 스카웃 기본 장갑·하우징 `1e346af`와 AH-10 레벨별 장갑·하우징 `c2a7381`의 변경을 합쳤다. 기존 기능 커밋과 개별 검증 기록은 보존했다. 그림을 다시 그리거나 축소하지 않았다. 샤이엔과 벨루가의 완성된 파츠도 유지했다.

같은 조립 파일과 검사에서 겹친 변경을 기체별 이름으로 합쳤다. 스카웃 기본 무장의 이미지 설정이 Lv.2–10의 명시된 하우징을 덮지 않도록 기존 스카웃 수정의 우선순위를 유지했다. AH-10 로켓·공습은 기존 21칸 공유 이미지와 각각의 시작 칸을 유지한다. 병합한 정의에서 MCB 4개·PartsList·TileList를 다시 준비했다.

통합 결과:

- 기본 4기체와 레벨별 40기체가 실제 정의로 남는다. 각 레벨의 표시 노드는 스카웃 5개, AH-10 9개, 샤이엔 7개, 벨루가 11개다. 네 기체 모두 10개의 서로 다른 레벨 이미지 구성을 갖는다. 미사용 작성 파츠는 0개다. [소비 조사](aircraft-upgrades-integration-20261109/consumption.json).
- 파츠 정의·전체 헬기·무장·발사 효과·조작·캠페인 검사 6개와 파츠 소비 검사를 통과했다. 기본 스카웃·AH-10 비교 장면은 장갑 2개 추가를 포함해 스프라이트 28개·그림자 14개다. [코어 검사](aircraft-upgrades-integration-20261109/core-tests.txt).
- 통합 직전의 이미지 1,251개와 헬기 이외의 확장 정의 77개를 그대로 유지했다. 가져온 기능 파일 137개가 원래 기능 커밋과 같다. 이동·회피·무장 수치·총구를 변경하지 않았다. [보존 대조](aircraft-upgrades-integration-20261109/preservation.json), [반입 파일](aircraft-upgrades-integration-20261109/imported-files.json).
- 웹 치트빌드를 새로 만들고 패키지의 모든 자원이 현재 자원과 같은지 대조했다. 검사 화면은 10레벨×7자세를 직접 선택한다. [패키지 대조](aircraft-upgrades-integration-20261109/web-package.json).

기체별 이전 웹·미술 검사는 각 기능 보고서에 남겼다. [스카웃](scout-base-surface-20261109.md), [AH-10](ah10-upgrades-20261109.md), [벨루가](beluga-upgrades-20261109.md), [샤이엔](cheyenne-upgrades-20261109.md). 최종 통합본도 Chrome·WebKit 각각 70자세·40무장 조합·4기체별 생산/강화 UI 6상태를 통과했다. 자세 반복 텍스처 업로드와 추가 AudioContext는 0이었다. 두 브라우저 총 140장 모두 이번 변경 대상인 스카웃·AH-10 밖 픽셀 차이가 0으로, 샤이엔·벨루가·원작 탱크·배경이 보존됐다. [최종 픽셀 대조](aircraft-upgrades-integration-20261109/final-web-preservation.json), [Chrome 전투](aircraft-upgrades-integration-20261109/chrome-combat.json), [WebKit 전투](aircraft-upgrades-integration-20261109/webkit-combat.json), [Lv.1 실제 화면](aircraft-upgrades-integration-20261109/level-1-pose-0.png), [Lv.10 실제 화면](aircraft-upgrades-integration-20261109/level-10-pose-0.png). 이 결과를 사용자 미술 승인으로 처리하지 않는다.

다수 기체 성능용 별도 검사 패키지는 기존 검사 명령 21의 사격을 켠다. 제품 코드는 바꾸지 않았고 정규 패키지와 자원이 완전히 같다. [검사 차이](aircraft-upgrades-integration-20261109/combat-probe.json). 성능 수치·iPhone 설치·main 배포는 이 통합 커밋의 완료 범위에 포함하지 않는다.

후속 유지보수 지시에 따라 [main 병합 뒤 생성 산출물 정리](../usability-todo.md#md-post-main-cleanup-01--main-병합-뒤-생성-산출물-정리)를 TODO로 추가했다. 이번에는 추가 삭제나 자동 정리 도구를 구현하지 않았다.
