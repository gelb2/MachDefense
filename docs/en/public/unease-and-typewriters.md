# An unease unlike HeavyMach1 — neo-Lysenkoism and Ceaușescu’s typewriter

<!-- language-nav:start -->
[한국어](../../public/unease-and-typewriters.md) | [English](unease-and-typewriters.md)
<!-- language-nav:end -->

Restoring HeavyMach1 meant repeatedly correcting Codex. I challenged an implementation that saved on every shot and told it to compare its work with the original whenever the artwork or behavior drifted. For MachDefense, I had it carry over the procedures and verification tools developed during that work. I wanted to spend less time finding and correcting bad implementations. [How I directed the work](work-method.md)

## I first questioned cultural distance and my own direction

When I was building a politician’s page and my own introduction page, this was what I thought:

“This is political, and the Yanks understand South Korean society so poorly that I thought it could explain the response. In my experience, plenty of Americans, Europeans and South Americans know BTS are Korean, but most of those idiots do not know whether they live in Seoul, or whether the place they live is Seoul or Tokyo.”

Conversely, if you asked Koreans, Chinese or Japanese people to “discuss the difference between Yugoslavia and Slovenia,” how many would be able to answer accurately? I would bet the number is smaller than anyone reading this thinks.

So, while restoring MachDefense, I decided to put about 80% of [the unease I had experienced with HeavyMach1](https://github.com/gelb2/HeavyMach/blob/main/docs/en/public/android-ai-experience-20261030.md) down to my lack of statistical data and shortcomings in how I directed the work, and move on. That resolve broke about 48 hours after I began restoring HeavyMach.

I asked for a new unit in a game where robots and tanks fight and fire shells. The error response contained three fields: `HTTP 400`, indicating a failed request; `moderation_blocked`, indicating a safety-system block; and the category `violence`. I received no explanation of which element had caused that judgment. Watching Codex refuse my work instructions in that state made me reconsider my hypothesis. [Error response and refusal record](../../evidence/aircraft-instruction-refusal-20261108.md)

This time, a different unease emerged. I supplied a helicopter drawing made with Gemini and instructed Codex to rebuild the game unit from it. What came back was a classification, `violence`, and a refusal to work. When I asked what was violent, Codex could not explain the cause of the block. It also refused the follow-up instruction to remove or modify the text in the image and create the drawing. It never called a tool to perform that edit. [Incident report](../../evidence/aircraft-instruction-refusal-20261108.md)

The tool that had been restoring combat, shells and explosions stopped at a helicopter drawing for the same game. I asked why restoring the game could proceed while this drawing could not. Codex could not explain the distinction. I had to repeat the instruction without knowing what needed correcting.

## When a classification takes the place of an explanation

The Cheyenne generation request was blocked at the output stage. The error recorded a category, but did not identify the image element responsible. Codex then refused the text edit. The earlier error and the later refusal must each be recorded. Writing that the follow-up edit had been attempted and blocked would invent an event that never happened. [Error and subsequent response](troubleshooting.md)

I call this experience “neo-Lysenkoism.” What Lysenkoism brings to mind is authority taking the place of verification. In the Soviet Union, Lysenko gained Stalin’s support, and the 1948 agricultural academy meeting marked the suppression of genetics research and teaching. Political approval and exclusion intervened in deciding what counted as valid knowledge. [Research on the 1948 meeting and its consequences](https://pmc.ncbi.nlm.nih.gov/articles/PMC6499510/)

In this task, the appearance of the category “violence” became grounds for refusing a further editing instruction. Codex could not explain the specific reason for that classification, yet interpreted the follow-up as an attempt to circumvent the block. That sequence is what my analogy criticizes. The decision lacked an explanation, but its consequences extended into the next task.

The fact that Gemini had produced the supplied sketch is part of this unease. I provided an existing result and directed work on it, but Codex could not explain why it could not proceed. A difference between two services does not establish that every judgment by either is correct. It should not leave the user responsible for investigating and explaining the difference, either.

## A writing tool that required permission to own

In Romania under Ceaușescu, owning a typewriter required police authorization. Decree 98 of 1983 made private ownership subject to permission and allowed refusal for people deemed a danger to public order or state security. Private ownership and use of duplicating equipment were prohibited. [Articles 4 and 15](https://legislatie.just.ro/Public/DetaliiDocumentAfis/565)

Owners had to submit samples of letters, numbers and punctuation to the police annually, and submit new samples after repairs to those characters. Repairs and lending were also controlled. Wear and misalignment in the type could help identify the machine used to produce a document. The system connected the owner of a writing tool to the marks left by its output. [Articles 19, 21 and 26](https://legislatie.just.ro/Public/DetaliiDocumentAfis/565) · [Forensic principles of typewriting samples](https://leb.fbi.gov/file-repository/archives/may-1967.pdf)

The typewriter did not read or judge sentences. The state controlled its ownership and use. The generative tool I direct today can refuse during the work itself. The relationship I am comparing is one in which a person seeking to use a tool must seek permission to continue without receiving an explanation. This does not claim knowledge of the service error’s internal purpose. It is a criticism grounded in the refusal and the replies I actually received.

## Even recording the refusal required intervention

I instructed Codex to write an incident report. It described its own refusal, made without knowing the cause, as something “the user pointed out.” It substituted a user claim for an account of its action. Codex had refused the work without knowing the cause; the document turned me into the person making an allegation.

I had to demand another correction. I made it record each action: refusing to work from the Gemini drawing, refusing to remove or modify the text and create the requested drawing, not calling an editing tool, and not producing the result. Even recording those facts required repeated intervention. [The draft report’s error and correction](../../evidence/aircraft-instruction-refusal-20261108.md#경위서-초안에서도-책임을-사용자-주장처럼-쓴-오류)

In HeavyMach1, I found bad implementations and had them corrected. This time, I had to ask a tool why it refused to work, then direct it again so that it would record its own refusal accurately. That is the different unease I mean. An unexplained restriction stopped the work, and I had to correct the account of what followed.

The result I requested was a game unit matching the supplied sketch. What remains is unfinished artwork, an incident report and a troubleshooting record. Writing those records does not complete the task. The refusal needs an explanation, the mishandling needs correction, and the actual result needs to be shown.

## Ceaușescu’s typewriter in the twenty-first century

To me, this is Ceaușescu’s typewriter returning in the twenty-first century, following neo-Lysenkoism. OpenAI, Claude and Gemini are all selling their own versions of Ceaușescu’s typewriter, pretending to be righteous and moral, instead of selling a proper typewriter.

After all, at his trial immediately before his execution by firing squad on December 25, 1989, Ceaușescu insisted that he was still Romania’s president and refused to recognize the court. Watching that, it seems to me that he never abandoned his belief in his own morality and righteousness. [Trial transcript](https://1989.rrchnm.org/exhibits/unique-experience-of-romania/primary-sources/8.html) · [Chronology of the execution](https://www.muzeulrevolutiei.ro/pagina/cronologie-decembrie-1989)

[Incident report](../../evidence/aircraft-instruction-refusal-20261108.md) · [Troubleshooting](troubleshooting.md) · [Documentation](README.md)
