# 원작 Lv.5 무장과 새 몸체 연결

2026-11-07 · 작업 브랜치 `feat/level10-effects-art`

라이플·미사일·발칸·이온·레이저·플라즈마 포탑의 Lv.6~10 무장은 각 계열의 원작 Lv.5 그림을 사용한다. 공용 몸체 5개는 사용자가 채택한 design-v5 시안으로 교체했다.

## 변경

- 무장 PNG 30개는 해당 원작 Lv.5 PNG를 바이트 그대로 복사했다. 파일명은 기존 레벨별 참조를 유지한다.
- 몸체 PNG 5개는 `Art/Level10/20261107/design-v5/review/body-6.png`부터 `body-10.png`까지 그대로 복사했다.
- 포장 스크립트도 같은 선택을 사용한다. [자원 출처·해시](../../Art/Level10/20261107/runtime-art.json)에 입력과 출력 해시를 기록했다.
- 몸체 캔버스는 모두 64×64다. 실제 그림 범위는 Lv.6부터 52×60, 53×60, 54×60, 55×60, 53×60px다. 채택한 몸체의 비율을 유지했다.
- 작업 전후 해시를 대조했다. Resources에서 바뀐 파일은 포탑 PNG 35개다. 성능 테이블, 부품 연결 데이터, 원작 Lv.1~5, 음원, 게임 코어와 Apple·Android 코드는 같다.

## 웹 치트빌드 검사

Emscripten 치트빌드 `30ca4856cede770c`를 로컬 Chrome·WebKit에서 실행했다. `TurretArtSweep.hpp`는 실제 BattleModels로 조립한 Frame을 게임의 GPU 렌더러에 전달한다.

| 검사 | Chrome | WebKit |
| --- | --- | --- |
| 6계열 × Lv.5~10 × 4방향 | 144자세 통과 | 144자세 통과 |
| Lv.5 무장 바이트·몸체 레벨 연결 | 통과 | 통과 |
| 캔버스·부품 위치·회전축·총구 좌표 | 유지 | 유지 |
| 강화 데이터·구매·거절 | 130행·117건·143건 | 130행·117건·143건 |
| 저장·복원 코드 재로드 | 130건 | 130건 |
| 유닛 생산·배치·특수공격 | 280건·120건·100건 | 280건·120건·100건 |
| 생산 HUD 포인터·휠 입력 | 통과 | 통과 |
| 검사 중 실제 저장 쓰기·JS 오류 | 0건·0건 | 0건·0건 |

Chrome에서 미션 실패→재시도→승리→다음 미션과 폰·태블릿·데스크톱 크기 변경도 통과했다. 비교 캡처의 레이저 포신이 화면 끝에 닿아 검사 화면 간격을 넓혔다. 게임의 화면 크기나 포탑 배율은 바꾸지 않았다.

현재 자원으로 PlayerParts·Level10Data·Level10HUDSweep·TransferLevel10 검사 실행파일을 다시 빌드해 4개 모두 통과했다. Level10Data에서는 추가 레벨의 공격력·내구력 증가, 사거리 증가 구간에서의 실제 무장 컨트롤러 발사와 경계 밖 미발사를 검사했다.

## 근거와 적용 상태

- 로컬 로그·화면: `.local/body-v5-integration/`.
- `turret-art-{chromium,webkit}.json`, `level10-{chromium,webkit}.json`, `game-flow.json`, `core-tests-current.log`, `preservation.json`.
- [실제 렌더링 비교 보고서](https://heavymach-dev-report-20261025.hiphopxy.chatgpt.site/machdefense.html#turret-body-only-20261107).
- 이번 몸체 변경은 작업 브랜치와 로컬 웹 치트빌드에 적용했다. 기기 앱 설치·develop/main 병합·운영 PWA 갱신은 아직 하지 않았다.
