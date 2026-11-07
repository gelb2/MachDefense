# 기술 구조

iOS·iPadOS·Android 네이티브 앱과 웹·PWA가 같은 C++ 게임 코드를 사용한다.

## 공통 게임 코드와 플랫폼 실행기

| 영역 | 소스 위치 | 하는 일 |
|---|---|---|
| Core | `Sources/Core/` | 피해·이동·사거리·생산·경제·강화·미션·저장·게임 메뉴와 HUD |
| Audio | `Sources/Audio/` | 효과음 버퍼·재생 소스·컨텍스트 재사용 |
| SaveTransfer | `Modules/SaveTransfer/` | 복원 코드 생성·해석·검증 |
| 자원 | `Resources/` | 이미지·음원·맵·표 |
| Apple | `Apple/`, `Sources/Platform/Apple/` | 워크스페이스·렌더링·입력·오디오 세션·저장 경로 |
| Android | `Android/` | Kotlin·JNI·EGL·오디오 출력·앱 생명주기 |
| 웹 | `Web/`, `Distribution/Web/` | Wasm·WebGL2·브라우저 입력·음향·IndexedDB·시작 화면·PWA |

Core는 플랫폼 SDK를 직접 부르지 않는다. `ResourceAccess`와 `HostServices`를 통해 자원 읽기·저장·음향·텍스트 입력을 요청한다. 게임 화면은 Core가 만든 `Frame`을 각 실행기가 그린다. 메뉴와 HUD 버튼도 이 경로를 쓴다.

## 렌더링과 음향

Apple은 OpenGL ES 1 fixed-function, Android는 EGL/OpenGL ES 1.1, 웹은 WebGL2로 화면을 출력한다. 텍스처를 업로드해 재사용하며 발사마다 이미지를 새로 만들지 않는다.

Apple의 효과음은 OpenAL Soft, 음악은 AudioFile·AudioQueue로 재생한다. Android 음악은 MediaPlayer를 사용한다. 웹 효과음은 기존 코드의 Emscripten OpenAL 연결, 음악은 Web Audio 출력으로 처리한다. 어떤 소리를 언제 재생할지는 게임 코드가 결정한다.

## 저장과 복원

기준 판본의 저장은 5슬롯·42,496바이트다. 복원 코드는 버전·길이·CRC32를 검사하고 PackBits와 Base64url로 데이터를 옮긴다. 손상되거나 지원하지 않는 코드는 적용하지 않는다.

웹은 게임 저장을 IndexedDB에 기록하고 트랜잭션 완료 뒤 성공을 반환한다. 자원은 Cache Storage에 둔다. 자원 캐시를 다시 받아도 진행 데이터는 유지된다. 두 창에서 같은 진행을 열었을 때 오래된 창이 최신 저장을 덮지 않도록 저장 버전을 대조한다. 사용자가 사이트 데이터를 삭제하면 복원 코드로 진행을 다시 가져올 수 있다.

복원 적용 전에는 기존 저장의 백업을 남긴다. 현재 메뉴는 전체 5슬롯을 교체한다. Apple·Android는 공용 FileStore로 백업·중단 복구·파일 교체를 처리한다. 복사·검사·취소와 확인 버튼은 공용 C++ 메뉴가 그린다. 플랫폼은 텍스트 입력·키보드·클립보드와 저장 경로를 연결한다. 단일 슬롯 선택은 검토 제안이며, 현재 복원 범위는 전체 5슬롯이다.

[게임 소개](../../README.md) · [문서 목차](README.md)
