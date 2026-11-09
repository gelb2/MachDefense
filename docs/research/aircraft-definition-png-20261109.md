# 헬기 정의 파일과 PNG 준비 수정

2026-11-09. 헬기 파츠·기체 조립 정의를 C++ 헤더에서 원본 QChunk 형식의 MCB 파일로 옮겼다. 헬기 PNG 29개에 Apple `pngcrush -iphone -f 0`을 적용했다. 원본 유닛의 코드·이미지·표·MCB와 플랫폼 렌더러는 바꾸지 않았다.

## 정의와 조립

[작성 원본](../../Art/Aircraft/runtime/assembly.json)에 파츠 23개와 기본 기체 4개의 소켓·타입·부모 연결점·층 순서를 기록했다. [준비 도구](../../tools/prepare_aircraft_definitions.py)가 원본과 같은 QChunk 구조로 `AircraftParts.mcb`와 `AircraftMachines.mcb`를 만든다. 실제 앱은 기존 로더가 읽는 추가 자원 `Level10Parts.mcb`·`Level10Machines.mcb`에 포함된 같은 노드를 읽는다. 기존 추가 파츠 44개·조립 33개는 노드 바이트가 그대로다.

샤이엔 오른쪽 미사일은 기존 별칭 대신 동일 그림을 가진 `ADuctMissileR` 파츠로 기록했다. 원본 기체 정의와 이름이 겹치지 않는다. Lv.1–10 무장 별칭은 기존 공통 변형 생성 경로를 유지한다. 40개 레벨별 기체를 모두 별도 MCB로 작성했다는 뜻은 아니다. 몸체에 합쳐진 벨루가 날개·엔진과 AH-10 비표시 무장 노드의 아트 분리는 다음 제작 항목에 남아 있다.

## PNG와 색

[도장 완료 입력 29개](../../Art/Aircraft/runtime/png-source/)를 보존했다. [최적화 도구](../../tools/optimize_aircraft_png.py)가 이 입력에서 CgBI PNG를 만든다. 기존 도장 준비 도구도 이 경로를 사용한다. 실행 중 PNG를 다시 만들거나 원본 유닛에 추가 색 변환을 적용하지 않는다.

[실제 디코더 대조](../evidence/aircraft-repair-20261109/optimized-decode.json)에서 29개 모두 크기·알파·불투명 RGB가 같았다. 반투명 RGB는 사전 곱셈과 역산의 정수 반올림으로 달라질 수 있다. 알파를 곱한 RGB 차이의 최대값은 0–255 기준 약 0.9961이다. 모든 RGBA 바이트가 같다고 보고하지 않는다. 총 파일 크기는 224,723바이트에서 199,445바이트로 바뀌었다. 이 수치를 GPU 메모리 감소나 전투 성능 향상으로 해석하지 않는다.

Apple·Web·Android의 기존 stb 디코더와 텍스처 업로드 코드는 보존했다. 원본 패키지의 CgBI 형식에 맞춘 준비 작업이며, 원본 실행기의 CoreGraphics 처리 전체를 복제했다는 뜻은 아니다. 원본 이미지의 색상 프로필을 헬기에 임의 복사하지 않았다.

## 검사 결과

- [원본 보존](../evidence/aircraft-repair-20261109/definition-png-preservation.json): 보호 파일 2,652개 해시 동일. 기존 추가 파츠 44개·조립 33개 동일.
- [원본 유닛 회귀](../evidence/aircraft-repair-20261109/definition-png-original.txt): 휴대폰·태블릿 크기 각각 80미션, 조건당 360틱의 위치·HP·렌더 명령·사운드·저장 해시가 기준과 모두 같다.
- [코어 53개](../evidence/aircraft-repair-20261109/definition-png-core-passed.txt) 통과. [PNG/JPEG 크기 판독 추가 검사](../evidence/aircraft-repair-20261109/png-header-test.txt) 통과.
- [Chrome](../evidence/aircraft-repair-20261109/definition-png-chrome.json)·[WebKit](../evidence/aircraft-repair-20261109/definition-png-webkit.json) 치트빌드에서 4종 Lv.1·10 발사·피해·회피 효과를 확인했다. 검사 중 저장 쓰기 0회, 오디오 컨텍스트 생성·종료 횟수 변화 없음. 캡처의 기체와 그림자를 직접 확인했다.
- [도장 검사](../evidence/aircraft-repair-20261109/optimized-paint.json) 통과. 도장 준비를 재실행해 승인 입력에서 다시 출력되는 경로를 확인했다.

검사 도구 두 곳의 변수 이름 오류를 고쳤다. 그 뒤 전체 검사에서 JPG를 PNG 헤더로 읽던 기존 테스트 문제가 드러났다. [실패 기록](../evidence/aircraft-repair-20261109/definition-png-core-first-failure.txt)을 보존하고 실제 JPG 크기를 읽도록 수정한 뒤 같은 검사를 통과했다. 제품의 원본 이미지나 판정 조건을 바꿔 통과시키지 않았다.

이번 단계에서 실기기 CPU·GPU·프레임 비교와 원본 대비 질감 동등성은 확인하지 않았다. 선회 그림 제작·사거리 정지·회피 가감속 수정 후 최종 설치본으로 검사한다.
