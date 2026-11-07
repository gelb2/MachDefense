# MachDefense OG 시안과 공개 빌드 검사

2026-11-07 · 공개 main `0d3f84443910eec6db125803c6e1bb3f4d94015a` · 게임 v0.5

## OG 시안

HeavyMach1의 채택본처럼 메인 일러스트와 게임 로고로 화면을 채웠다. 검은 여백과 플랫폼 문구를 넣지 않았다. MachDefense의 원작 iPad 타이틀 이미지를 제공해 가로 구도로 재구성했다. 원본을 단순히 자른 이미지와는 다르다.

- A — GitHub 공유 카드: 위쪽 로고, 아래쪽 전차·메크. 1774×887, 2:1.
- B — 웹·PWA 공유 카드: 아래쪽 로고, 위쪽 교전 장면. 1734×907, 약 1.91:1.
- 사용자는 GitHub·PWA 모두 B안을 채택했다. 원래 1734×907 구도를 유지하고 JPEG로 내보냈다. 파일은 `Distribution/Web/social-preview.jpg`이며 627,410바이트다.
- GitHub Social preview 업로드를 완료했다. GitHub에서 내려받은 파일의 SHA-256도 `15fddebb459273543a2ebad916f7fae99ab8bf2cd90bd242050b6c482a93f633`으로 일치한다. 웹·PWA에는 같은 이미지의 절대 주소·제목·설명을 연결한다.

## 누구나 받을 수 있는가

공개 HTTPS 주소를 인증 없이 새 폴더에 clone했다. 기존 제작 폴더의 소스·에셋·앱·저장·IPA를 복사하지 않았다. 설치된 SDK와 컴파일러를 사용했다. 소스 파일은 변경하지 않았다.

| 경로 | 이번 확인 | 이용 조건 |
|---|---|---|
| 웹 | 공개 주소의 Chrome·WebKit에서 새 프로필 생성, Stage 1 진입, MachineGun 생산·이동 확인 | WebGL2를 지원하는 브라우저, 최초 자원 다운로드 |
| PWA | Mac Chrome에 실제 설치. 별도 창에서 타이틀·슬롯 진입, 오프라인 재실행 통과 | 첫 다운로드 후 설치. iPhone·iPad 홈 화면 신규 설치는 이번 검사에서 반복하지 않음 |
| Android | 새 클론에서 일반 APK 빌드. API 36 arm64 에뮬레이터에서 Stage 1 진입·생산·이동 확인 | Android 8 이상 arm64. Releases의 일반 APK도 다운로드 가능 |
| iPhone·iPad | 새 클론에서 시뮬레이터 Debug·기기 Release 빌드 성공. 새 시뮬레이터에 일반 앱 설치·실행 | Mac·Xcode·Python·CMake 필요. 실기기는 자신의 개발 팀으로 서명 |
| 웹 직접 빌드 | 새 클론에서 Emscripten 정식 빌드·패키징 성공 | Emscripten SDK. 생성된 build ID는 운영과 같은 `701b376ebb23a762` |

Chrome·WebKit 모두 다운로드 후 네트워크를 끊고 페이지를 다시 열어 게임이 시작되고 기존 프로필이 남는 것을 확인했다. 브라우저 실행 오류는 각각 0건이었다. Mac Chrome의 설치 가능성 검사도 오류 0건이었다.

iOS 치트앱 자동검사에서는 80개 미션의 실패·승리·다음 미션·디스크 저장 재읽기를 확인했다. 별도 구매 검사에서 기지 6종·유닛 4종·특수공격 3종을 게임 버튼으로 구매하고 저장을 확인했다. 두 XCTest가 통과했다. 이번 실행 장비는 시뮬레이터·에뮬레이터와 Mac 브라우저다. 사용자의 실제 휴대폰 앱과 저장은 사용하지 않았다.

## IPA 없이 동작하는가

iOS·Android 앱에 들어간 게임 자원 1,433개가 새 클론의 이미지·음원·맵·테이블·글꼴 파일과 SHA-256까지 일치했다. 공개 v0.5 일반 APK의 자원도 같은 1,433개와 일치했다. IPA 추출 도구를 실행하지 않았다. 빌드 로그에 원본 보관 폴더·Compatibility·기존 제작 checkout 경로가 없었다. 웹 빌드에서 사용한 Emscripten SDK 경로는 컴파일 도구의 위치다.

게임 코드와 준비된 자원은 일반 Git 파일로 들어 있다. LFS 파일은 0개다. 새로 받은 저장소의 파일만으로 iOS·Android·웹 빌드가 성공했다.

## GitHub에 IPA가 있는가

검사한 공개 범위에서 발견되지 않았다.

- 공개 브랜치·태그가 가리키는 161개 커밋, 고유 파일 내용 4,415개를 검사했다. 삭제된 과거 파일도 포함했다.
- `.ipa`·`Payload`·`Originals` 경로, 알려진 원본 IPA와 실행 바이너리의 SHA-256, ZIP 내부 경로와 중첩 압축, LFS 포인터를 검사했다.
- Release v0.1~v0.5의 첨부 12개를 모두 내려받아 실제 내용을 검사했다. 첨부는 Android APK 8개와 체크섬 파일 4개다. IPA는 없다.
- 현재 추적 파일과 LFS 목록에도 IPA가 없다. 원본 패키지 제외 규칙은 `.gitignore`에 있다.

이번 검사는 현재 GitHub가 공개 브랜치·태그·릴리스로 제공하는 범위다. 알 수 없는 삭제 커밋 주소나 GitHub 내부 보관 데이터까지 확인한 것은 아니다.

## 남아 있는 항목

기존 PWA의 구버전에서 처음 갱신할 때 이전 테이블을 한 번 실행하는 문제는 기존 TODO에 남아 있다. 새 설치·새 프로필 검사는 통과했다. Tauri Mac은 아직 구현 전이다. iOS 네이티브의 클릭 한 번 설치용 배포는 제공하지 않으며 Xcode 빌드 또는 PWA를 사용한다.

## 실행 주소

- 게임·PWA: https://gelb2.github.io/MachDefense/
- 저장소: https://github.com/gelb2/MachDefense
- Android APK: https://github.com/gelb2/MachDefense/releases/tag/v0.5
- 설치 안내: https://github.com/gelb2/MachDefense/blob/main/docs/public/build-and-install.md

검사 로그·화면·원본 해시는 로컬 `Documents/Codex/2026-11-07/machdefense-public-audit/`에 보존했다. [통합 보고서](https://heavymach-dev-report-20261025.hiphopxy.chatgpt.site/machdefense.html#og-public-audit-20261107).
