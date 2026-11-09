# 헬기 표적 접근 경로 수정

헬기가 첫 선회 지점을 표적 반대쪽에 잡고 표적 중심을 가로지르는 경로를 고쳤다. 처음 궤도에 들어가기 전에는 목적지 위상을 유지한다. 표적을 가로지르는 경로와 맵 경계에서 표적 쪽으로 좁아진 목적지는 가까운 선회 지점으로 바꾼다. 표적이 이동해 경로가 다시 중심 쪽으로 좁아지는 경우도 같은 목적지 계산에서 처리한다.

기존 속도·관성·좌우 제동·기수 회전·긴급회피 함수는 바꾸지 않았다. `face`, `advance`, `updateDodge`, `finishDodge`, 난수 함수는 e3118d0과 바이트가 같다. 원작 유닛 코드·이미지·정의·수치와 헬기 무장도 보존했다. 원작 160개 시나리오의 각 360틱 배우·그림·저장·사운드 추적은 이전 시계 수정 때 확보한 결과와 같다.

기본 접근 검사 3,040개가 통과했다. 네 기체별 속도·사거리·반경, 32개 시드, 8방향 시작, 중앙·경계·느린 이동 표적을 조합했다. 처음부터 겹친 32개는 분리 상태 접근 검사에서 제외했다. 네 모서리와 틱당 0.25 거리의 원운동 표적을 더한 독립 검사 7,040개에서도 헬기 중심이 표적 중심에서 헬기 반경의 두 배 안으로 들어간 사례가 없었다. 구버전은 같은 기본 접근 검사의 거리 조건에서 실패했다. 움직이는 궤도점 때문에 진입 단계가 계속 잠기는 과거 재현, 표적 해제·교체·회피 후 초기화도 통과했다.

관련 코어 검사는 총 13개가 통과했다. 기존 AlliedAircraft 검사 중 위치를 고정한 채 시간만 넘겨 12방향을 요구하던 검사는 새 진입 단계와 충돌했다. 실패 로그를 보존했다. 이 검사는 실제 `advance`로 7,200틱 이동하면서 같은 12방향 요구를 유지하도록 고쳤다. 진입 전 위치가 그대로이면 시간만 지나도 반대편으로 위상이 바뀌지 않는 검사도 추가했다. 기존 Evasion의 실제 12방향 이동과 기체별 초기 방향 다양성 검사는 그대로 통과했다.

Chrome과 WebKit의 실제 웹 치트빌드에서 각각 4종 × 10레벨을 600틱씩 실행했다. 모든 기체가 움직이며 공격했고, 적의 체력이 감소했으며 회피 효과도 나타났다. 검사 중 저장 쓰기와 추가 AudioContext 생성·종료는 0이었다. 배정된 표적과의 최소 중심거리는 pad 논리좌표에서 스카웃 123.618, AH-10 104.054, 샤이엔 104.069, 벨루가 60.955였다. 8953 사용자 빌드는 변경하지 않고 별도 8971에서 검사했다.

이 변경은 모든 적을 장애물로 회피하는 충돌 방지 기능이 아니다. 여러 적을 가깝게 놓은 웹 장면에서 자기 표적과 거리를 유지하더라도 이웃 적 중심에 약 1~2까지 근접한 사례가 있었다. 처음부터 겹친 상태, 표적 쪽 속도를 이미 가진 근접 상태, 헬기보다 빠르게 밀고 들어오는 표적까지 즉시 분리된다고 보장하지 않는다. 위 거리 검사는 중심과 헬기 반경 기준이며 모든 적의 외형 경계 간 비접촉을 검증한 것은 아니다. 아트 개선과 iPhone 설치는 이 커밋에 포함하지 않는다.

최종 WebKit 첫 재실행은 결과 파일을 쓰는 시점에 디스크 부족(ENOSPC)으로 실패했다. 이 결과를 통과로 세지 않았다. 병렬 브라우저 실행이 끝난 뒤 동일 빌드·검사를 단독 실행해 정상 결과를 저장했다.

- [기본 접근 결과](aircraft-approach-20261109/approach-result.txt) · [수정 전 실패](aircraft-approach-20261109/baseline-red.txt)
- [확장 검사 소스](aircraft-approach-20261109/extended.cpp) · [결과](aircraft-approach-20261109/extended.txt)
- [코어 6개](aircraft-approach-20261109/core-tests.txt) · [추가 7개 최초 결과](aircraft-approach-20261109/additional-tests.txt) · [AlliedAircraft 수정 후](aircraft-approach-20261109/allied-after.txt)
- [원작 함수 보존](aircraft-approach-20261109/source-preservation.json) · [원작 160개 추적](aircraft-approach-20261109/original-after.txt)
- [Chrome](aircraft-approach-20261109/chrome.json) · [WebKit](aircraft-approach-20261109/webkit.json) · [웹 빌드와 소스 해시](aircraft-approach-20261109/web-package.json)
- [디스크 부족 기록](aircraft-approach-20261109/webkit-disk-failure.txt). 실제 전투·회피 캡처는 로컬 `.local/approach-20261109/final-chrome/`에 보존했다.

재현: `ctest --test-dir build/salvo-clock -R 'Aircraft|CheyenneSalvoClock' --output-on-failure`로 관련 빌드의 검사를 실행한다. 웹은 `node Web/tests/aircraft-approach.mjs <출력폴더> <치트빌드URL>`이며 WebKit은 `MD_BROWSER=webkit`을 붙인다. 확장 검사는 `c++ -std=c++20 -O2 -UNDEBUG -ffp-contract=off -fno-fast-math -ISources docs/evidence/aircraft-approach-20261109/extended.cpp -o /tmp/md-approach-extended`로 빌드한다.
