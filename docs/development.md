# 개발 환경

Apple Silicon Mac·Xcode·CMake·Ninja·XcodeGen·Python 3를 사용한다. iPhone·iPad용 일반 앱과 치트 앱은 하나의 Apple 워크스페이스에서 빌드한다. 현재 대상은 arm64다.

```sh
python3 tools/setup.py
Apple/generate-projects.sh
open Apple/MachDefense.xcworkspace
```

Xcode에서 `MachDefense`는 일반 앱, `MachDefenseCheat`는 치트 앱이다. 시뮬레이터를 고르고 실행한다. 실기기는 개발 팀과 서명을 설정한다. `MachDefenseRendererTests`는 치트 앱을 호스트로 실제 렌더러·오디오·시험 저장을 검사한다.

OpenAL Soft는 `tools/build_openal.py`가 SDK별로 자동 빌드한다. 생성 파일은 `.local/openal`에 둔다. 게임 자원은 Resources에서 MachDefenseAssets.bundle로 복사한다. 빌드에 원본 IPA나 외부 원본 보관소를 사용하지 않는다.

## 검사

```sh
cmake -S . -B .local/source-build -DCMAKE_BUILD_TYPE=Release
cmake --build .local/source-build
ctest --test-dir .local/source-build --output-on-failure
python3 tools/check_modules.py
python3 tools/document_contract.py check
```

`xcrun simctl list devices available`에서 시뮬레이터 UDID를 확인한 뒤 실행한다.

```sh
python3 tools/run_apple_sweep.py --simulator <UDID> --mode full
python3 tools/run_apple_sweep.py --simulator <UDID> --mode soak
```

full은 모든 짧은 XCTest, soak는 10분 전투·20회 진입 검사다. quick은 구매·80미션·생산·강화·무장·짧은 전투의 6개 검사만 실행한다. 로그와 xcresult는 `.local/apple-sweep`에 보존한다. 성능 검사 때는 한 시뮬레이터만 실행한다.

## 원본 분석

REA/Ghidra 분석 파일은 저장소 밖에 보존한다. 원본 경로는 Git에서 제외한 `.local/originals.json`으로 관리한다. `tools/verify_originals.py`는 원본 읽기·해시 검사에만 사용한다. [출처](source-provenance.md), [빌드·모듈 구조](../Modules/README.md), [최근 검사](evidence/module-work-20261105.md).
