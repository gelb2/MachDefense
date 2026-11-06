# 웹·PWA 첫 릴리스

## 웹·PWA와 복원 코드

기존 C++ 게임을 WebAssembly로 빌드해 브라우저에서 실행한다. WebGL2 화면 출력, 브라우저 입력·음향, IndexedDB 진행 저장을 추가했다. PWA는 자원을 내려받아 오프라인으로 실행하고 업데이트 뒤에도 진행을 유지한다.

슬롯 화면에 GET CODE와 ENTER CODE를 추가했다. 같은 공용 코덱으로 전체 프로필 5개를 내보내고 복원한다. 버튼은 원작 에셋으로 그린다. 코드 입력과 복사만 브라우저 기능으로 연결한다.

Apple·Android 실행기와 전투·이동·강화 계산은 유지했다. 기존 공통 파일4개에 복원 서비스와 메뉴 연결을 추가했다. 네이티브 수정 전후160조건 추적은 같았고 iOS 시뮬레이터·Android 일반/치트 빌드도 성공했다.

Mac Chrome·Safari·WebKit에서 실행했다. 아이폰의 진행된 저장 복사본을 웹으로 옮겨 재실행까지 확인했다. [웹·PWA 실행과 설치](https://gelb2.github.io/MachDefense/). 실제 모바일 설치형 PWA의 추가 검사는 남아 있다. Tauri Mac과 Apple·Android 복원 메뉴 연결은 후속 작업이다.

[게임 소개](../../README.md) · [문서 목차](README.md)
