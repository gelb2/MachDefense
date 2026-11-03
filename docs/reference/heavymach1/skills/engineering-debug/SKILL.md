---
name: engineering-debug
description: 재현 가능한 오류·실패 검사·예상과 다른 동작의 원인을 좁혀 수정할 때 사용한다. 일반 설명이나 단순 문구 편집에는 적용하지 않는다.
---

# engineering-debug

실패 입력·예상·관측을 먼저 확보한다. 재현이 안 되면 그 한계를 남기고 증거를 더 읽는다. 현재 consumer와 관련 변경을 추적해 가설을 좁히고, 필요한 최소 재현으로 원인과 증상을 구분한다. 원인을 고친 뒤 원래 실패 경로와 인접 위험을 확인한다. 기존 사용자 변경을 되돌리거나 전체 suite를 자동 실행하지 않는다. 복구를 위한 새 승인·과거 RED 기록은 요구하지 않는다.

오류 유형별 진단이나 안전한 fallback이 필요할 때만 [원본](references/upstream.md)의 Triage Checklist, Error-Specific Patterns, Safe Fallback Patterns를 읽는다. 원본의 Verify End-to-End 예시는 해당 제품 위험에 맞게 선택한다.

이 파일은 Addy agent-skills 0.6.12를 프로젝트에 맞게 수정한 적용본이다. 원문과 MIT 라이선스는 references에 보존했다. 공통 작업 방식은 .agents/policy.md를 따른다.
