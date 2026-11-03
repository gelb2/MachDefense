# 현재 상태 — 2026-11-04

`feat/ios-compatibility`에서 Mach Defense 1.0.5의 iOS 호환 실행판을 개발 중이다. 원본·게임 에셋·바이너리는 변경하지 않았다. 빌드 산출물은 저장소 밖에 둔다.

확인한 결과:

- iOS 시뮬레이터 빌드 성공. 원본 타이틀 터치→문 열림/닫힘→Select Slot→새 슬롯 생성→스테이지 선택→도움말→첫 전투 진입을 확인했다. 유닛 생산 버튼을 눌러 에너지 차감과 아군 출격·이동을 확인했다.
- ARM64 실기기용 앱을 iPhone Air에 무선(localNetwork) 설치했다. 개발자 신뢰 처리 후 직접 실행해 타이틀 터치와 슬롯 화면 진입을 확인하고 종료했다. 슬롯 생성 오류 수정본도 무선 설치했다. 해당 수정본의 도구 실행 요청은 기기 잠금으로 실패했고, 이후 사용자가 직접 플레이하기로 했다. 기기 전투 검사는 미완료다.
- 원본에서 호출하는 iAd 배너 클래스를 광고 요청·표시 없이 처리했다.
- 원본 QTable이 srandom/random으로 해독하는 표가 실행기의 대체 난수식 때문에 깨지는 원인을 확인했다. Darwin 방식으로 수정한 뒤 맵 로딩 오류가 사라졌다. 5개 시작값·5,000개 결과를 macOS와 대조해 일치했다.
- UIScreen의 displayLinkWithTarget:selector:를 기존 CADisplayLink에 연결했다. 원본의 프레임 갱신과 화면 전환이 진행된다.

슬롯 생성 중 원본이 누락된 DummyParts.png를 참조할 때 호환 실행기가 빈 CGImage에서 종료됐다. macOS Core Graphics의 NULL 인수 동작을 직접 측정하고 너비·높이 0 반환, 빈 컨텍스트 변환·그리기 처리를 맞춘 뒤 슬롯 생성에 성공했다. 원본 파일명이나 게임 코드는 변경하지 않았다.

이름 입력칸이 포커스 후 아래로 이동하는 표시 문제가 남았다. 사용자도 같은 현상을 확인했다. 구버전 iOS API나 키보드 알림 처리 문제라는 사용자 추정을 조사 항목에 추가했다. 원인은 미확정이다. 입력 포커스 때 바뀌는 합성 경로와 좌표 변환, 키보드 알림에 따른 프레임 변경을 함께 대조할 예정이다. 전투·저장·재실행과 iPad 설치·조작 검사가 남아 있다. 전체 미션 정상 동작과 성능은 아직 확인하지 않았다.

광고는 HeavyMach 1·2·MachDefense의 모든 판본·플랫폼에서 표시하지 않는다. 현재 호환판도 같은 방침이다.

로컬 증거: `.local/device/iphone-install-2.json`, `iphone-launch-2.json`, `iphone-terminate-1.json`, `iphone-after-install-details.json`, `.local/simulator-build-13.log`, `.local/simulator-run-11.log`, `.local/coregraphics-null-probe.json`. 서명 누락 설치 실패는 `iphone-install-1.json`에 보존했다. Xcode 패키지 스크립트의 출력 파일을 선언한 뒤 후속 빌드의 서명 검사도 통과했다.

사용자의 정리·커밋 지시에 따라 추가 구현을 멈추고 현재 결과를 기록했다. 이번 재개 시 시뮬레이터는 종료된 상태였으며 새 전투 검사는 실행하지 않았다. 아이폰은 사용자가 직접 플레이하므로 앱 종료·재설치를 수행하지 않았다. 최신 설치·실행 요청 근거는 `.local/device/iphone-install-3.json`(성공), `iphone-launch-3.json`(Locked)이다.
