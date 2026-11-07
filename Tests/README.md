# Tests

<!-- language-nav:start -->
[한국어](README.md) | [English](README.en.md)
<!-- language-nav:end -->

공용 C++ 게임 검사와 자원·문서 도구 검사를 둔다. 생산·피해·강화·80미션 진행·저장·복원 코드를 검사한다. 실행 명령은 [공용 코어](../Sources/Core/README.md)에 있다.

플랫폼 렌더링·입력·오디오·생명주기 검사는 [Apple](../Apple/README.md), [Android](../Android/README.md), [웹](../Web/README.md)에서 실행한다. 실제 수행한 범위와 남은 검사는 [검사 결과](../docs/public/validation.md)에 기록한다.

문서 링크와 한·영 대응 검사:

```sh
python3 tools/document_contract.py check
python3 tools/check_translations.py
python3 -m unittest discover -s Tests -p 'test_*translations*.py'
```
