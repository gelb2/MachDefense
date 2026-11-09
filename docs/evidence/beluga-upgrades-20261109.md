# 벨루가 강화 파츠 연결 — 2026-11-09

벨루가 Lv.1–10에 날개 뿌리 보강판 두 개씩을 연결했다. 레벨별 20개 파일과 각 파일의 7자세를 CgBI PNG·명시적 MCB 파츠로 준비했다. 기존 승인 그림의 금속 조각을 재조립한 결과다. 새 그림 생성이나 전체 표면 개선 완료로 판단하지 않는다.

기존 몸체·날개·엔진·로터·무장 그림과 총구를 유지했다. 표시 파츠는 9개에서 11개로 늘었다. 두 보강판은 기존 몸체 소켓에 붙고, 원래 7단계 선회 타일을 사용한다. 원작 공용 로더·조립·렌더러와 이동·무장 수치는 수정하지 않았다. 제품 소스 변경은 헬기 전용 `AircraftVisual.hpp`에서 새 파츠 이름을 인식하는 한 줄이다.

## 확인한 결과

- [코어 6검사](beluga-upgrades-20261109/core-tests.txt): 정의·실제 소비·샤이엔 발사 시계·헬기·이동·Campaign 통과. 기존 파츠 좌표·총구와 보강판의 등록 위치를 대조했다.
- [웹 요약](beluga-upgrades-20261109/web-summary.json): Chromium·WebKit 각각 70자세, 40레벨 무장 조합, 생산·강화 UI 6상태 통과.
- [화면 비교](beluga-upgrades-20261109/web-preservation.json): 140장 모두 벨루가 영역 밖 픽셀 차이 0. 기준은 `10ae41c`의 자원과 같은 레벨 검사 입력을 지원하는 별도 웹 빌드다.
- [패키지 대조](beluga-upgrades-20261109/web-package.json): 새 20개 파일 바이트가 Resources와 같다. 두 브라우저에서 실제로 읽었고, 준비 뒤 반복 자세 변경의 추가 업로드는 0이다.
- [UI 비교](beluga-upgrades-20261109/ui-comparison.json): 기존 벨루가 아이콘 좌표는 같다. 화면 밖 무작위 적의 위치와 그 레이더 표시만 다르며, 나머지 기존 화면 안 스프라이트 좌표는 같다. 실제 크기에서 아이콘 잘림과 이름·가격 가림은 보이지 않았다.
- [보존 파일](beluga-upgrades-20261109/source-preservation.json): 기존 이미지·오디오와 이동·무장 코드는 그대로다. 기존 혼합 MCB의 원작 노드는 [정의 생성 기록](beluga-upgrades-20261109/definition-generation.json)에서 보존 해시를 확인한다.

[Lv.1 중립](beluga-upgrades-20261109/level-1-pose-0.png), [Lv.10 중립](beluga-upgrades-20261109/level-10-pose-0.png), [강한 좌선회](beluga-upgrades-20261109/level-10-pose-3.png), [강한 우선회](beluga-upgrades-20261109/level-10-pose-6.png), [강화 상세](beluga-upgrades-20261109/upgrade-detail-level10.png)는 실제 웹 캡처다. [시각 검수](beluga-upgrades-20261109/visual-review.txt)에서 관찰한 범위를 구분했다. 실제 게임 크기에서 보강판 변화는 작으며 사용자 미술 승인은 별도다.

## 비용과 남은 작업

[준비 명세](../../Art/Aircraft/upgrades-20261109/beluga/manifest.json)의 추가 POT RGBA 크기는 레벨당 두 장 64KiB, 10레벨 전부를 읽으면 640KiB다. 보강판 두 개와 각각의 그림자 때문에 기체당 표시 제출은 4회 늘어난다. 이를 CPU·GPU 시간이나 전체 프레임 성능 개선으로 해석하지 않는다.

스카웃 기본 표면·AH-10 강화 외형의 별도 작업을 통합한 뒤 네 기체의 최종 전투·캐시·오디오·저장과 40/100기 CPU·GPU·프레임을 같은 조건에서 비교해야 한다. 이번 UI 검사는 치트로 지급·장착했으며 자연 획득 경로는 검사하지 않았다. 아이폰 설치와 보고 사이트 후속 게시도 이번 커밋의 완료 범위에 포함하지 않는다.

제작 원본의 `authored-registration.json`과 `provenance.json`에 남은 연결 전 상태 문구는 당시 후보 기록이다. 현재 연결·실행 결과는 이 보고서와 웹 증거가 설명한다. 처음 레벨 검사를 지원하지 않았던 기준 빌드 캡처와 비교 마스크의 알파 처리 오류는 제외했고, 원본 실패 자료는 로컬에 보존했다.
