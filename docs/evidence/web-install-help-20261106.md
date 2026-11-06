# 웹 설치 안내 — 2026-11-06

Mac에서 INSTALL GAME을 눌러도 게임이 시작되지 않고 PLAY IN BROWSER로 실행된다는 사용자 관측을 조사했다. 두 버튼의 목적은 설치와 브라우저 실행으로 나뉜다. 기존 코드는 설치 프롬프트가 없을 때에도 INSTALL GAME을 표시하고 일반적인 안내만 바꿨다. 당시 사용자 브라우저의 프롬프트가 없었던 이유까지는 확인하지 않았다.

## 수정

`Distribution/Web/app.js`와 `index.html`만 제품 동작을 수정했다. 프롬프트가 있으면 INSTALL GAME, 없으면 HOW TO INSTALL을 표시한다. Mac Safari의 File → Add to Dock, Chrome 메뉴의 Install page as app, iPhone/iPad의 Share → Add to Home Screen을 구분한다. 설치 취소·완료·설치형 창 상태를 처리하며 PLAY IN BROWSER는 독립적으로 실행한다.

공용 C++·Apple·Android·게임 자원·저장 형식은 변경하지 않았다. 트랙패드 휠과 키보드는 [별도 연구](../research/desktop-input-20261106.md)에 남겼다.

## 검사

`node Web/tests/install-ui.mjs http://127.0.0.1:8951/`로 Chrome과 WebKit에서 설치 안내, 안내 포커스, 설치형 창, 설치 취소/승인 이벤트, appinstalled, 게임 타이틀 진입, 저장 쓰기 0회, 페이지 오류 0개를 확인했다. iPad 식별과 설치 승인/취소는 브라우저 이벤트 모의 검사이며 실제 기기 설치 검사를 뜻하지 않는다. 사용자 브라우저 저장은 조작하지 않았다. 로그는 로컬 `.local/web-install-scroll/install-ui.json`에 있다.

## 브라우저 문서

- [Chrome 웹 앱 설치](https://support.google.com/chrome/answer/9658361?hl=en)
- [Safari 웹 앱](https://support.apple.com/en-ca/104996)
- [beforeinstallprompt](https://developer.mozilla.org/en-US/docs/Web/API/Window/beforeinstallprompt_event)
