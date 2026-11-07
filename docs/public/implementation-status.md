# 현재 구현 상태

<!-- language-nav:start -->
[한국어](implementation-status.md) | [English](../en/public/implementation-status.md)
<!-- language-nav:end -->

2026-11-07 기준 게임 릴리스는 **v0.6**다. [릴리스](https://github.com/gelb2/MachDefense/releases/tag/v0.6) · [변경 이력](../../CHANGELOG.md)

| 영역 | 구현과 확인 범위 |
|---|---|
| iOS·iPadOS | 유니버설 앱. iPhone·iPad 정식/치트 v0.6 설치. 두 기기에서 강화·구매·복원·생산 HUD 검사 |
| Android | 같은 C++ 게임 모듈을 쓰는 네이티브 앱. Galaxy 정식/치트 v0.6 설치·앱 검사. 개발용 서명 APK 제공 |
| 웹·PWA | GitHub Pages 배포. Chrome·WebKit 검사, Galaxy 설치형 PWA 오프라인·복원 검사, 사용자 iPhone↔iPad PWA 복원 확인 |
| Tauri Mac | macOS 14+ Apple Silicon 앱. 파일 저장·복원·Cmd-Q·메뉴 종료·서명된 업데이트 검사 |
| 원작 기능 | 80미션, 생산·전투·강화·특수공격·승패·보상·5슬롯 저장 |
| 추가 기능 | 30/45/60초 반복 긴급지원, 10레벨 강화, 설치한 전체 유닛 목록 스크롤, 레이저 포탑 해금·소급 지급 |
| 진행도 이전 | 공용 MD1 코드로 전체 5슬롯 복원. 추가 레벨을 받는 앱은 v0.4 이상 필요 |

웹 자원 캐시와 게임 저장을 분리했다. v0.6은 자원의 해시·길이를 검사하고 변경된 조각을 받는다. 현재와 이전 빌드 자원을 보존하며 게임 진행은 정리 대상에서 제외한다. Chrome·WebKit에서 운영 v0.5 파일로 만든 구버전 설치의 갱신과 저장 보존을 검사했다. [PWA 검사](../evidence/pwa-update-20261107.md).

검사 수치는 [검사 결과](validation.md)와 해당 기록에서 확인한다. 실제 출력음 대조, 장시간 성능, iPhone 설치형 PWA 직접 검사는 남아 있다. [남은 과제](roadmap.md)

[게임 소개](../../README.md) · [문서 목차](README.md)
