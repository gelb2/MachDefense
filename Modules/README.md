# Modules

`project.yml`에서 `MachDefenseCore` 정적 라이브러리와 `MachDefenseAssets.bundle`을 생성한다. Apple 앱은 이 두 출력을 연결한다. `Sources/Core`에는 Apple 타입이나 UIKit/OpenGL 호출을 넣지 않는다. 이후 웹 실행기도 같은 코어의 `Game.hpp` 계약과 `Resources`를 소비한다.

자원 번들은 `Images`, `Audio`, `RawData`, `Tables`, `Generated`를 포함한다. 원본 보관용 `OriginalImages`와 IPA·실행 바이너리는 앱에 포함하지 않는다.

공용 기능은 실제 사용과 검증 후 분리한다.
