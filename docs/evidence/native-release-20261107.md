# v0.6 실기기 빌드 검사

2026-11-07. iPhone·iPad의 정식·치트 앱 네 개를 0.6(6)으로 빌드·설치했다. 설치 전 앱별 저장 42,496바이트를 백업했고, 설치 후 네 개 모두 바이트가 같았다. 사용자 저장은 `.local/release-v06/apple-backup`에 보존한다.

iPhone과 iPad 실기기에서 `testLevel10AndScrollableProductionHUD`를 실행했다. 각각 강화 130행·구매 117건·거절 143건·저장/복원 130건·표시 수치 240건과 실제 렌더·생산 HUD를 통과했다. 이 검사는 짧게 압축한 기능 검사이며 실시간 프레임 성능 측정값으로 쓰지 않는다.

CLI 서명은 `errSecInternalComponent`로 실패했다. 기존 Xcode GUI 서명을 사용해 검사했고, 이후 Clean Build Folder로 검사 플러그인이 없는 정식·치트 앱을 다시 만들었다. 개인 개발 팀 설정은 커밋에서 제외했다. 아이폰 정식 앱에서는 기존 프로필과 미션 선택 화면을 직접 확인했다.

Galaxy 정식·치트 앱도 0.6으로 설치했다. 설치 전후 저장이 같았고 치트 검사의 복원 9항목·생산/강화/음향/생명주기/승패 13항목이 통과했다. 검사용 저장은 검사 후 원래 값으로 돌렸다.

근거: `.local/release-v06/apple-save-preserved.json`, `iphone-level10.xcresult`, `ipad-level10.xcresult`, `galaxy-transfer.log`, `galaxy-smoke.log`. 저장 원본·개인 복원 코드는 Git에 올리지 않는다.
