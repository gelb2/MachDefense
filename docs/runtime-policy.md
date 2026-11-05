# 시간·저장·음향

게임 tick과 표시 주기, pause/resume, 오디오 중단·복귀, 저장 시점·포맷·슬롯을 원본에서 확인한다. 발사나 피격마다 DB/파일을 기록하는 새 경로를 만들지 않는다. 일반 저장을 치트 검증으로 덮지 않는다. 백그라운드/포그라운드, 화면 잠금, GPU 소실, 오디오 focus, 프로세스 종료를 구분해 검사한다.

호환 실행 중 QTable의 표 해독에 srandom/random의 Darwin 난수열이 필요함을 확인했다. 호환 실행기의 대체 난수식을 Darwin TYPE_3으로 교체했으며 5,000회 결과를 macOS 함수와 대조했다. 소스판은 [DarwinRandom](../Sources/Core/Contracts/DarwinRandom.hpp)으로 이 난수열을 사용한다. [GameSession](../Sources/Core/Session/GameSession.cpp)은 프로필·명시적 저장·결과 화면의 저장 지점을 연결한다. 전투 중 값은 메모리에서 바꾸며 FrameIO 검사가 전투·그리기의 반복 파일 접근을 검사한다. 비활성 전환은 전투를 멈추고 복귀 뒤에도 메뉴에서 재개한다. [MDAudio](../Sources/Platform/Apple/MDAudio.mm)는 활성 중 OpenAL 컨텍스트를 유지하고 비활성 때 중단한다. 원인과 측정은 [트러블슈팅](troubleshooting.md), 남은 중단 복귀·전체 무기 검사는 [TODO](usability-todo.md)에 있다.

오디오 중단은 게임 입력과 화면 갱신을 차단하지 않는다. 복귀 때 AVAudioSession을 재활성화하고 게임은 멈춤 메뉴에서 재개한다. 생산 완료 효과음은 World의 생산 완료 처리에서 한 번 호출한다. [중단 복귀와 완료음 검사](evidence/source-ui-audio-lifecycle-20261104.md)를 따른다.

## 모듈화 후 음향 연결 — 2026-11-05

효과음 버퍼·source·컨텍스트는 `Sources/Audio/OpenALEffects`가 관리한다. Apple 실행기는 AudioFile로 WAV 원본 PCM을 한 번 읽는다. 발사·피격 때 디코딩·버퍼 생성·파일 저장을 하지 않는다. BGM은 원본과 같은 AudioFile/AudioQueue 압축 패킷 반복 재생으로 처리한다.

피격음은 원본의 카메라 상대 위치 계산을 거쳐 HostServices.playSoundAt으로 전달한다. 원본의 제곱근 계산이 음수 좌표에서 NaN을 만드는 경우 이전 source 위치를 유지한다. 피해·음원 선택·난수 호출 순서는 유지한다. [원본 주소와 검증](evidence/module-work-20261105.md).

AVAudioSession 설정·활성화·비활성화는 Apple 어댑터의 직렬 큐에서 비동기로 실행한다. 게임 입력·렌더링·OpenAL 객체는 메인 스레드에서 유지한다. 오래된 세션 완료가 최신 비활성 상태를 덮지 않게 요청 세대 번호를 대조한다. [실기기 검사](evidence/audio-session-20261105.md).

효과음의 백그라운드/복귀는 OpenAL Soft 장치의 pause/resume으로 처리한다. 컨텍스트 suspend/process는 출력 장치 재시작을 대신하지 않는다. OS 세션 활성화 완료 후 장치를 재개하고, 재생 호출 횟수와 실제 샘플 진행을 함께 검사한다. [복귀 후 효과음 소실 수정](evidence/effects-resume-20261105.md).

## Android 실행기

게임/렌더 작업은 하나의 HandlerThread에서 실행한다. Surface와 EGL context가 사라져도 GameSession과 진행 상태를 유지한다. Activity 재생성·백그라운드 복귀는 게임을 일시정지하며 사용자가 게임 메뉴에서 재개한다. 효과음은 공통 Audio/OpenAL Soft의 OpenSL ES 출력을, 음악은 MediaPlayer를 쓴다. 파일 저장은 GameSession 요청 때만 임시 파일·fsync·rename으로 교체한다. [검사 결과](evidence/android-native-20261106.md).
