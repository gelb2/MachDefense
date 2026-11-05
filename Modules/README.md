# 공용 모듈

| 출력 | 소스·자원 | 역할 |
|---|---|---|
| `MachDefenseCore` | `Sources/Core` | 게임 규칙·메뉴·HUD·저장 직렬화·그리기 명령 |
| `MachDefenseAudio` | `Sources/Audio` | PCM 효과음 버퍼·재생 source 재사용·컨텍스트 수명 |
| `MachDefenseAssets.bundle` | `Resources` | Images·Audio·RawData·Tables·Generated |

Apple 워크스페이스는 `project.yml`로 세 출력을 생성하고 앱에 연결한다. CMake의 코어 파일 목록은 [core-sources.cmake](core-sources.cmake)에서 관리한다. XcodeGen은 같은 Core 디렉터리를 사용한다. `python3 tools/check_modules.py`가 파일 목록·플랫폼 헤더·OpenAL 출처 해시를 검사한다.

Core는 SDK를 참조하지 않는다. `Contracts/Game.hpp`의 ResourceAccess·HostServices로 자원 읽기, 저장, 음향, 이름 입력을 요청한다. OpenAL 효과음 구현은 Audio에 둔다. Apple 실행기는 텍스처 업로드·AudioFile·AudioQueue·생명주기·파일 경로를 처리한다.

포팅판에서 사용한 OpenAL Soft 1.22.2의 출처·해시는 `ThirdParty/openal-soft-source.json`, 라이선스는 `ThirdParty/openal-soft/COPYING`에 있다. `tools/build_openal.py`가 Apple 빌드 전에 SDK별 arm64 라이브러리를 만든다.

원본 보관용 OriginalImages·IPA·실행 바이너리는 앱 번들에 포함하지 않는다. Android·웹·Tauri 실행기는 후속 과제다. [분리와 검증 기록](../docs/evidence/module-work-20261105.md).
