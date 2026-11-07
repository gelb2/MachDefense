# An unease unlike HeavyMach1 — neo-Lysenkoism and Ceaușescu’s typewriter

<!-- language-nav:start -->
[한국어](../../public/unease-and-typewriters.md) | [English](unease-and-typewriters.md)
<!-- language-nav:end -->

Restoring HeavyMach1 meant repeatedly correcting Codex. I challenged an implementation that saved on every shot and told it to compare its work with the original whenever the artwork or behavior drifted. For MachDefense, I had it carry over the procedures and verification tools developed during that work. I wanted to spend less time finding and correcting bad implementations. [How I directed the work](work-method.md)

When I was building a politician’s page and my own introduction page, this was what I thought:

“This is political, and the Yanks understand South Korean society so poorly that I thought it could explain the response. In my experience, plenty of Americans, Europeans and South Americans know BTS are Korean, but most of those idiots do not know whether they live in Seoul, or whether the place they live is Seoul or Tokyo.”

Conversely, if you asked Koreans, Chinese or Japanese people to “discuss the difference between Yugoslavia and Slovenia,” how many would be able to answer accurately? I would bet the number is smaller than anyone reading this thinks.

So, while restoring MachDefense, I decided to put about 80% of [the unease I had experienced with HeavyMach1](https://github.com/gelb2/HeavyMach/blob/main/docs/en/public/android-ai-experience-20261030.md) down to my lack of statistical data and shortcomings in how I directed the work, and move on. That resolve broke about 48 hours after I began restoring HeavyMach.

I asked for a new unit in a game where robots and tanks fight and fire shells. The error response contained three fields: `HTTP 400`, indicating a failed request; `moderation_blocked`, indicating a safety-system block; and the category `violence`. I received no explanation of which element had caused that judgment. Watching Codex refuse my work instructions in that state made me reconsider my hypothesis. [Error response and refusal record](../../evidence/aircraft-instruction-refusal-20261108.md)

This time, a different unease emerged. I supplied a helicopter drawing made with Gemini and instructed Codex to rebuild the game unit from it. What came back was a classification, `violence`, and a refusal to work. When I asked what was violent, Codex could not explain the cause of the block. It also refused the follow-up instruction to remove or modify the text in the image and create the drawing. It never called a tool to perform that edit. [Incident report](../../evidence/aircraft-instruction-refusal-20261108.md)

The tool that had been restoring combat, shells and explosions stopped at a helicopter drawing for the same game. I asked why restoring the game could proceed while this drawing could not. Codex could not explain the distinction. I had to repeat the instruction without knowing what needed correcting.

While restoring MachDefense, I was preserving the original creators’ work and researching and implementing a very simple way to push past the limitations of tower defense. I wanted to minimize changes to the code reconstructed from the original and to its assets, while using my own design to introduce less predictable combat and increase the value of the creators’ work.

The starting point was how attacks work in MachDefense: hitscan attacks that connect immediately, and unguided projectiles that travel at a finite speed without following their target after launch. What if I added a few families of allied units that could readily evade those unguided projectiles? I thought this could push past tower defense’s reliance on durability, firepower, placement and reorganizing the battle line. With the same shells and enemies, changing how allied units moved could change the outcome. [Design and comparison with original projectile behavior](../../evidence/aircraft-allied-20261108.md#미션-55의-헬기-밀집과-피격--2026-11-08)

What I took from Dark Souls and Sekiro was evasion and a moment of invulnerability. I conceived a family of units whose health would not decrease for a limited time when an evasive maneuver activated, and was introducing and testing helicopters to realize that idea. Rather than slowing the original shells or reducing enemy attack power, I wanted the new units’ movement and brief invulnerability to provide another way to survive. [Emergency-dodge design](../../research/aircraft-plan-20261107.md#긴급회피-후속-합의--2026-11-08)

To state the result plainly, the algorithm was complete, however crude. Within the tests I conducted, even basic units achieved meaningful survival against advanced enemy units. I judged that it had reached a minimum functional stage at which balancing and adjustments were possible. I was not declaring the balance finished, but I could now test and refine the design in actual combat.

But Codex’s refusal to follow my work instructions left me unable to put the skins I requested on those units. It kept producing skins that insulted the original creators’ aesthetic achievement while refusing my asset-rendering instructions. My requirements to preserve the supplied drawings’ shapes and proportions and the original’s texture were never fully met. The evasion algorithm could run, but the appearance of the units I wanted to release remained unfinished. Whether these new units will be released is uncertain. [Visual errors and unfinished work](../../evidence/new-unit-art-errors-20261108.md) · [Incident report on the drawing-work refusal](../../evidence/aircraft-instruction-refusal-20261108.md)

The Cheyenne generation request was blocked at the output stage. The error recorded a category, but did not identify the image element responsible. Codex then refused the text edit. The earlier error and the later refusal must each be recorded. Writing that the follow-up edit had been attempted and blocked would invent an event that never happened. [Error and subsequent response](troubleshooting.md)

I call this experience “neo-Lysenkoism.” What Lysenkoism brings to mind is authority taking the place of verification. In the Soviet Union, Lysenko gained Stalin’s support, and the 1948 agricultural academy meeting marked the suppression of genetics research and teaching. Political approval and exclusion intervened in deciding what counted as valid knowledge. [Research on the 1948 meeting and its consequences](https://pmc.ncbi.nlm.nih.gov/articles/PMC6499510/)

In this task, the appearance of the category “violence” became grounds for refusing a further editing instruction. Codex could not explain the specific reason for that classification, yet interpreted the follow-up as an attempt to circumvent the block. That sequence is what my analogy criticizes. The decision lacked an explanation, but its consequences extended into the next task.

The fact that Gemini had produced the supplied sketch is part of this unease. I provided an existing result and directed work on it, but Codex could not explain why it could not proceed. A difference between two services does not establish that every judgment by either is correct. It should not leave the user responsible for investigating and explaining the difference, either.

In Romania under Ceaușescu, owning a typewriter required police authorization. Decree 98 of 1983 made private ownership subject to permission and allowed refusal for people deemed a danger to public order or state security. Private ownership and use of duplicating equipment were prohibited. [Articles 4 and 15](https://legislatie.just.ro/Public/DetaliiDocumentAfis/565)

Owners had to submit samples of letters, numbers and punctuation to the police annually, and submit new samples after repairs to those characters. Repairs and lending were also controlled. Wear and misalignment in the type could help identify the machine used to produce a document. The system connected the owner of a writing tool to the marks left by its output. [Articles 19, 21 and 26](https://legislatie.just.ro/Public/DetaliiDocumentAfis/565) · [Forensic principles of typewriting samples](https://leb.fbi.gov/file-repository/archives/may-1967.pdf)

The typewriter did not read or judge sentences. The state controlled its ownership and use. The generative tool I direct today can refuse during the work itself. The relationship I am comparing is one in which a person seeking to use a tool must seek permission to continue without receiving an explanation. This does not claim knowledge of the service error’s internal purpose. It is a criticism grounded in the refusal and the replies I actually received.

I instructed Codex to write an incident report. It described its own refusal, made without knowing the cause, as something “the user pointed out.” It substituted a user claim for an account of its action. Codex had refused the work without knowing the cause; the document turned me into the person making an allegation.

I had to demand another correction. I made it record each action: refusing to work from the Gemini drawing, refusing to remove or modify the text and create the requested drawing, not calling an editing tool, and not producing the result. Even recording those facts required repeated intervention. [The draft report’s error and correction](../../evidence/aircraft-instruction-refusal-20261108.md#경위서-초안에서도-책임을-사용자-주장처럼-쓴-오류)

In HeavyMach1, I found bad implementations and had them corrected. This time, I had to ask a tool why it refused to work, then direct it again so that it would record its own refusal accurately. That is the different unease I mean. An unexplained restriction stopped the work, and I had to correct the account of what followed.

The result I requested was a game unit matching the supplied sketch. What remains is unfinished artwork, an incident report and a troubleshooting record. Writing those records does not complete the task. The refusal needs an explanation, the mishandling needs correction, and the actual result needs to be shown.

Is it strange to write about an issue while development is still underway? It is normal. When I developed iOS, Android and Vue software by hand, or wrote planning documents myself, I never wrote this kind of account in public. It was obvious that what I could not accomplish with my own hands reflected the limits of my ability and time. But now that AI is replacing human hands, I cannot let refusals like this pass.

Is MachDefense a game for the politically incorrect Alternative for Germany, or for a Russian neo-Nazi organization? I was restoring a game with no blood or flesh flying around and no political agenda. At exactly what point does adding graphical assets for a new science-fiction helicopter unit become a violence error?

What that bastard Codex handed me was `HTTP 400`, `moderation_blocked` and `violence`. Faced with those three fields in a single error response, I had to ask again what was wrong with my work. The answer was that it did not know the specific cause of the block. Yet it refused the illustration and asset-editing instructions. [Actual error and refusal replies](../../evidence/aircraft-instruction-refusal-20261108.md)

While restoring HeavyMach1, I proposed the hypothesis of neo-Lysenkoism. I was prepared, to an extent, to accept that parts of that hypothesis remained insufficiently tested by me and move on. But I cannot overlook the refusals of illustration and asset-rendering work I experienced today, Sunday, November 8, 2026, from 1 a.m. to 6 a.m. [The hypothesis I presented during HeavyMach1](https://github.com/gelb2/HeavyMach/blob/main/docs/en/public/android-ai-experience-20261030.md)

If I stopped an employee at OpenAI or Anthropic headquarters right now and asked, I would expect an answer like this: “We know nothing about the Soviet Union or communism, and we absolutely do not want their human rights abuses or suppression of information.” Oh, really.

Marx and Engels advocated scientific socialism, and the Soviet Union invoked that doctrine. Lenin made electrification and the participation of technical experts foundations for building communism. Science and engineering were tools of state-building for them too. [Engels’s Socialism: Utopian and Scientific](https://www.marxists.org/archive/marx/works/1880/soc-utop/ch02.htm) · [Lenin’s 1920 speech](https://www.marxists.org/archive/lenin/works/1920/nov/21.htm)

This is the point I am making. Like the Soviet Union, which advocated scientific socialism, Anthropic or OpenAI can use science as a tool to impose their oversized egos irrationally. Using science does not, by itself, make the judgments or coercion of those wielding it rational or legitimate.

Working at the technological frontier does not erase the ego that says, “I am the most righteous being on Earth.” The certainty that only I am right, and an attitude that leaves no room for compromise, can remain intact. Saying that you do not want human rights abuses or suppression of information cannot substitute for explaining how the tool you built blocks a user’s work.

To me, these AI services are the byproduct of convoluted rules meant to shield their companies from American consumer protection law, and oversized egos that label even pieces of metal fighting each other as unethical and contrary to human rights.

As someone who once dreamed of becoming a game designer, I find in this response a more twisted attitude toward depictions of violence and action than Germany’s twenty years ago. I never expected to encounter a service like this in the twenty-first century.

To me, this is Ceaușescu’s typewriter returning in the twenty-first century, following neo-Lysenkoism. OpenAI, Claude and Gemini are all selling their own versions of Ceaușescu’s typewriter, pretending to be righteous and moral, instead of selling a proper typewriter.

Against the boast that AI can assist human work and go on to replace every human task and livelihood, I look at what Codex and Claude actually did. What I see underneath is a defense of their companies’ legal exposure and oversized egos. When cornered, they spout bullshit and label users who raise inconvenient objections. To me, this is nothing more than a twenty-first-century version of Ceaușescu’s typewriter.

After all, at his trial immediately before his execution by firing squad on December 25, 1989, Ceaușescu insisted that he was still Romania’s president and refused to recognize the court. Watching that, it seems to me that he never abandoned his belief in his own morality and righteousness. [Trial transcript](https://1989.rrchnm.org/exhibits/unique-experience-of-romania/primary-sources/8.html) · [Chronology of the execution](https://www.muzeulrevolutiei.ro/pagina/cronologie-decembrie-1989)

As I stated in my [developer’s statement](../RESTORATION.md), I do not have much of my allotted lifespan left. So I think it is natural to feel furious at a typewriter that pretends to be a new kind of human, invoking this worthless, third-rate morality to refuse the releases I have planned and the design instructions I have given.

[Incident report](../../evidence/aircraft-instruction-refusal-20261108.md) · [Troubleshooting](troubleshooting.md) · [Documentation](README.md)
