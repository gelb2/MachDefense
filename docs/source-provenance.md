# 원본 보관과 판본

출처는 Internet Archive의 heavy-mach-archive_202406 모음이다. 다운로드 파일·크기·SHA·패키지 정보는 [원본 명세](originals-manifest.json)에 있다. 명세에는 HeavyMach 1·2·HD·Defense·Assault 전체 19개를 포함해 실수로 반입되는 다른 시리즈 IPA도 검사한다.

MachDefense 후보는 다음 세 파일이다. 최초 호환 실행 기준은 1.0.5(kr.indieapps.MachDefense)로 정했다. 세 판본의 파일·데이터 표·메서드 목록을 비교했다. 재개발의 미션·에셋 기준은 사용자 지시에 따라 위메이드 1.0.3으로 정한다. [비교 근거](evidence/version-audit-20261104.md).

- Mach Defense 1.0.3 — kr.indieapps.MachDefense.
- Mach Defense 1.0.5 — kr.indieapps.MachDefense.
- Heavy MACH: Defense 1.0.3 — com.wemadeent.HeavyMachDefence.

모두 패키지에서 iPhone/iPad 지원으로 표시됐다. 실행 바이너리의 cryptid는 세 판본 모두 0이다. 1.0.5에는 ARMv6·ARMv7이 있으며 현재 호환 실행기는 ARMv7을 실행한다. 기존 두 판본은 미션 70개, 위메이드판은 80개다. 공통 에셋·음원·표와 변경 수치는 비교 근거에 정리했다. 전체 게임 함수의 동등성 검사는 남아 있다. 원본은 `/Users/sokol/Documents/Codex/Originals/HeavyMach/heavy-mach-archive_202406`에 있다. 다른 Mac에서는 별도 원본 경로를 로컬 설정에 지정한다.

원본 IPA·추출 실행 바이너리는 이 저장소로 복사하지 않는다. 업로드·첨부·원격 분석·LFS·릴리스 자산에 넣지 않는다. 분석 도구가 로컬 서비스인지 먼저 확인한다. 변환한 에셋은 별도 Resources에 준비하며 원본 파일·변환 방식·SHA를 남긴다. 최종 소스 재구현 앱은 IPA를 빌드 입력으로 사용하지 않는다. 현재 요청한 호환 비교 앱은 로컬 설정에 명시한 외부 IPA의 SHA-256을 확인해 외부 빌드 폴더의 앱에 넣는다. 이 앱과 패키지도 외부로 업로드하지 않는다.

기존 Mesabloo 포크 `/Users/sokol/Git/hm-defense`와 HeavyMach 1 소스는 그대로 보존한다. 이 저장소는 기존 포크를 복사하거나 이력을 바꾸지 않았다.
