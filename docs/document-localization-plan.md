# GitHub 한·영 문서와 Sass 전달 계획

2026-11-07 사용자 요청. HeavyMach1의 문서별 언어 선택·번역 대응 목록·집필 자료 인계 구조를 MachDefense에 적용한다. 방문자 문서 29쌍과 제목 아래 언어 전환 링크를 작성했다. 원문·영문 해시와 명령·링크 검사도 추가했다. [작성·검사 기록](evidence/document-localization-20261107.md).

## 대상과 경로

GitHub 방문자가 게임을 이해하고 빌드·설치·플레이하며 개발 동기·기획·기술·수정 과정·남은 일을 읽는 문서는 모두 한·영으로 제공한다. [번역 대상 목록](translations/scope.json)에 각 원문과 번역 경로를 기록한다. 플랫폼·모듈 폴더의 이용 안내도 포함한다.

| 한국어 | 영어 |
|---|---|
| `README.md`, `CHANGELOG.md`, `CREDITS.md` | 같은 위치의 `README.en.md`, `CHANGELOG.en.md`, `CREDITS.en.md` |
| `docs/RESTORATION.md` | `docs/en/RESTORATION.md` |
| 방문자용 `docs/public/*.md` | `docs/en/public/`의 같은 이름 |
| Apple·Android·Web·모듈 등의 `README.md` | 같은 폴더의 `README.en.md` |

현재 영어만 있는 모듈 안내는 한국어판도 작성한다. 라이선스 원문·코드·명령·그림·영상은 공유하며 캡션·대체 텍스트를 번역한다. 원본 라이선스 문구를 번역문으로 대체하지 않는다.

개발 정본·정책·스킬·사용자 명령·인터뷰 원문·내부 TODO·검사 로그·과거 보고 원고는 개발 기준과 증거로 유지한다. 방문자가 읽을 남은 과제와 현재 구현 상태는 별도 공개 설명으로 정리해 번역한다. 공개 설명을 개발 정본 대신 적용하지 않는다.

## 언어 전환

각 문서 제목 아래에 **한국어 | English** 링크를 둔다. GitHub에서 누르면 같은 문서의 해당 언어판으로 이동한다. 별도 웹 UI나 스크립트는 필요하지 않다.

영어 README → 영어 목차 → 영어 설치·기능·기술·개발자의 변으로 이어지게 한다. 영어판의 절 링크는 번역된 제목과 맞춘다. 기존 한국어 주소와 앵커는 보존한다. 번역이 없는 내부 검사 기록을 연결할 때는 근거 자료임을 구분한다. 아직 없는 영어판으로 연결하는 버튼은 먼저 달지 않는다.

## 집필과 검사 순서

1. 최신 origin/main과 작업 브랜치의 원문을 비교한다. 사용자가 직접 고친 개발자의 변과 README 문장을 우선한다.
2. 대상 목록을 확정하고 방문자용 구현 상태·남은 과제를 작성한다. 현재 목차의 과거 릴리스 초안은 현행 안내와 구분한다.
3. README·설치·개발자의 변부터 번역하고 나머지 대상 문서를 번역한다. 저자의 인칭·감정·비판·비유와 수치·기능 상태를 보존한다. 국영문 모두 메타카피·컴플라이언스 문체·자기합리화 문장을 넣지 않는다.
4. 국문·영문 경로, 검토한 원문과 번역의 해시를 매니페스트에 기록한다. 원문이 바뀌면 해당 영어판을 재검토 대상으로 표시한다.
5. HeavyMach1의 번역 검사기를 참고해 누락·양방향 언어 링크·영문 내부 링크·앵커·이미지·표·수치·명령을 검사한다. 자동 검사는 문단의 의미와 저자 어조 대조를 대신하지 않는다.
6. GitHub 렌더 화면에서 README부터 두 언어의 설치·기능·개발자의 변으로 이동하고 왕복한다. 모바일 폭에서도 링크와 그림을 확인한다.
7. 별도 문서 브랜치의 변경을 확인하고 develop→main으로 반영한다. 공개 원고 변경 시 대응 번역과 Sass 자료 안내도 갱신한다. 내부 개발 문서만 바뀐 작업에 전체 번역을 선행 조건으로 걸지 않는다.

## Sass 세션의 진입점

[기존 Sass 집필 안내](editorial/sass-publication-guide.md)를 계속 갱신한다. Apps용 기술 설명과 Mags용 인터뷰·저자 원문, 채택 이미지·검사 근거·미완료 기능·한영 대응을 한곳에서 찾게 한다. Sass 허브에는 이 안내와 정확한 작업 사본·브랜치·게시 상태를 연결하는 짧은 서신을 둔다. Apps·Mags 집필과 sass.studio 수정·배포는 Sass 담당 세션이 수행한다.

## 참고한 HeavyMach1 구조

로컬 대조 사본은 HeavyMach의 `build/pr24-review`다. 문서 구조를 참고했으며 HeavyMach 파일은 수정하지 않았다.

- `docs/document-localization-plan.md`: 방문자용 번역 범위와 개발 기준 보존.
- `docs/translations/manifest.json`, `tools/check_translations.py`: 대응 경로·해시·언어 링크 검사.
- `docs/public/publication-inventory.md`: 주제별 원고·이미지·근거 목록.
- `docs/editorial/web-tauri-sass-handoff-20261102.md`: Apps/Mags용 주제·사용자 의도·구현·검사 연결.
- Sass 허브의 `governance/operations/heavymach-web-tauri-handover-2026-11-02.md`: 집필 세션에 전달하는 서신.

HeavyMach의 Tauri 출시 상태·저장 슬롯 수·기여자 기록을 MachDefense의 사실로 옮기지 않는다. MachDefense는 전체 5슬롯 MD1 복원을 쓰며 Tauri 앱은 아직 없다.
