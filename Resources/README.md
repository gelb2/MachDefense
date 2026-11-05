# Resources

위메이드 MachDefense 1.0.3의 게임 자원을 소스 빌드용으로 준비했다. 앱 빌드는 이 폴더의 자원을 읽는다. 준비 도구는 빌드 단계에 연결하지 않는다.

| 경로 | 내용 |
|---|---|
| `Images/` | 이미지 1,109개. CgBI PNG 948개를 일반 RGBA PNG로 변환하고 JPEG 161개는 그대로 복사 |
| `OriginalImages/` | 변환 전 이미지. 픽셀·해시 비교용 |
| `Audio/` | 원본 WAV 77개와 MP3 3개 |
| `RawData/` | MCB 3개, 암호화 표 94개, 일반 표 1개 |
| `Tables/` | UTF-8 TSV 94개. `MachBuild_Parts.tbl`과 `.tble`의 해독 내용이 같아 한 파일로 사용 |
| `Data/` | MCB의 노드 주소·종류·원시 필드·자식 관계를 기록한 JSON 3개 |
| `Generated/` | 원작 `TrebuchetMS-Bold`의 ASCII 글리프 아틀라스·배치·폭 정보와 타이틀 확장 그림·출처 JSON |

`inventory.json`은 원본 자원과 준비한 파일의 SHA-256을 연결한다. `images.json`에는 크기·알파·변환 경로가 있고 `tables.json`에는 표별 헤더·행 수가 있다. `preparation-validation.json`은 자원 검사 결과다.

준비 명령:

```sh
python3 tools/prepare_resources.py --source /outside/repository/MachDefense.ipa
xcrun swift tools/prepare_font.swift Resources/Generated
```

준비 도구는 지정한 위메이드 파일의 SHA-256을 확인한다. 게임 자원 확장자만 허용하며 원본 IPA·실행 바이너리·NIB·서명·광고 SDK를 복사하지 않는다. 원본은 저장소 밖에 둔다.

PNG 변환은 BGRA 채널, premultiplied alpha, Adam7 인터레이스를 처리한다. 원본 6개 인터레이스 이미지와 일반 이미지 4개의 변환 결과를 macOS ImageIO와 대조했고 RGBA 픽셀이 모두 같았다. 반투명 픽셀의 원본 RGB는 CgBI에 이미 양자화되어 있으므로 변환 전 파일도 보관한다.

스폰 표 80개의 한글 주석은 CP949이며 UTF-8로 변환했다. 영어 설명 표는 `Description_us.tsv`다. 이 주석 변환은 미션·무기·생산 수치를 바꾸지 않는다.

글리프 아틀라스는 WeMade ARMv7의 CFString `0x6aaac`에서 읽은 `TrebuchetMS-Bold`를 사용한다. 설치된 동일 PostScript 이름을 확인해 오프라인으로 생성했다. 폰트 파일을 복사하거나 게임 실행 중 글자 이미지를 생성하지 않는다. 원작의 숫자 이미지 폰트도 `Images/`에 유지한다.

타이틀 확장 그림은 원본 패키지 자원이 아닌 추가 생성물이다. [출처·합성 기준](Generated/TitleOutpaint.json)과 [수정 이력](../docs/evidence/hud-title-20261105.md)을 따른다. 사용자 확인에서 경계선이 남아 현재 미해결이며, 원본 Title.jpg는 유지한다.
