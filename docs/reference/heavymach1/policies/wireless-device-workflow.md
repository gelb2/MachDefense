# iPhone·iPad 무선 설치와 디버깅

빌드는 Mac의 Xcode가 수행한다. 같은 네트워크에 있는 페어링된 iPhone·iPad로 앱을 전송하고 실행·디버깅한다. 이 프로젝트는 `HeavyMach.xcworkspace`와 기존 iOS target을 그대로 사용한다.

## 연결 확인

Xcode27.1의 Device Hub에서 기기를 선택한다. 이미 페어링된 기기는 신뢰·개발자 모드 설정을 반복하지 않는다. 최초 연결이나 재부팅 직후에는 기기의 잠금 해제 상태를 확인한다. 충전은 유지할 수 있지만 무선 검사에서는 Mac의 USB 데이터 경로를 사용하지 않아야 한다.

```sh
xcrun devicectl list devices
xcrun devicectl device info details --device <확인한-기기-ID> --json-output build/device-details.json
xcrun devicectl device info lockState --device <확인한-기기-ID>
```

`result.connectionProperties.transportType`이 `localNetwork`, `tunnelState`가 `connected`인지 확인한다. 설치·디버깅 전후에 다시 기록한다. 단순히 기기 이름을 지정하거나 Xcode에 나타난다는 이유로 무선이라고 판정하지 않는다.

## 빌드·설치

일반 앱은 `HeavyMachRestored` / `com.gelb2.heavymach.restored`, 치트 앱은 `HeavyMachValidation` / `com.gelb2.heavymach.validation`이다. Xcode에서 스킴과 실제 기기를 선택한 뒤 Run으로 빌드·설치·디버거 연결을 수행한다. 이때 사용하는 구성은 개발 서명이 적용된 Debug다. 정식 앱은 치트 기능을 뺀 개발 서명 앱이다.

기존 서명 앱을 다시 설치할 때는 다음 명령을 사용한다. 앱을 먼저 삭제하지 않는다.

```sh
codesign --verify --deep --strict <서명된-app-경로>
xcrun devicectl device install app --device <기기-ID> <서명된-app-경로> --json-output build/install-result.json
xcrun devicectl device process launch --device <기기-ID> com.gelb2.heavymach.restored --json-output build/launch-result.json
```

CLI 빌드가 `errSecInternalComponent`로 서명에 실패한 경우, 이 환경에서는 같은 프로젝트를 Xcode GUI로 빌드하면 기존 키체인 권한으로 성공했다. 암호를 로그에 쓰거나 ad-hoc 서명으로 바꾸지 않는다. 컴파일·서명·설치 실패를 구분한다.

## LLDB 검증

Xcode Run의 기기 이름과 앱 스킴을 확인한다. Pause로 중단하고 다음 one-shot 중단점을 설정한다.

```text
breakpoint set --one-shot true --name "-[HMGameView frame:]"
continue
```

`HMGameView.mm`의 프레임 함수에서 중단되는지 확인한다. Debug navigator의 호출 스택과 Variables의 `self`를 읽고 Continue로 재개한다. 콘솔에서 `p self`도 조회할 수 있지만 객체 전체 출력이 길어질 수 있다. 명령별 응답을 기다리고 다음 프롬프트에서 입력한다. Xcode 자동 완성이 Return을 소비해 명령 두 개가 합쳐진 사례가 있어, 오류가 나면 실제 입력 문자열부터 확인한다.

검사 뒤 만든 중단점만 정리한다. 정식 앱의 게임 저장을 초기화하거나 다른 세션의 중단점을 지우지 않는다.

## XCTest와 Instruments

Xcode Product → Build For → Testing으로 서명된 테스트 묶음을 만든다. Run용 빌드 뒤에는 `.app/PlugIns/HeavyMachRenderTests.xctest`가 빠질 수 있으므로 경로를 확인한다. `.xctestrun`이 있다는 이유만으로 준비됐다고 판단하지 않는다.

```sh
xcodebuild -workspace HeavyMach.xcworkspace -scheme HeavyMachValidation \
  -destination 'id=<기기-UDID>' -derivedDataPath <실제-DerivedData> \
  -parallel-testing-enabled NO \
  -only-testing:HeavyMachRenderTests/HMAppleInputTests \
  -only-testing:HeavyMachRenderTests/HMFramePerformanceTests/testProductionTimerAndFullScreenTouchViewport \
  -resultBundlePath build/wireless-input.xcresult test-without-building
```

실제 테스트 묶음 생성 오류는 테스트 시작 실패다. 게임 assertion 실패와 구분해 기록한다. 반복 실행은 새 결과 경로를 사용해 이전 실패를 보존한다.

```sh
xcrun xctrace record --template 'Time Profiler' --device <기기-ID> \
  --time-limit 20s --output build/wireless-cpu.trace \
  --launch -- com.gelb2.heavymach.validation
xcrun xctrace export --input build/wireless-cpu.trace --toc --output build/wireless-cpu-toc.xml
```

기록·내보내기 성공과 trace에 실제 대상·CPU 표본이 있는지 확인한다. 위20초 기록은 연결 검사다. 장시간 발열·안정성 검사로 보고하지 않는다. 프레임 성능 검사는 debugger/Instruments를 떼고 별도로 실행한다.

## 검사 후 앱 종료·홈 화면 복귀

2026-10-29 사용자 요청에 따라 설치·실행·자체 검사의 마지막 단계에 앱 종료와 기기 홈 화면 복귀를 포함한다. 사용자가 실행 상태를 유지하라고 요청하면 앱을 켜 둔다. 여기서 홈 화면은 iPhone·iPad의 앱 아이콘 화면이다.

먼저 검사 결과와 로그·화면·trace를 저장한다. 입력을 해제하고 게임을 일시정지한 뒤, 사용 가능한 기기 UI 도구로 홈으로 이동해 앱을 백그라운드로 보낸다. 일반 앱의 저장 데이터는 유지한다. 저장 완료 여부를 확인할 수 없으면 저장 확인으로 기록하지 않는다.

이번 작업에서 연결한 debugger·Instruments·테스트 실행을 끝낸 다음 대상 앱의 현재 프로세스를 조회한다. launch 결과의 PID와 현재 실행 경로를 대조한다. PID가 바뀌었다면 새 프로세스의 앱 경로를 다시 확인한다.

```sh
xcrun devicectl device info processes --device <기기-ID> --json-output <실행별-경로>/processes-before-stop.json
xcrun devicectl device process terminate --device <기기-ID> --pid <현재-확인한-앱-PID> --json-output <실행별-경로>/terminate.json
xcrun devicectl device info processes --device <기기-ID> --json-output <실행별-경로>/processes-after-stop.json
```

`terminate`는 기본 종료 신호를 사용한다. 일반 종료 실패 시 오류와 현재 프로세스를 다시 확인하고 대응한다. `--kill`을 기본값으로 쓰지 않는다. 앱을 삭제하거나 사용자 저장을 지우지 않는다. 위 명령 형식은 설치된 Xcode의 `devicectl --help`에서 확인했다.

종료 후 대상 앱이 실행 목록에서 사라졌는지 확인한다. 홈 화면은 사용 가능한 기기 화면 캡처 또는 UI 도구로 따로 확인한다. `devicectl`의 프로세스 종료 응답은 화면 확인을 대신하지 않는다. 홈 이동 도구가 없으면 앱 종료를 먼저 수행하고, 그 결과 실제 홈 화면이 표시됐는지 확인한다. 화면을 읽을 수 없으면 홈 화면 복귀는 미확인으로 남긴다. 홈 화면 캡처는 로컬 검사 자료로만 보관한다.

기기·앱별 결과에 다음을 남긴다.

| 항목 | 기록 |
|---|---|
| 설치·실행 | 소스/빌드 식별, 앱 종류, 실제 transport, 설치·실행 결과 |
| 자체 검사 | 실행한 항목, 통과·실패·미실행, 로그·xcresult 경로 |
| 종료 | 대상 앱 식별, 종료 결과, 종료 후 프로세스 확인 |
| 최종 화면 | 홈 화면 확인 결과와 로컬 근거, 또는 확인하지 못한 이유 |

자동 검사 흐름은 실패하더라도 로그를 보존한 뒤 종료 단계를 실행한다. 검사 실패와 종료 실패를 별도로 기록한다. 종료 후 상태 확인이 끝났으면 검증 앱이나 일반 앱을 다시 실행해 두지 않는다.

## 조사한 스킬과 채택한 방법

설치된 스킬 목록에 Xcode 무선 기기 전용 스킬은 없었다. 외부 후보로 Sentry의 MobileBuildMCP 공식 `mobilebuildmcp`와 `mobilebuildmcp-cli`를 읽었다. 기존 XcodeBuildMCP 저장소 주소는 조사 시 MobileBuildMCP로 연결됐다. 검토한 소스 커밋은 `2998ac77ebbca59544e88a27f34bfb830db9d924`다.

이 도구의 기기 목록은 devicectl의 `wired`·`localNetwork`를 읽으며, 설치·실행·검사 도구를 제공한다. 조사한 LLDB attach 도구 `debug_attach_sim`은 Simulator용이다. 이를 실제 기기의 무선 LLDB 지원 증거로 쓰지 않는다. 해당 외부 실행 파일은 현재 설치돼 있지 않으며, 이번에는 기존 Xcode·devicectl·CUA로 실제 기기를 검증했다. 실행한 절차와 이 환경의 오류 해결을 프로젝트의 [apple-wireless-device 스킬](../.agents/skills/apple-wireless-device/SKILL.md)에 정리했다.

[MobileBuildMCP 공식 저장소](https://github.com/getsentry/MobileBuildMCP), [CLI 스킬](https://github.com/getsentry/MobileBuildMCP/blob/2998ac77ebbca59544e88a27f34bfb830db9d924/skills/mobilebuildmcp-cli/SKILL.md), [기기 목록 소스](https://github.com/getsentry/MobileBuildMCP/blob/2998ac77ebbca59544e88a27f34bfb830db9d924/src/mcp/tools/device/list_devices.ts), [Simulator attach 소스](https://github.com/getsentry/MobileBuildMCP/blob/2998ac77ebbca59544e88a27f34bfb830db9d924/src/mcp/tools/debugging/debug_attach_sim.ts).

Apple의 현행 진입점은 [Device Hub](https://developer.apple.com/documentation/xcode/managing-your-simulated-and-physical-devices-in-device-hub)다. [무선 실행 안내](https://help.apple.com/xcode/mac/current/en.lproj/dev3e2f4ee6d.html)와 [페어링 안내](https://help.apple.com/xcode/mac/current/en.lproj/devbc48d1bad.html)는 네트워크 실행 원리를 설명하지만 구형 Devices and Simulators 메뉴를 사용하므로 현재 UI와 구분한다.

## 2026-10-27 실제 검증 결과

두 기기의 설치·디버깅 전후 transport가 localNetwork였다. iPhone 일반 앱의 무선 재설치·실행, iPad 일반·치트 앱의 무선 설치·실행, 두 기기의 LLDB 중단점·변수 조회·재개를 확인했다. iPad 선택 검사8개가 모두 통과했고 Time Profiler21.716초 기록과 CPU 표본10,202개 내보내기가 성공했다. iPhone 무선 XCTest·Instruments는 이번에 실행하지 않았다. iPad는 사용자 재부팅 후 연결 상태였고 추가 재부팅 검사는 하지 않았다.

기존 표시시간 실패는 유지한다. [상세 결과](public/apple-platform-progress-20261026.md#iphoneipad-무선-설치디버깅--2026-10-27), [증거 JSON](evidence/wireless-apple-20261027.json).
