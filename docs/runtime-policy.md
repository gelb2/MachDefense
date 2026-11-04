# 시간·저장·음향

게임 tick과 표시 주기, pause/resume, 오디오 중단·복귀, 저장 시점·포맷·슬롯을 원본에서 확인한다. 발사나 피격마다 DB/파일을 기록하는 새 경로를 만들지 않는다. 일반 저장을 치트 검증으로 덮지 않는다. 백그라운드/포그라운드, 화면 잠금, GPU 소실, 오디오 focus, 프로세스 종료를 구분해 검사한다.

호환 실행 중 QTable의 표 해독에 srandom/random의 Darwin 난수열이 필요함을 확인했다. 호환 실행기의 대체 난수식을 Darwin TYPE_3으로 교체했으며 5,000회 결과를 macOS 함수와 대조했다. 소스판은 [DarwinRandom](../Sources/Core/DarwinRandom.hpp)으로 이 난수열을 사용한다. [GameSession](../Sources/Core/GameSession.cpp)은 프로필·명시적 저장·결과 화면의 저장 지점을 연결한다. 전투 중 값은 메모리에서 바꾸며 FrameIO 검사가 전투·그리기의 반복 파일 접근을 검사한다. 비활성 전환은 전투를 멈추고 복귀 뒤에도 메뉴에서 재개한다. [MDAudio](../Sources/Platform/Apple/MDAudio.mm)는 활성 중 OpenAL 컨텍스트를 유지하고 비활성 때 중단한다. 원인과 측정은 [트러블슈팅](troubleshooting.md), 남은 중단 복귀·전체 무기 검사는 [TODO](usability-todo.md)에 있다.

오디오 중단은 게임 입력과 화면 갱신을 차단하지 않는다. 복귀 때 AVAudioSession을 재활성화하고 게임은 멈춤 메뉴에서 재개한다. 생산 완료 효과음은 World의 생산 완료 처리에서 한 번 호출한다. [중단 복귀와 완료음 검사](evidence/source-ui-audio-lifecycle-20261104.md)를 따른다.
