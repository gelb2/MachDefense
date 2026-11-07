# AH-10·Beluga 제공 도안 작업

사용자가 제공한 IMG_1166과 IMG_1167을 각각 AH-10·Beluga의 외형 기준으로 지정했다. 후속 지시는 “제공한 이미지의 비율을 지킨다”다. [원본·해시](../../Art/Aircraft/references/user-20261108/manifest.json), [생성 파일·포장·프롬프트](https://github.com/gelb2/MachDefense/blob/a144abef9b91949dbf30793d2ff1f8bab277a672/Art/Aircraft/parts/user-20261108/README.md).

## 결과

- 게임 표시 이름은 AH-10·Beluga로 바꿨다. Description_us와 공용 기체 정의가 같은 이름을 사용한다. 기존 저장·해금·강화 키 Gunship/OrcaBomber는 유지한다.
- Beluga 그림을 생성해 고정 몸체·중앙 로터·기관포 2개·미사일 2개를 기존 조립 경로에 연결했다. 이전 두 팬 그림을 중앙 로터 하나로 교체했다. 무기·회피·이동·피해 계산은 변경하지 않았다.
- 최초 로터 크롭에는 옆 그림 조각이 섞였다. 웹 캡처에서 발견해 로터를 별도로 다시 추출했다. 같은 문제를 수정하지 않고 완료로 보고하지 않는다.
- **AH-10 외형은 미완료다.** 생성 요청에서 output 단계 violence 차단을 받았고 결과 PNG가 없다. 이전 건쉽 그림을 새 도안 적용본으로 보고하지 않는다. [오류 원문](../../Art/Aircraft/parts/user-20261108/ah10-generation-error.json).

## 에셋만 수정한 웹 빌드에서 이전 그림이 남은 오류

최초 로터 수정 뒤 `package.py`만 실행했고, 이 도구는 이미 만들어진 `game.data`를 다시 나눴다. 기존 Web CMake에는 PNG·테이블 입력의 링크 의존성이 없어서 일반 빌드도 에셋 변경만으로 다시 링크하지 않았다. 그 결과 캡처에는 이전 로터가 남았다.

Web/CMakeLists.txt에 자원 파일의 LINK_DEPENDS를 추가했다. 다시 링크·포장한 뒤 `game.data` 안의 Beluga PNG 4개와 Description_us가 현재 Resources와 바이트 단위로 같은지 검사했다. 모두 일치했다. 이전 묶음으로 실행한 결과를 최종 그림의 검증으로 사용하지 않고 새 묶음으로 웹 검사를 다시 실행했다.

## 검사

C++ 관련 8개 검사 통과. 캠페인 검사에서 세 표시 이름과 40개 생산·강화·사격·복원 조건을 확인했다. 웹 Chromium·WebKit의 3화면 이동·사격·피해·일시정지 검사도 통과했다. 워밍업 뒤 추가 텍스처 업로드와 게임 저장 쓰기는 0회다. 실제 Beluga 몸체·조립본을 원작 메크·탱크와 같은 배율로 캡처했다. 로컬 근거는 `.local/aircraft-user-20261108/`다.

이는 외형의 사용자 최종 승인이나 장시간 실기기 성능 검사가 아니다. iPhone·iPad·Android 재설치, main 병합, 운영 PWA 배포는 이번에 수행하지 않았다.

## Cheyenne 추가 지시와 생성 실패

사용자가 IMG_1168을 기존 오르카의 새 도안으로 주고 이름을 Cheyenne으로 지정했다. 원본 PNG와 해시를 위 원본 목록에 추가하고, 저장 키 Orca는 유지하면서 표시 이름을 바꿨다. 생성 요청은 input 단계 violence 차단으로 실패했다. [응답 원문](../../Art/Aircraft/parts/user-20261108/cheyenne-generation-error.json). 새 Cheyenne 그림은 없으며 이전 오르카 그림을 새 도안 적용본으로 보고하지 않는다.
