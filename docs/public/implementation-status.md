# 현재 구현 상태

<!-- language-nav:start -->
[한국어](implementation-status.md) | [English](../en/public/implementation-status.md)
<!-- language-nav:end -->

2026-11-07 기준 게임 릴리스는 **v0.5**다. [릴리스](https://github.com/gelb2/MachDefense/releases/tag/v0.5) · [변경 이력](../../CHANGELOG.md)

| 영역 | 구현과 확인 범위 |
|---|---|
| iOS·iPadOS | 유니버설 앱. iPhone·iPad 정식/치트 v0.5 설치. iPad 강화·구매·복원·전투 검사 |
| Android | 같은 C++ 게임 모듈을 쓰는 네이티브 앱. Galaxy 정식/치트 v0.5 설치·앱 검사. 개발용 서명 APK 제공 |
| 웹·PWA | GitHub Pages 배포. Chrome·WebKit 검사, Galaxy 설치형 PWA 오프라인·복원 검사, 사용자 iPhone↔iPad PWA 복원 확인 |
| Tauri Mac | 미구현. 앱 창·파일 저장·종료·업데이트는 후속 개발 |
| 원작 기능 | 80미션, 생산·전투·강화·특수공격·승패·보상·5슬롯 저장 |
| 추가 기능 | 30/45/60초 반복 긴급지원, 10레벨 강화, 설치한 전체 유닛 목록 스크롤, 레이저 포탑 해금·소급 지급 |
| 진행도 이전 | 공용 MD1 코드로 전체 5슬롯 복원. 추가 레벨을 받는 앱은 v0.4 이상 필요 |

웹의 자원 캐시와 게임 저장은 분리했다. v0.5 운영 검사에서는 구버전 캐시로 첫 재진입할 때 이전 테이블이 실행된 사례가 있었다. 다시 열면 최신값을 읽었고 저장은 유지됐다. 첫 업데이트 전환은 아직 수정해야 한다.

검사 수치는 [검사 결과](validation.md)와 해당 기록에서 확인한다. 실제 출력음 대조, 장시간 성능, iPhone 설치형 PWA 직접 검사는 남아 있다. [남은 과제](roadmap.md)

[게임 소개](../../README.md) · [문서 목차](README.md)
