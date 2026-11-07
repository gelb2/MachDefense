# 제작자와 사용 자원

<!-- language-nav:start -->
[한국어](CREDITS.md) | [English](CREDITS.en.md)
<!-- language-nav:end -->

## 원작

MachDefense의 그림·음악·게임 설계는 원작 제작진의 작업이다. 이 복원은 당시 게임을 다시 실행하고 다른 플랫폼에서 플레이할 수 있도록 만드는 프로젝트다.

## 복원과 추가 개발

나는 게임을 직접 플레이하며 복원 방향과 보존할 조건, 추가 기능을 정했고 잘못된 동작의 수정을 지시했다. 새 소스와 플랫폼 연결은 Codex로 구현했다.

HeavyMach1에서 사용한 복원 절차와 스킬, 공통 코드·플랫폼 실행기 구성, 웹·PWA 확장 과정을 참고했다. HeavyMach1의 웹판 외부 기여와 후속 수정 기록은 해당 저장소에 남아 있다. [HeavyMach1](https://github.com/gelb2/HeavyMach)

## 먼저 복원을 시도한 개발자

[mesabloo](https://github.com/Mesabloo/hm-defense)는 나보다 먼저 MachDefense 복원을 시도했다. 그의 시도는 HeavyMach1과 MachDefense 복원에 큰 영향을 주었다.

## 라이브러리

OpenAL Soft는 효과음 재생, stb는 이미지 디코딩에 사용한다. 각 라이브러리의 소스와 라이선스는 `Modules/ThirdParty/`에 있다. [외부 고지](THIRD_PARTY_NOTICES.md) · [코드 라이선스](LICENSE-SCOPE.md) · [자원 조건](ASSET-LICENSES.md).

HeavyMach의 웹판을 기여한 [fastutlego](https://github.com/fastutlego)에게 감사드린다. [HeavyMach PR #24](https://github.com/gelb2/HeavyMach/pull/24)의 WebAssembly 실행부·WebGL2 렌더링·입력·음향·저장 구현과 그 뒤의 PWA·Tauri 확장 절차가 MachDefense를 웹과 데스크톱으로 확장하는 데도 토대가 됐다. MachDefense의 PWA를 이 구조를 참고해 만들었고, 같은 웹 실행기를 Tauri Mac 앱에서도 사용했다.

[게임 소개](README.md) · [문서 목차](docs/public/README.md)
