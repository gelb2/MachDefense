# 사용자 플레이 영상·이미지 정리 — 2026-11-07

HeavyMach1의 `docs/media-publication.md`와 `tools/extract_gameplay_media.py`에서 원본 보존·장면 시각·GIF/MP4·README 배치 절차를 참고했다. [공개 캡처 모음](../public/gameplay-gallery.md) · [English](../en/public/gameplay-gallery.md).

## 원본과 선정

바탕화면에서 녹화 2개, 이미지 8개를 확보했다. `IMG_1129.PNG`와 `IMG_1129 2.PNG`는 SHA-256이 같아 한 장만 추출했다. 원본 이름·해시·크기는 [장면 명세](../public/media/gameplay-20261107/scenes.json)에 있다. 원본 보관 폴더는 저장소 루트의 `.local/media-originals/20261107/`이며 Git 제외를 확인했다. 바탕화면 원본을 이동·삭제하지 않았다.

두 영상은 1260×2736, 252.378초와 255.853초다. 전체 구간을 15초 간격으로 추출해 장면을 확인했다. 후보 구간은 1초 간격으로 다시 확인했다. Stage 18의 포탑·메크 교전 158–170초, Stage 39의 기존 특수공격 지정·발사 232–242초, 긴급지원 수령 40–49초를 골랐다. 게임 화면이나 HUD를 자르거나 덧그리지 않았고 재생 속도를 유지했다. 원본 녹화의 Dynamic Island도 그대로 남았다.

정지 이미지 11장, GIF 2개, MP4 3개를 생성했다. GIF는 420×912·12fps·8초이며 각각 약 4.5MiB·5.2MiB다. MP4는 630×1368·30fps·H.264/AAC이며 12초·10초·9초다. [GitHub의 첨부 안내](https://docs.github.com/en/get-started/writing-on-github/working-with-advanced-formatting/attaching-files)에서 H.264 권장을 확인했다. 파일은 저장소의 문서 자원에 보관하고 README에는 GIF·정지 이미지와 MP4 링크를 넣었다.

HUD 덮개와 길찾기 제보 사진은 촬영 당시 문제를 설명하는 별도 절에 뒀다. 정확한 앱 빌드 번호는 기록되지 않아 최신 v0.5 검증 자료라고 표시하지 않았다. 미구현 락온 기능과 기존 특수공격을 구분했다.

## 검사

- 원본 10개를 보관 사본과 SHA-256으로 대조했다. 파생 파일 16개의 해시·크기·해상도·형식을 명세에 기록했다.
- MP4 3개와 GIF 2개의 전체 디코딩, 길이, 화면 비율을 검사했다. MP4의 음향 트랙 존재를 확인했다.
- 로컬 Chrome에서 이미지 13개(정지 11·GIF 2) 로드와 MP4 3개의 실제 재생 시간 증가를 확인했다. 스크립트 오류 0건, 390px 화면 가로 넘침 없음. 데스크톱·모바일 화면과 GIF 중간 프레임을 확인했다.
- 한·영 캡션·장면 시각·파일 경로를 대조하고 번역 목록을 30쌍으로 갱신했다.

재추출 도구는 [extract_gameplay_media.py](../../tools/extract_gameplay_media.py)다. Sass 담당 세션에는 [집필 안내](../editorial/sass-publication-guide.md)와 허브 서신으로 같은 장면 ID·시각·캡션·파일 경로를 전달한다. 게임 코드·배포 자원·저장 형식은 변경하지 않는다.
