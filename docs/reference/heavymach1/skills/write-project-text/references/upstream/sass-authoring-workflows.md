# Sass 원고와 파생 콘텐츠

일반 구현·디버깅은 설치된 Agent Skills 프레임워크를 사용하고, 이 스킬은 Sass 원문의 편집 경계를 담당한다. [CONTENT_AUTHORING_CANON.md](./CONTENT_AUTHORING_CANON.md)와 [content-authoring-contract.json](../contracts/content/content-authoring-contract.json)이 원문·목소리·메타카피·모델의 정본이다.

1. 요청이 원문 보고, 편집 후보, 번역, 파생 동기화, 게시 중 무엇인지 식별한다. 원본·후보·승인본과 바꿀 표면을 구분하며 읽기·보고를 윤문 허가로 확대하지 않는다.
2. Magazine/Interview 새 후보는 정본 §3.2에 따라 원본·기존 비교 호·렌더링 후보를 대조한다. 질문·발문·이미지 역할을 실제 모델에 연결한다.
3. 번역·챗봇·OG는 승인 원문에서 파생하고 대상별 누락·오역·불일치를 남긴다. 게시에는 locale·OG 실물 확인이 포함된다.
4. 검수 전달은 `sass-review-publication`와 정본의 원문·이미지 접근 조건을 따른다. 링크 생성과 사용자 접근 성공은 구분한다.
5. 제안·지원·콜드메일·경력 문장은 [PROPOSAL_AUTHORING_CANON.md](./PROPOSAL_AUTHORING_CANON.md)의 STAR·CAR 전략과 `sass-applications`로 연결한다. 단순 수집·보관·번역을 재작성으로 확대하지 않는다. 경력 주장에는 이력서 정본, DB 저장에는 `sass-admin`을 함께 적용한다.

원본 대비 변경, 미확인 사실, 남은 파생 표면을 보고한다. 검사 실행·재사용은 [단일 검증 담당 절차](VERIFICATION.md), 권한·게시·통합은 [프로젝트 운영 계약](./PROJECT_CONTRACT.md)을 따른다.


# Sass 경력 주장과 이력서 파생본

[RESUME_CAREER_CANON.md](./RESUME_CAREER_CANON.md)의 검토·편집 경계와 [resume-career-contract.json](../contracts/content/resume-career-contract.json)의 노출 표면을 적용한다. 일반 코드·PDF 도구 운용은 해당 프레임워크/형식 스킬에 맡기고 이 스킬은 경력 사실과 파생 일치를 담당한다.

- 주장마다 현재 문면, 출처·시점, 개인 기여, 승인 범위와 노출 경로를 연결한다. 검토 요청의 결과는 대조와 제안이며 수정 요청의 범위만 편집한다. URL 응답을 운영 실적으로 격상하지 않는다.
- `public/resume.html` 수정 후 `node scripts/resume/contact-variant.mjs --write`로 연락처 제외 HTML을 동기화한다. 한쪽만 게시하지 않는다. PDF 영향이 있으면 정본의 `npm run resume:pdf:write`로 생성하며 두 HTML 및 `resume:pdf:check` 등 필요한 검사는 단일 reviewer에게 넘긴다.
- Career 번역·Apps·timeline·챗봇의 같은 주장을 요청 범위에서 대조한다. Control 개인 원본을 공개 소스·보고서에 복제하지 않는다.
- 경험 문장 작성은 [PROPOSAL_AUTHORING_CANON.md](./PROPOSAL_AUTHORING_CANON.md)의 STAR·CAR 전략과 `sass-applications`를 따른다. 승인 문면과 데스크톱·모바일 렌더링, 게시 요청이면 실제 공개 문면까지 연결한다.

주장 오류의 편집·복구는 정본의 승인 경계를 따른다. [프로젝트 운영 계약](./PROJECT_CONTRACT.md)과 [검증 담당 절차](VERIFICATION.md)는 이 스킬로 우회하지 않는다.
