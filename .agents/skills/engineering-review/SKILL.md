---
name: engineering-review
description: 요청된 코드 검토 또는 중요한 기능·보안·데이터 변경의 독립 검토에 사용한다. 모든 질문과 사소한 편집에 검토자를 추가하지 않는다.
---

# engineering-review

요청·수용 기준·실제 diff와 원본 검사 결과를 대조한다. 정확성, 단순성, 기존 구조, 보안, 성능 중 변경에 관계된 위험부터 확인한다. 스타일 취향을 필수 결함으로 만들지 않는다. 같은 입력의 유효한 원본 결과는 재사용하고 미확인 위험만 추가 검사한다. 발견은 파일·근거·영향·필수 여부로 보고한다. 없으면 검토 범위와 한계를 알린다. 테스트 통과를 요구 전체·운영 배포 완료로 확대하지 않는다.

구조 개선·검토 난점은 [원본](references/upstream.md)의 Five-Axis Review, Structural Remedies, Honesty in Review를 참고한다. Multi-Model Review는 선택 사항이며 모델 변경·사용자 재승인·추가검토를 의무화하지 않는다.

이 파일은 Addy agent-skills 0.6.12를 프로젝트에 맞게 수정한 적용본이다. 원문과 MIT 라이선스는 references에 보존했다. 공통 작업 방식은 .agents/policy.md를 따른다.
