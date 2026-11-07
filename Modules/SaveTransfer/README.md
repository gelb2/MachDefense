# MachDefense 복원 코드

<!-- language-nav:start -->
[한국어](README.md) | [English](README.en.md)
<!-- language-nav:end -->

`Codec.hpp`는 원본 저장 슬롯 5개와 해석하지 않는 바이트를 모두 보존한다. `MD1-`에는 버전 1, 42,496바이트 원문의 CRC32, 크기를 제한한 PackBits와 Base64url이 들어간다. 원본 저장 시드와 체크섬도 데이터에 남는다. 저장을 해석하고 다시 인코딩해도 바이트를 보존한다.

서버·salt·암호화·플랫폼별 코덱은 없다. 코덱은 기존 `SaveData` 접근 함수를 사용한다. 공용 메뉴가 GET CODE / ENTER CODE와 확인 창을 담당한다. 실행기는 텍스트 입력과 영구 저장을 연결한다. 복원 기능을 켜지 않은 실행기는 기존 메뉴를 유지한다.

Apple·Android·웹은 같은 코덱과 C++ 메뉴를 사용한다. GET CODE는 읽기 전용 입력란을 열고 COPY는 내용을 복사한다. ENTER CODE는 편집 가능한 입력란을 연다. CHECK로 검사한 다음 원작 YES/NO 버튼으로 전체 슬롯 5개 교체를 확인한다. CLOSE와 CANCEL은 저장을 바꾸지 않는다. 실행기는 입력란·키보드·클립보드를 제공한다.

`FileStore.hpp`는 Apple/Android의 복원 트랜잭션을 구현한다. 백업, 적용 대기 데이터, 원자적 교체, 재읽기, 영구 완료 표시와 정리를 처리한다. 시작할 때 중단된 트랜잭션을 완료한다. 완료 표시는 오래된 작업 기록이 이후 게임 저장을 덮지 못하게 한다. I/O 결과가 불확실하면 재시작 전까지 추가 저장을 막는다. 웹은 기존 IndexedDB 트랜잭션으로 백업과 교체를 처리한다.

단일 슬롯 선택과 향후 설정 확장은 제안 단계다. 현재 기능은 슬롯 5개 전체를 옮긴다. [구현과 검사](../../docs/evidence/save-transfer-all-platforms-20261106.md).
