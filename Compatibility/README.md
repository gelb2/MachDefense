# MachDefense iOS 호환 실행판

<!-- language-nav:start -->
[한국어](README.md) | [English](README.en.md)
<!-- language-nav:end -->

원본 Mach Defense 1.0.5를 현재 iPhone·iPad에서 비교 실행하는 앱이다. 원본 ARMv7 코드를 touchHLE로 실행한다. 소스 재구현 제품과 구분한다.

원본은 저장소 밖에서 읽으며 SHA-256을 확인한다. 원본을 포함한 앱·빌드 폴더도 저장소 밖에 둔다. IPA와 이 비교 앱은 GitHub·보고 사이트·릴리스 자산에 올리지 않는다.

## 구성

- `ios/MachDefenseCompatibility.xcodeproj`: iOS arm64 앱. 표시 이름 MachDefense Port, 번들 ID com.gelb2.machdefense.compatibility.
- `bootstrap-local.py`: 확인된 HeavyMach 1 호환 실행기에서 별도 사본을 만들고 MachDefense 패치를 적용한다. HeavyMach 1 사본은 수정하지 않는다.
- `upstream.json`, `patches/`: 런타임·SDL 커밋과 원본 SHA, 호환 수정.
- `build-runtime.sh`, `package-local.py`: 인터프리터 빌드와 로컬 앱 포장. JIT와 MoleWorld 전용 기능은 사용하지 않는다.
- `check-decimal.py`: 업그레이드 증감값을 포함한 정수 문자열 320개 조합을 macOS snprintf와 대조한다.
- `check-random.py`: 원본 표 해독에 필요한 Darwin 난수열을 macOS와 대조한다.
- `MachDefense_options.txt`: 원본 iPhone 논리 화면 크기로 실행한다. 현재 iPad에서도 같은 원본 화면을 비율에 맞춰 표시한다.

## 이 Mac의 준비·빌드

원본과 HeavyMach 1 런타임 위치를 확인한 뒤 다음처럼 별도 빌드 경로를 지정한다. 기존 경로가 있으면 스크립트는 덮어쓰지 않는다.

```sh
python3 Compatibility/bootstrap-local.py \
  --heavymach-compatibility /path/to/HeavyMach/Compatibility \
  --originals-root /path/to/heavy-mach-archive_202406 \
  --build-root /path/outside/git/MachDefense-build
python3 Compatibility/check-random.py
```

Xcode에서 `ios/MachDefenseCompatibility.xcodeproj`를 열고 MachDefenseCompatibility 스킴과 실제 기기를 선택해 빌드한다. 기기 선택 뒤 도구막대의 이름을 확인한다. 서명 설정에는 해당 Mac의 개발자 팀을 사용한다. 원본 경로·도구 경로는 Git 제외된 `.local/compatibility.json`에 저장된다.

시뮬레이터 예시:

```sh
xcodebuild -project Compatibility/ios/MachDefenseCompatibility.xcodeproj \
  -scheme MachDefenseCompatibility -destination 'generic/platform=iOS Simulator' \
  -derivedDataPath /path/outside/git/DerivedData CODE_SIGNING_ALLOWED=NO build
```

## 호환 수정과 원본 근거

- ADBannerView: 원본 NIB의 광고 클래스 호출을 받아 광고를 표시하거나 요청하지 않는다.
- QTable initWithFile:의 ARMv7 주소 `0x3369c`: EncTbl 표를 `srandom(0xb27f7)`과 `random()%227`로 해독한다. 기존 실행기의 xorshift 난수는 다른 결과를 냈다. [Apple Libc의 random.c](https://github.com/apple-oss-distributions/Libc/blob/Libc-594.9.5/stdlib/FreeBSD/random.c)를 참고해 같은 난수열을 구현했다.
- UIScreen displayLinkWithTarget:selector:: 원본이 사용하는 화면 갱신 API를 기존 CADisplayLink 구현에 연결했다.

- UIGraphics 이미지 컨텍스트: 원본 ImageAttacher가 유닛 썸네일을 회전·합성하는 API를 구현했다.
- CGImage/CGContext의 빈 인수 처리: 실제 Core Graphics 결과와 맞춰 원본의 더미 부품 처리에서 종료되지 않도록 수정했다.
- UIAlertView: 원본이 추가한 이름 입력칸을 표시하고 입력을 전달한다. 포커스 후 위치 이동은 아직 검사 중이다.
- 정수 문자열: `%+d`의 양수 부호와 정밀도·필드 폭을 처리한다.
- Xcode 패키지 단계: Stub 링크 대신 스크립트가 실행 파일을 생성하고 이를 출력으로 선언한다. 이후 CodeSign이 실행 파일을 서명한다.

실행·설치 결과와 남은 문제는 [현재 상태](../docs/current-status.md)에 기록한다. 프레임 주기나 게임 밸런스를 별도로 조정하지 않는다.

## 판본별 시뮬레이터 비교 앱

`validation/package-version.py`는 기존 시뮬레이터 wrapper, 빌드한 runtime, 외부 원본 경로를 받아 별도 `com.gelb2.machdefense.audit.indie105` 또는 `.wemade103` 앱을 만든다. 입력 SHA-256과 플랫폼을 검사하고 기존 출력 덮어쓰기를 거부한다. 원본·runtime·앱 출력은 모두 저장소 밖에 둔다. `--help`에 인수가 있다. 일반 앱이나 실기기 저장에 설치하지 않는다.

`validation/save_fixture.py`는 indie 42,476바이트와 WeMade 42,496바이트 저장을 구분한다. 생성한 자금 시험 저장은 해당 audit 앱이 종료된 동안에만 넣는다. 먼저 앱에서 만든 저장을 `.local`에 보관한다. 두 판본의 저장을 교환하지 않는다.

위메이드 시작 시 frameInterval 0 처리와 GuiBanner 광고 차단을 추가했다. [분석·실행 근거](../docs/evidence/version-audit-20261104.md).
