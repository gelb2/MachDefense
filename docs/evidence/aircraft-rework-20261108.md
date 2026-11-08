# 사용자 도안 재작업 — 2026-11-08

AH-10은 IMG_1166의 좌측 상면도로 내장 image_gen에서 재제작했다. 첫 시트의 길어진 몸체와 6엽 로터를 수정해 몸체와 3엽 로터를 따로 생성했다. 고정 포신을 몸체에서 제거하고 별도 무장을 조립했다. 로터 두 축과 무장 연결점·총구를 새 그림에 맞췄다. 무장 종류·피해·회피·이동 수치는 변경하지 않았다.

[몸체](../../Art/Aircraft/parts/user-20261108/ah10-body-v3.png), [로터](../../Art/Aircraft/parts/user-20261108/ah10-rotor-v2.png), [무장 시트](../../Art/Aircraft/parts/user-20261108/ah10-sheet-v2.png), [포장](../../tools/pack_ah10_user.sh), [해시·크롭](../../Art/Aircraft/parts/user-20261108/ah10-runtime-v3.json), [실제 프롬프트](../../Art/Aircraft/parts/user-20261108/rework-prompts.txt).

몸체 1425×973 그림 범위를 112×76으로 균일 축소했다. 생성 몸체 비율은 약 1.46이며 도안에서 추정한 1.40과 완전히 같지는 않다. X/Y를 따로 늘리지 않았다. 픽셀 단위 동일 복제나 사용자 최종 승인으로 보고하지 않는다.

C++ 관련 8개 검사 통과. Chrome·WebKit 각각 640×640, 320×480, 768×1024에서 이동·사격·피해·일시정지를 검사했다. 준비 후 추가 텍스처 업로드와 검사 중 저장 쓰기는 0회다. 원작 유닛과 몸체·조립본을 같은 배율로 캡처하고 확인했다. 새 game.data 안의 AH-10 PNG 5개가 Resources와 바이트 단위로 일치한다. 로컬 증거: `.local/aircraft-rework-20261108/`.

Cheyenne은 IMG_1168 재요청도 출력 단계 `400 / moderation_blocked / violence`로 실패했다. request ID `d5446d4a-41de-4165-ab17-37004b1bb435`. [오류](../../Art/Aircraft/parts/user-20261108/cheyenne-retry-error.json). 새 그림이 없어 기존 실행 자원을 유지했다. Beluga는 앞선 IMG_1167 적용본을 유지했다. 전체 헬기 재작업은 미완료다.

시작 HEAD f942a1f, feat/aircraft-visual-repair-20261108에서 수정했다. 기기 재설치·운영 PWA 배포·커밋·병합은 수행하지 않았다.
