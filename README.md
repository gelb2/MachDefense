# MachDefense

<!-- language-nav:start -->
[한국어](README.md) | [English](README.en.md)
<!-- language-nav:end -->

[![Release v0.6](docs/public/media/badges/version.svg)](https://github.com/gelb2/MachDefense/releases/tag/v0.6)
[![iOS, iPadOS, Android, Web, PWA, macOS](docs/public/media/badges/platforms.svg)](docs/public/build-and-install.md)
[![arm64, WebAssembly](docs/public/media/badges/arch.svg)](docs/public/architecture.md)
[![C++20, Objective-C++, Kotlin, JavaScript, Rust](docs/public/media/badges/code.svg)](docs/public/architecture.md)
[![Code license MIT](docs/public/media/badges/license.svg)](LICENSE-SCOPE.md)

## MachDefense를 다시 만든 이유

내 처음 목표는 Mach 시리즈를 모두 복원하는 것이었다. HeavyMach1부터 시작했고, 공개한 저장소에서 생애 처음으로 PR을 받았다. 다음으로 택한 작품이 MachDefense다. 나는 캐나다와 미국 앱스토어 계정으로 이 게임을 구매해 플레이했다.

이 복원판을 접한 사람이 개발자의 변을 읽었으면 한다. 왜 이 시리즈를 다시 만들었는지, 무엇을 보존하고 무엇을 더하려 했는지 적었다.

나는 이것으로 돈을 벌 생각이 없다. 돈은 이승에서나 필요한 것이다.

[개발자의 변 전문](docs/RESTORATION.md)

## Quick Play — PWA·Mac

[**MachDefense 플레이·PWA 설치**](https://gelb2.github.io/MachDefense/)

위 주소를 열어 게임을 설치하거나 브라우저에서 바로 플레이한다. iPhone·iPad는 Safari의 공유 메뉴에서 **홈 화면에 추가**, Android·Mac은 **INSTALL GAME**을 누른다. 자원 다운로드가 끝나면 인터넷 없이도 플레이할 수 있다.

Xcode나 Android Studio로 빌드하기 어렵다면 PWA나 [Tauri Mac 앱](https://github.com/gelb2/MachDefense/releases/download/v0.6/MachDefense-Desktop-v0.6-arm64.zip)을 이용하면 된다. Mac 앱은 macOS 14 이상 Apple Silicon에서 실행하며 진행을 앱 데이터 폴더에 저장한다. [설치와 실행](docs/public/build-and-install.md)에서 네이티브 앱 설치법도 확인할 수 있다.

HeavyMach의 웹판을 기여한 [fastutlego](https://github.com/fastutlego)에게 감사드린다. [HeavyMach PR #24](https://github.com/gelb2/HeavyMach/pull/24)의 WebAssembly 실행부·WebGL2 렌더링·입력·음향·저장 구현과 그 뒤의 PWA·Tauri 확장 절차가 MachDefense를 웹과 데스크톱으로 확장하는 데도 토대가 됐다. MachDefense의 PWA를 이 구조를 참고해 만들었고, 같은 웹 실행기를 Tauri Mac 앱에서도 사용했다.

### 레벨업이 힘든 사람을 위한 방법

내 아이폰 네이티브 앱에서 직접 플레이한 진행도의 복원 코드를 공유한다. **Stage 42·보유 유닛 12종·20,329Cr** 상태에서 이어서 플레이할 수 있다. **[복원 코드와 적용 방법](docs/public/quick-play.md#레벨업이-힘든-사람을-위한-방법)**.

## 원작의 전투와 성장

MachDefense는 메크와 포탑을 생산해 적 기지를 공격하는 세로 화면 전략 게임이다. 유닛은 전장으로 나가 이동하고 교전한다. 플레이어는 생산과 포탑 배치, 강화와 특수기술 사용을 결정한다. 원작의 80개 미션과 이미지·음원·유닛 데이터를 사용한다.

나는 타워디펜스를 자주 하던 사람이 아니었다. MachDefense에 손이 간 이유도 “그 사람들 후속작인가? 신기하니 해봐야겠다”는 마음이었다. 캐나다와 미국 앱스토어 계정으로 구매했고, 내가 플레이한 게임의 유닛 이름과 화면은 영어였다.

HeavyMach1에서는 직접 전차를 조종했다. MachDefense에서는 유닛을 생산하고 교전을 지켜봤다. 폭음과 폭발이라는 전투의 기본을 관전하는 재미가 있었다. 나는 사용자의 개입을 줄이면서 교전의 화려함을 보여주는 것이 이 장르의 재미라고 생각한다.

![Android 정식 앱의 Stage 33](docs/evidence/images/android-20261106/normal-iphone-stage33.png)

오른쪽 목록에서 유닛을 고르면 셀을 사용해 생산한다. 크레딧으로 기지와 장비를 강화하고 특수기술을 보충한다. 프로필에는 미션 진행·장비·강화·자금·특수기술 잔량을 저장한다.

![보유 장비와 강화 단계](docs/evidence/images/android-20261106/normal-iphone-upgrades.png)

<img src="docs/public/media/gameplay-20261107/combat.gif" alt="포탑 사격과 메크 교전" width="360">

[12초 전투 영상](docs/public/media/gameplay-20261107/combat.mp4) · [전체 캡처와 영상](docs/public/gameplay-gallery.md)

<img src="docs/public/media/gameplay-20261107/special-attack.gif" alt="특수공격 표적 지정과 미사일 발사" width="360">

[10초 특수공격 영상](docs/public/media/gameplay-20261107/special-attack.mp4)

<a href="docs/public/media/gameplay-20261107/iphone-stage40.jpg"><img src="docs/public/media/gameplay-20261107/iphone-stage40.jpg" alt="Stage 40: 여러 무장이 동시에 교전하는 장면" width="360"></a>

## 내가 기획해 추가한 기능

긴급지원은 원작 복원과 별개로 내가 구상한 기능이다. 타워디펜스의 난관이 결국 결제로 산 아이템을 써야 넘어갈 수 있는 것으로 바뀌는 것이 마음에 걸렸다. 작동하지 않는 인앱결제 버튼을 긴급지원 창으로 바꾼 이유다.

한 번 받고 끝내는 지원을 원한 것은 아니었다. 게임 시간에 따라 소량의 크레딧과 셀을 반복해서 받도록 했다. 사용자가 결제 없이 난관을 돌파할 수 있게 하고 싶었다. 지급량은 현재 셀 용량과 생산량, 미션 보상을 기준으로 계산했다. 지급 간격·반복 수령·자원 용량을 고려한 계산을 지시했고, Codex가 공통 C++ 코드와 원작 에셋으로 구현했다.


<img src="docs/public/media/gameplay-20261107/resupply.jpg" alt="전투 중 긴급지원 수령" width="360">

[지원 수령 장면: 9초 영상](docs/public/media/gameplay-20261107/resupply.mp4)

## 헬기 유닛과 작업 기록

헬기 유닛을 릴리즈에 추가했지만, 원하는 사람만 사용했으면 한다.

토큰의 사용량과 Codex의 경악스러울 정도의 업무지시 거부 때문에, 헬기 유닛의 추가 디버깅과 디자인 개선은 단기적으로 불가능하다는 것이 내 결론이다. 이미지와 그림을 적어도 200장 깎아 보면서도 거의 겪어 보지 못한 기상천외한 거부와 태업이었다.

내 목표는 Mach 시리즈 전체의 복원이지, Codex 새끼의 비위를 맞추는 베이비시팅이 아니다. 그따위 짓을 하느라 얼마 남지 않은 내 시간을 쓰고 싶지 않다.

맥디펜스에는 아직 아이패드 개선과 락온 시스템 기획을 비롯한 퍼포먼스 개선·기획 TODO가 남아 있다. 그러나 Mach 시리즈 전체의 복원을 위해 헬기 유닛과 관련된 모든 TODO와 미완성 사항은 미뤄 두려 한다.

단순히 자기 성깔 못 이겨서 작업보류를 한다고 생각해도 좋다. 이렇게 된거 대놓고 말 하겠다, 나는 코덱스가 이런식으로 반복적으로 업무지시 거부를 하고 원인도 파악 불가능한 태업을 할때 일을 밀어붙였던 경우, 좋게 끝난 적이 없다. 턱뼈 한쪽에 감각이 없어지거나, 바이크를 타고 가다가 사고가 나거나, 말로가 좋지 않았다. ai의 지능적인 업무지시 거부는 나의 경우엔 급사의 제 1요인이며, 나에겐 아직 복원해야 할 게임이 하나 더 있다. 그러므로 헬기 유닛 개선은 당분간 멈춘다

[헬기 작업에 대한 기록 - 내가 간과한 것. 실리콘밸리에서 DEI는 수정헌법5조보다 강력하다는 사실.](docs/editorial/helicopter-work-record-20261109.md) · [헤비멕1과는 다른 위화감](docs/public/unease-and-typewriters.md) · [남은 과제](docs/public/roadmap.md)

## 화면·플랫폼 확장과 검증 도구

- iPhone·iPad 화면에 맞춘 전장·메뉴·HUD 표시 영역.
- 같은 C++ 게임 코드와 자원을 사용하는 Android·웹 실행기.
- iPhone·iPad·Android·웹/PWA 사이에서 전체 프로필 5개를 한 줄 코드로 옮기는 기능. [사용법](docs/public/gameplay.md).
- 저장을 분리한 일반 앱과 치트 검증 앱.

[추가 기능과 기획 의도](docs/public/features.md) · [조작과 저장](docs/public/gameplay.md)

## 플랫폼과 실행 상태

| 플랫폼 | 실행 | 현재 상태 |
|---|---|---|
| iPhone·iPad | Xcode 워크스페이스 | v0.6. iPhone·iPad 정식·치트 설치, 두 기기의 강화·복원·생산 HUD 검사 |
| Android | Android Studio 또는 APK | [v0.6 APK](https://github.com/gelb2/MachDefense/releases/tag/v0.6). Galaxy 정식·치트 설치, 생산·강화·음향·복귀·승패 검사 |
| 웹 | 브라우저 | GitHub Pages 배포. Mac Chrome·Safari·WebKit 검사 |
| PWA | 브라우저에서 설치 | 설치·오프라인 실행 지원. 기기별 검사 결과는 검사 문서 참고 |
| Tauri Mac | 데스크톱 앱 | [v0.6 다운로드](https://github.com/gelb2/MachDefense/releases/tag/v0.6). 파일 저장·복원·종료 검사. macOS 14+ Apple Silicon |

## 고친 문제와 남은 일

이번에도 목표는 원작자의 의도와 기획, 기능과 성능을 되살리는 것이었다. 포팅판과 재개발판을 직접 플레이하며 차이를 찾았다. 기지 기관총이 발사될 때 화면이 끊기고, 생산 완료음이 빠지고, 백그라운드에서 돌아오면 효과음이 사라졌다. 원작에서 보던 가격과 안내, 선택 효과도 빠져 있었다. 그런 차이를 지적하며 수정을 시켰다.

Codex는 원작에서 유지하던 오디오 컨텍스트를 발사 때마다 연결·해제했고, 오디오 세션 설정도 메인 스레드에서 호출했다. 내가 예상하지 못한 구현이었고, 기지 기관총 발사 때 화면이 끊기는 원인이 됐다. 나는 원작과 포팅판의 동작을 대조하고 Instruments로 측정하라고 지시했다.

[트러블슈팅](docs/public/troubleshooting.md) · [검사 결과](docs/public/validation.md)

새 락온 기능도 생각하고 있다. 사거리 안의 적을 표시하고, 사용자가 터치한 적과 가장 가까운 메크 한 대가 크로스파이어를 발사하는 방식이다. 적 포탑과 탱크도 대상이며 메크마다 3초 간격을 둔다. 피해와 발사 수·속도·효과는 현재 크로스파이어 레벨을 따른다. HeavyMach1에 넣은 자동조준을 보고 지인이 “아 이거라도 있으니 게임 쉽게 할 수 있다. 어깨 빠지는 줄 알았네”라고 했던 기억이 있다.

MachDefense의 락온은 원작의 적 배치와 공격력, 적과 아군의 행동을 바꾸지 않으면서 화면을 보고 판단할 기회를 더하려는 구상이다. 기존 크로스파이어의 에셋과 조준·사거리 로직을 쓰되, 크로스파이어 자체는 그대로 둘 생각이다.

10레벨 강화와 INSTALL한 전체 유닛의 생산 목록 스크롤은 구현했다. 원형 구조물과 포탑 밀집 구간의 길찾기, 생산 목록 순서 편집, 새 락온 기능, PWA·Tauri 운영 업데이트 확인은 남은 과제다. [남은 과제](docs/public/roadmap.md) · [기술 구조](docs/public/architecture.md) · [개발 과정](docs/public/development-history.md) · [크레딧](CREDITS.md)

[현재 구현 상태](docs/public/implementation-status.md) · [문서 목차](docs/public/README.md) · [변경 이력](CHANGELOG.md) · [크레딧](CREDITS.md)

## 나는 이렇게 일을 시키고 테스트했다

계획과 보고서로 수정 범위를 확인하고, 포팅판과 치트앱으로 게임을 검증한 과정을 기록했다. 내가 플레이하며 발견한 오류와 Codex가 수정·재검사한 결과를 함께 담았다.

[나는 이렇게 일을 시켰다](docs/public/work-method.md) · [나는 이렇게 테스트했다](docs/public/testing-method.md)

## 라이선스와 자원

이 프로젝트에서 새로 작성한 코드에는 [MIT](LICENSE)를 적용한다. [코드 적용 범위](LICENSE-SCOPE.md), [그림·음원·게임 데이터](ASSET-LICENSES.md), [외부 라이브러리 고지](THIRD_PARTY_NOTICES.md)를 구분해 정리했다.

## 이 프로젝트의 존재 목적을 확실히 하기 위한 추가 문건

최근 맥디펜스 레포지토리에 방문한 방문객의 활동 내역을 간략히 조사하고, 동선을 역추적하여 다른 레포지토리의 방문 이력을 살펴봤다. 그 결과, 헤비멕과 맥디펜스, 나아가 내가 숨이 더 붙어 있다면 복원하게 될 헤비멕2 레포지토리의 존재 목적에 대해 확실히 해야 할 듯하여 별도의 문건을 남긴다.

활동 내역에 대한 캡처와 분석을 갖고 있다. 평범한 사람이나 법과 도덕이 통용되는 사회에서만 살아본 사람이라면 나의 생각에 대해 정황증거, 억측이라고 할 수 있다.  

그러나 나에게 방금 발생한 일은 CIA식으로 표현하자면 Actionable Intelligence라고 생각한다. 그러므로 이 문건을 남긴다.

나는 이걸로 돈을 벌 생각이 없다.

그런데 이 레포지토리를 보는 당신이, 만일 이걸 그대로 포팅해서 혼자 즐기는 것이 아니라 앱스토어나 스팀에 올리거나, 당신이 복원한 척 강의를 팔거나, 제3의 판매 채널에 넘긴다면 나는 이렇게 말하고 싶다.

나는 내 미래나 내 돈과 관련한 좋은 징조는 잘 맞추지 못한다. 하지만 나는 나쁜 징조에 대해서는 당신 생각보다 잘 맞추며, 그 나쁜 일에는 대한민국에서 일어난 두 차례의 계엄령과 유럽에서 일어난 우크라이나–러시아 전쟁도 포함된다.

당신이 이것을 정식 마켓이나 제3의 판매 채널에 넘긴다면, 그 뒤의 대가는 한 개인이 감당하기 힘들 것이다.

나는 수명이 얼마 남지 않았다. 당신의 수명이 얼마인지는 모르겠으나, 돈에 눈이 멀어 당신의 명을 재촉하지 마라. 내가 아니더라도, 그 누구든 당신의 명줄을 줄여버릴지도 모르니.

