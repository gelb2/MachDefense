# 개발 환경

Python 3, Git, GitHub CLI로 준비 상태를 검사한다. 새 clone 후 다음 명령으로 로컬 훅을 연결한다.

```sh
python3 tools/setup.py
python3 tools/document_contract.py check
python3 -m unittest discover -s Tests/tools -v
python3 tools/original_guard.py --history
python3 tools/verify_originals.py --root /absolute/path/to/heavy-mach-archive_202406
```

원본 경로는 `.local/originals.json`에 기록하며 Git에서 제외한다. 원본 파일은 복사하지 않는다. `tools/verify_originals.py`는 읽기·해시 검사만 한다.

REA 스킬이 설치된 것과 MCP 서버·분석 엔진 연결은 별개다. 이 세션에서는 REA 도구가 제공되는 것을 확인했지만 MachDefense 바이너리를 열거나 디컴파일하지 않았다. 실제 분석 때 로컬 도구 연결과 입력 SHA를 기록한다. 외부 업로드가 필요한 분석기는 사용하지 않는다.

Xcode·Android Studio·Emscripten·Tauri의 설치·버전은 해당 플랫폼 구현 시 확인한다. 현재 빌드 가능한 MachDefense 타깃은 없다. HeavyMach 1 앱을 MachDefense 빌드로 대신하지 않는다.
