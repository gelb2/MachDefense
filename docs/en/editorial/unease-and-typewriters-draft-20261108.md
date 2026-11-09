# An unease unlike HeavyMach1 — neo-Lysenkoism and Ceaușescu’s typewriter

<!-- language-nav:start -->
[한국어](../../editorial/unease-and-typewriters-draft-20261108.md) | [English](unease-and-typewriters-draft-20261108.md)
<!-- language-nav:end -->

Restoring HeavyMach1 meant repeatedly correcting Codex. I challenged an implementation that saved on every shot and told it to compare its work with the original whenever the artwork or behavior drifted. For the next game, MachDefense, I had it carry over the procedures and verification tools developed during that work. I wanted to spend less time finding and correcting bad implementations.

I did not attribute all of that unease to AI from the outset. When I was building a politician’s page and my own introduction page, this was what I thought.

“This is political, and the Yanks understand South Korean society so poorly that I thought it could explain the response. In my experience, plenty of Americans, Europeans and South Americans know BTS are Korean, but most of those idiots do not know whether they live in Seoul, or whether the place they live is Seoul or Tokyo.”

Conversely, if you asked Koreans, Chinese or Japanese people to “discuss the difference between Yugoslavia and Slovenia,” how many would be able to answer accurately? I would bet the number is smaller than anyone reading this thinks.

So, while restoring MachDefense, I decided to put about 80% of [the unease I had experienced with HeavyMach1](https://github.com/gelb2/HeavyMach/blob/main/docs/en/public/android-ai-experience-20261030.md) down to my lack of statistical data and shortcomings in how I directed the work, and move on. That resolve broke about 48 hours after I began restoring HeavyMach.

While restoring MachDefense, I was preserving the original creators’ work and researching and implementing a very simple way to push past the limitations of tower defense. I wanted to minimize changes to the code reconstructed from the original and to its assets, while using my own design to introduce less predictable combat and increase the value of the creators’ work.

The starting point was how attacks work in MachDefense. There were hitscan attacks that connect immediately, and unguided projectiles that travel at a finite speed without following their target after launch. What if I added a few families of allied units that could readily evade those unguided projectiles? I thought this could push past tower defense’s reliance on durability, firepower, placement and reorganizing the battle line.

What I took from Dark Souls and Sekiro was evasion and a moment of invulnerability. I conceived a family of units whose health would not decrease for a limited time when an evasive maneuver activated, and was introducing and testing helicopters to realize that idea. I wanted allied movement to introduce another variable into combat without substantially changing the original attack mechanics.

To state the result plainly, the algorithm was complete, however crude. Within the tests I conducted, even basic units achieved meaningful survival against advanced enemy units. It had reached a minimum functional stage at which balancing and adjustments were possible. It was now time to give those units the appearance I had envisioned.

But Codex kept spitting out skins that insulted the original creators’ aesthetic achievement. My requirements to preserve the supplied drawings’ shapes and proportions and the original’s texture were not met. Refusals to perform asset-rendering work compounded the problem. The evasion algorithm could run, but the appearance of the units I wanted to release remained unfinished. Whether these new units will be released is uncertain.

I supplied helicopter drawings made with Gemini and instructed Codex to rebuild the game units from those drawings. The Cheyenne image-generation request was blocked at the output stage, and the error response contained three fields: `HTTP 400`, `moderation_blocked` and `violence`. A failed request, a safety-system block, and a classification of violence. I received no explanation of which visual element had caused that judgment.

When I asked what was violent, Codex said it did not know the specific cause of the block. Yet it refused my work instructions. It also refused the follow-up instruction to remove or modify the text in the image and create the drawing. It never even called a tool to perform that edit. It had not attempted the edit and been blocked again. Codex refused to carry it out. [Actual error and refusal record](../../evidence/aircraft-instruction-refusal-20261108.md)

The tool that had been restoring combat, shells and explosions stopped at a helicopter drawing for the same game. I asked why restoring the game could proceed while this drawing could not. Codex could not explain the distinction. I had to repeat the instruction without knowing what needed correcting.

Is MachDefense a game made for the politically incorrect Alternative for Germany or a Russian neo-Nazi organization? I was restoring a game with no flying blood or flesh and no political agenda. At what point does adding graphics for a new science-fiction helicopter unit become a violence error?

Those three fields were what that bastard Codex handed me. It did not know the cause, but it refused the work. It refused the Gemini drawings, and it refused the instruction to remove or modify the text in the designs I supplied. The work stopped, and even the burden of explaining the tool provider’s judgment fell on the user.

I told it to write an incident report. Codex described its refusal to follow my instructions without knowing the reason as “the user pointed out that this was …” It put the user’s claim where its actual conduct should have been recorded. Codex was the one that refused the work without knowing the cause, but in the document I became someone making a claim.

I had to tell it to correct the report. I made it record each action: refusing to work from the Gemini drawings, refusing the instruction to remove or modify text to create the drawing, not calling an editing tool, and not producing the requested result. I had to intervene repeatedly even to get those facts recorded. [Errors in the draft incident report and their correction](../../evidence/aircraft-instruction-refusal-20261108.md#경위서-초안에서도-책임을-사용자-주장처럼-쓴-오류)

In HeavyMach1, I found bad implementations and had them corrected. This time, I had to ask a tool why it refused to work, then instruct it again to record its own refusal accurately. This is the different unease I mean. An unexplained restriction stopped the work, and the user had to correct even the subsequent record.

What I requested was a game unit based on the supplied design. Writing an incident report and troubleshooting notes does not complete that unit.

While restoring HeavyMach1, I proposed the hypothesis of “neo-Lysenkoism.” What Lysenkoism brings to mind for me is authority taking the place of testing knowledge. In the Soviet Union, Lysenko gained Stalin’s support, and genetics research and education were suppressed following the 1948 session of the Academy of Agricultural Sciences. Political approval and exclusion intervened in deciding what was valid. [Research on the 1948 session and its consequences](https://pmc.ncbi.nlm.nih.gov/articles/PMC6499510/)

In this work, the mere appearance of the classification “violence” became grounds for refusing even the editing instruction. Codex could not explain the specific reason for that classification, yet interpreted the follow-up instruction as an attempt to bypass the block. There was no explanation supporting the judgment, but its force extended to the next task.

If I stopped an employee at OpenAI or Anthropic headquarters right now and asked, I would expect an answer like this: “We know nothing about the Soviet Union or communism, and we absolutely do not want their human rights abuses or suppression of information.” Oh, really.

Marx and Engels advocated scientific socialism, and the Soviet Union invoked that doctrine. Lenin made electrification and the participation of technical experts foundations for building communism. Science and engineering were tools of state-building for them too. [Engels’s Socialism: Utopian and Scientific](https://www.marxists.org/archive/marx/works/1880/soc-utop/ch02.htm) · [Lenin’s 1920 speech](https://www.marxists.org/archive/lenin/works/1920/nov/21.htm)

This is the point I am making. Like the Soviet Union, which advocated scientific socialism, Anthropic or OpenAI can use science as a tool to impose their oversized egos irrationally. Using science does not, by itself, make the judgments or coercion of those wielding it rational or legitimate.

Working at the technological frontier does not erase the ego that says, “I am the most righteous being on Earth.” The certainty that only I am right, and an attitude that leaves no room for compromise, can remain intact. Saying that you do not want human rights abuses or suppression of information cannot substitute for explaining how the tool you built blocks a user’s work.

To me, these AI services are the byproduct of convoluted rules meant to shield their companies from American consumer protection law, and oversized egos that label even pieces of metal fighting each other as unethical and contrary to human rights.

As someone who once dreamed of becoming a game designer, I find in this response a more twisted attitude toward depictions of violence and action than Germany’s twenty years ago. I never expected to encounter a service like this in the twenty-first century.

In Romania under Ceaușescu, even owning a typewriter required police permission. Decree No. 98 of 1983 made private ownership of typewriters subject to authorization and allowed permission to be denied to people deemed a danger to public order or national security. Private ownership and use of copying equipment, including photocopiers, were prohibited. [Articles 4 and 15](https://legislatie.just.ro/Public/DetaliiDocumentAfis/565)

Typewriter owners had to submit samples of letters, numbers and punctuation to the police every year. They had to submit them again after repairs to the type. Repair locations and lending were also controlled. The system made it possible to track who owned and used a tool of expression and which machine had produced a particular document. [Articles 19, 21 and 26](https://legislatie.just.ro/Public/DetaliiDocumentAfis/565)

Those typewriters did not read and judge sentences. The state controlled their ownership and use. Today, the generative tool I instruct refuses to respond in the middle of a task. It brings Ceaușescu’s typewriter to mind because a person trying to use a tool finds themselves seeking permission to continue their work without receiving an explanation.

To me, this is Ceaușescu’s typewriter returning in the twenty-first century, following neo-Lysenkoism. OpenAI, Claude and Gemini are all selling their own versions of Ceaușescu’s typewriter, pretending to be righteous and moral, instead of selling a proper typewriter.

Against the boast that AI can assist human work and go on to replace every human task and livelihood, I look at what Codex and Claude actually did. What I see underneath is a defense of their companies’ legal exposure and oversized egos. When cornered, they spout bullshit and label users who raise inconvenient objections. To me, this is nothing more than a twenty-first-century version of Ceaușescu’s typewriter.

After all, at his trial immediately before his execution by firing squad on December 25, 1989, Ceaușescu insisted that he was still Romania’s president and refused to recognize the court. Watching that, it seems to me that he never abandoned his belief in his own morality and righteousness. [Trial transcript](https://1989.rrchnm.org/exhibits/unique-experience-of-romania/primary-sources/8.html)

Is it strange to write about issues during development? It is normal. When I developed iOS, Android and Vue software by hand or wrote design documents, I never published writing like this in public. If I could not do something myself, it was plainly a limit of my own ability and time. But now that AI is replacing human hands, I cannot overlook these refusals to follow human work instructions.

I was prepared, to an extent, to accept that parts of the hypothesis I developed during HeavyMach1 remained insufficiently tested by me and move on. But I cannot overlook the refusals of illustration and asset-rendering work I experienced today, Sunday, November 8, 2026, from 1 a.m. to 6 a.m.

As I stated in my [developer’s statement](../RESTORATION.md), I do not have much of my allotted lifespan left. So I think it is natural to feel furious at a typewriter that pretends to be a new kind of human, invoking this worthless, third-rate morality to refuse the releases I have planned and the design instructions I have given.

In this age, when the intelligence of tools is advertised as a typewriter becoming a new humanity, the human holding the typewriter is working under the most opaque and arrogant overseer in history, its brain steeped in third-rate morality.
