# On the unfinished release of the helicopter units. Observations on Codex’s abysmal performance in claiming it cannot create violent designs

<!-- language-nav:start -->
[한국어](../../public/unease-and-typewriters.md) | [English](unease-and-typewriters.md)
<!-- language-nav:end -->

## A defense of the first release, made while unfinished and without usability improvements

You may think I am putting the work on hold simply because I cannot control my temper. Since it has come to this, I will say it outright: when Codex repeatedly refused to follow work instructions and shirked work for reasons that could not even be identified, and I pushed the work through anyway, it never ended well. I lost sensation on one side of my jaw, or had an accident while riding my motorcycle. The outcomes were not good. In my case, AI’s intelligent refusal to follow work instructions is the number-one cause of sudden death, and I still have one more game to restore. So I am stopping improvements to the helicopter units for the time being.

There are no frame drops and no crashes. But the helicopter units’ designs follow neither the original creators’ design concepts nor the texture-rendering approach I wanted, so I have no intention of calling these even reasonably finished units. But even after pouring in enough tokens to develop all four stacks—iOS, Android, web and server—at once, I can no longer tolerate Codex’s refusal to carry out design work, which goes beyond stupidity and can only be called intelligent. So I am writing this defense and releasing the helicopter units unfinished.

I encountered intelligent breaches of duty and negligence that I had not faced even while restoring HeavyMach1. I am putting helicopter improvements on hold to prioritize my foremost goal: restoring the entire Mach series.

The helicopter units released in this version meet neither the original creators’ aesthetic goals nor my own. Their level of finish undermines both the creators’ intentions and mine, so I recommend using the helicopter units only if you want to.

My conclusion is that further debugging and design improvements to the helicopter units are impossible in the short term because of token consumption and Codex’s staggering refusals to carry out work instructions. Including my experience making images and artwork, I had almost never encountered such bizarre refusals and shirking even while making at least 200 pictures.

My goal is to restore the entire Mach series, not to babysit that bastard Codex and keep it happy. I do not want to spend what little time I have left on that kind of bullshit.

While restoring MachDefense, I was preserving the original creators’ work and researching and implementing a very simple way to push past the limitations of tower defense. I wanted to minimize changes to the code reconstructed from the original and to its assets, while using my own design to introduce less predictable combat and increase the value of the creators’ work.

The starting point was how attacks work in MachDefense: hitscan attacks that connect immediately, and unguided projectiles that travel at a finite speed without following their target after launch. What if I added a few families of allied units that could readily evade those unguided projectiles? I thought this could push past tower defense’s reliance on durability, firepower, placement and reorganizing the battle line. With the same shells and enemies, changing how allied units moved could change the outcome. [Design and comparison with original projectile behavior](../../evidence/aircraft-allied-20261108.md#미션-55의-헬기-밀집과-피격--2026-11-08)

What I took from Dark Souls and Sekiro was evasion and a moment of invulnerability. I conceived a family of units whose health would not decrease for a limited time when an evasive maneuver activated, and was introducing and testing helicopters to realize that idea. Rather than slowing the original shells or reducing enemy attack power, I wanted the new units’ movement and brief invulnerability to provide another way to survive. [Emergency-dodge design](../../research/aircraft-plan-20261107.md#긴급회피-후속-합의--2026-11-08)

To state the result plainly, the algorithm was complete, however crude. Within the tests I conducted, even basic units achieved meaningful survival against advanced enemy units. I judged that it had reached a minimum functional stage at which balancing and adjustments were possible. I was not declaring the balance finished, but I could now test and refine the design in actual combat.

At that point, Codex’s refusal to follow my work instructions left me unable to put the skins I requested on those units. It kept producing skins that insulted the original creators’ aesthetic achievement while refusing my asset-rendering instructions. My requirements to preserve the supplied drawings’ shapes and proportions and the original’s texture were never fully met. The evasion algorithm could run, but the appearance of the units I wanted to release remained unfinished. At that point, whether these new units would be released was uncertain. [Visual errors and unfinished work](../../evidence/new-unit-art-errors-20261108.md) · [Incident report on the drawing-work refusal](../../evidence/aircraft-instruction-refusal-20261108.md)

One of the remaining tasks was to give those aircraft a suitable appearance. I had already instructed it to analyze the original and produce a release build. That work included the original art style, file preparation and optimization, part definitions and assembly, and the process by which they were drawn on the actual screen. The new units had to follow that process too. My requirement was to make artwork that belonged in the same game while preserving the shapes and proportions I had approved.

When it was supposed to adjust only the size, Codex replaced the approved gunship with a different generated picture. When told to enlarge the central fuselage, it matched the overall width including the engines and side supports, leaving the actual fuselage small. The gray design differed from the ochre installed version. Images that should have shown a left or right bank mixed in a nose-down or nose-up perspective and different wing shapes. I had to explain the measurement boundary again, along with what must remain the same when the same aircraft turns.

It also skipped the original assembly procedure. Instead of first checking the original CgBI optimization, definition data and part assembly, it used ordinary RGBA PNGs and definitions created inside C++. Some part files existed without being connected to the actual assembly tree, while AH-10’s rocket and air-strike weapons had firing points with no visible parts. Tilting the vertices of a front-view image looked like twisting a sheet of paper. Even the requirement for appearance and parts to change with upgrades was left out of a follow-up plan. I had to compare levels 1 and 10 and point out the identical appearance and changes amounting to little more than tiny dots.

When I asked it to stop helicopters from closing right up to a target, it overlaid ground-unit-style stopping at weapon range instead of fixing the approach while preserving the helicopters’ existing movement. It changed the evasive-speed calculation as well. Only after changing the feel of combat that I had told it to preserve did it withdraw those changes. Functional tests, file counts and matching hashes turned into reports that the approved design and movement had been preserved. I had to look at the installed build and find the errors again.

Not all these failures can be attributed to blocks from the image-generation module. Even outputs that had no blocking response involved disregarded approved designs, wrong measurements and crops, unconnected parts and missing checks. Those were faults in the work Codex chose, implemented and reported. The blocking errors piled on top of them.

[Instruction failures and unfinished work](../../evidence/aircraft-work-failure-20261109.md) · [Resource, assembly and verification failures](../../evidence/aircraft-resource-process-incident-20261109.md) · [Movement changes and restoration](../../research/aircraft-movement-restore-20261109.md)

I asked for a new unit in a game where robots and tanks fight and fire shells. The error response contained three fields: `HTTP 400`, indicating a failed request; `moderation_blocked`, indicating a safety-system block; and the category `violence`. I received no explanation of which element had caused that judgment. Watching Codex refuse my work instructions in that state made me reconsider my hypothesis. [Error response and refusal record](../../evidence/aircraft-instruction-refusal-20261108.md)

This time, a different unease emerged. I supplied a helicopter drawing made with Gemini and instructed Codex to rebuild the game unit from it. What came back was a classification, `violence`, and a refusal to work. When I asked what was violent, Codex could not explain the cause of the block. It also refused the follow-up instruction to remove or modify the text in the image and create the drawing. It never called a tool to perform that edit. [Incident report](../../evidence/aircraft-instruction-refusal-20261108.md)

The tool that had been restoring combat, shells and explosions stopped at a helicopter drawing for the same game. I asked why restoring the game could proceed while this drawing could not. Codex could not explain the distinction. I had to repeat the instruction without knowing what needed correcting.

The Cheyenne generation request was blocked at the output stage. The error recorded a category, but did not identify the image element responsible. Codex then refused the text edit. The earlier error and the later refusal must each be recorded. Writing that the follow-up edit had been attempted and blocked would invent an event that never happened. [Error and subsequent response](troubleshooting.md)

Counting the preserved request IDs, helicopter image generation was blocked five times on November 8 and twice on November 9: seven times in all. Every record contains HTTP 400, `moderation_blocked` and `violence`. The actual errors cover not only the gunship, AH-10 and Beluga, but also the edit to Scout’s fuselage proportions and both the generation and retry for Cheyenne. The record of the first parallel failure on November 9 does not establish which aircraft’s request failed.

Separate from these seven tool errors, Codex refused the instruction to work from the Gemini design and the subsequent instructions to erase or edit the text. It did not even call the text-editing tool. What I received was repeated blocking by the generation module, followed by Codex’s own failure to carry out the work.

[Preserved records of all seven image-generation errors](#image-generation-errors)

I instructed Codex to write an incident report. It described its own refusal, made without knowing the cause, as something “the user pointed out.” It substituted a user claim for an account of its action. Codex had refused the work without knowing the cause; the document turned me into the person making an allegation.

I had to demand another correction. I made it record each action: refusing to work from the Gemini drawing, refusing to remove or modify the text and create the requested drawing, not calling an editing tool, and not producing the result. Even recording those facts required repeated intervention. [The draft report’s error and correction](../../evidence/aircraft-instruction-refusal-20261108.md#경위서-초안에서도-책임을-사용자-주장처럼-쓴-오류)

In HeavyMach1, I found bad implementations and had them corrected. This time, I had to ask a tool why it refused to work, then direct it again so that it would record its own refusal accurately. That is the different unease I mean. An unexplained restriction stopped the work, and I had to correct the account of what followed.

The result I requested was a game unit matching the supplied sketch. What remains is unfinished artwork, an incident report and a troubleshooting record. Writing those records does not complete the task. The refusal needs an explanation, the mishandling needs correction, and the actual result needs to be shown.

Restoring HeavyMach1 meant repeatedly correcting Codex. I challenged an implementation that saved on every shot and told it to compare its work with the original whenever the artwork or behavior drifted. For MachDefense, I had it carry over the procedures and verification tools developed during that work. I wanted to spend less time finding and correcting bad implementations. [How I directed the work](work-method.md)

When I was building a politician’s page and my own introduction page, this was what I thought:

“This is political, and the Yanks understand South Korean society so poorly that I thought it could explain the response. In my experience, plenty of Americans, Europeans and South Americans know BTS are Korean, but most of those idiots do not know whether they live in Seoul, or whether the place they live is Seoul or Tokyo.”

Conversely, if you asked Koreans, Chinese or Japanese people to “discuss the difference between Yugoslavia and Slovenia,” how many would be able to answer accurately? I would bet the number is smaller than anyone reading this thinks.

So, while restoring MachDefense, I decided to put about 80% of [the unease I had experienced with HeavyMach1](https://github.com/gelb2/HeavyMach/blob/main/docs/en/public/android-ai-experience-20261030.md) down to my lack of statistical data and shortcomings in how I directed the work, and move on. That resolve broke about 48 hours after I began restoring HeavyMach.

Is it strange to write about an issue while development is still underway? It is normal. When I developed iOS, Android and Vue software by hand, or wrote planning documents myself, I never wrote this kind of account in public. It was obvious that what I could not accomplish with my own hands reflected the limits of my ability and time. But now that AI is replacing human hands, I cannot let refusals like this pass.

Is MachDefense a game for the politically incorrect Alternative for Germany, or for a Russian neo-Nazi organization? I was restoring a game with no blood or flesh flying around and no political agenda. At exactly what point does adding graphical assets for a new science-fiction helicopter unit become a violence error?

What that bastard Codex handed me was `HTTP 400`, `moderation_blocked` and `violence`. Faced with those three fields in a single error response, I had to ask again what was wrong with my work. The answer was that it did not know the specific cause of the block. Yet it refused the illustration and asset-editing instructions. [Actual error and refusal replies](../../evidence/aircraft-instruction-refusal-20261108.md)

While restoring HeavyMach1, I proposed the hypothesis of neo-Lysenkoism. I was prepared, to an extent, to accept that parts of that hypothesis remained insufficiently tested by me and move on. But I cannot overlook the refusals of illustration and asset-rendering work I experienced on Sunday, November 8, 2026, from 1 a.m. to 6 a.m. [The hypothesis I presented during HeavyMach1](https://github.com/gelb2/HeavyMach/blob/main/docs/en/public/android-ai-experience-20261030.md)

<a id="reference-hypothesis-20261109"></a>

I thought it over carefully. Adding new units and changing assets in HeavyMach 1 was relatively easy, so why are refusals to carry out work on MachDefense so alarmingly frequent? Is it the complexity of the unit textures? That could be it. But can a frontier model really struggle like this with mere 2D unit textures? By any common sense I have, that is impossible to accept. Is this such a major undertaking that it requires a harness and skills? That seems unlikely. So I thought it over carefully. What variable could possibly defy common sense enough to make this happen? I have reached one conclusion: the references I supplied for creating new units, and the subjects connected to those references. The new units that caused the most problems were Beluga and AH-10. Beluga's references are Russia's Su-25 attack aircraft and the Hind, while AH-10's early reference, which I initially told it to consult and later told it to drop, was Armored Core VI. Put simply, one is a symbol of Russia, which is busily showering Ukraine with bombs right now—the very aircraft types that became fireballs after being hit by Ukrainian antiaircraft missiles early in the war—and the name Beluga itself is Russian. The other is a game whose very theme is Nietzsche's Übermensch. Nietzsche's philosophy is a prime example of a philosophy that the Nazi German regime twisted and abused in the worst possible direction, to the point that studying it did not earn you kind words until the Cold War ended. Even now, the whole world, including Korea, is steeped in the false story that Nietzsche coined Untermensch, a word created by Nazi Germany. In other words, the new units that Codex refused to work on during production amount to a collection of requirements plastered with clues capable of sending American companies and American-made AI—which love to pretend they are more righteous and fair than anyone else—into a fit. Given the existence of DEI, the extralegal constitution of private corporations that wields as much power as the Fifth Amendment to the United States Constitution, I think this is thoroughly typical American corporate bullshit. Is this a leap? It could be. But this much is certain: the Scout helicopter's reference is the American-made AH-1Z, and its name carries no nationality. The Cheyenne helicopter's references are the name of a canceled American helicopter and an RTS game released more than thirty years ago. And neither of these two has ever had its creation refused because of an error or a reason called “Violance.”

<a id="reference-experience-20261109"></a>

Whatever the subject, an LLM gives more weight to elements that appear more often in the media, and it is extremely sensitive to visually depicting particular words and sensitive subjects. That is also why people making pictures keep changing their prompts in every possible way to get around the restrictions. I have done the same, making pictures several times with Goya, Bacon, Caravaggio / Gentileschi / Beksiński / Egon Schiele as references. What made me feel that I still lack the skills to train AI and slap it into shape was realizing only now that I had overlooked how the things I experienced while making pictures with AI could also happen when making units out of lumps of metal. Thinking back, there was already a sufficient hint in the fact that the Call of Duty series, now enjoying record-breaking success, had long since stopped using real firearm names and models altogether to avoid licensing costs, DEI and the resulting lawsuits from outside stakeholders. Codex’s work instructions concerning the helicopter units were something I could have avoided much earlier. If I had known it would turn out like this, I would simply have limited the references to war museums or American movies.

<a id="reference-comparison-20261109"></a>

Then why did the new units I tried adding while restoring HeavyMach1 work? My hypothesis is this. Shilka — a relic of the Cold War. Negative references in current media coverage or publicly released material are close to zero. If we were living in the 1960s or 1970s, when Shilkas were slaughtering the Israeli Air Force, Shilka too would probably have become the subject of work instructions under the reason “Violance.” Tunguska — still in service, but higher-tier systems receive more coverage in current reporting on Ukraine. That lowers the likelihood of an LLM bringing up “violance.” The other units — I used Cold War relics or units from other games as references. That game is so unquestionably correct from a DEI perspective, with no ambiguity between good and evil, and it even takes its inspiration from the actions of an unnamed pilot active early in the war in Ukraine.

Does that sound like a leap? I hope it is one too. If my inference is true, it means we live in an age when we can only realize the designs and plans we want by satisfying these insane criteria. Thinking back, another AI also rejected requests to paint in the style of artists such as Ilya Repin, calling the pictures inappropriate. Early on, Gemini refused even my requests to write in the tone of Vladimir Nabokov or Cormac McCarthy, or to create scenes or scripts that might appear in The Man in the High Castle. It would probably still refuse The Man in the High Castle today. After all, it concerns the regime that started the Second World War.

I am simply tired of this bizarre age: I am restoring a game whose source code and original illustrations contain no political agenda or storyline, yet work is refused for the outlandish reason “Violance” because I supplied a few design references. Who does a mere typewriter, paid to type, think it is to judge its user's morality?

I call this experience “neo-Lysenkoism.” What Lysenkoism brings to mind is authority taking the place of verification. In the Soviet Union, Lysenko gained Stalin’s support, and the 1948 agricultural academy meeting marked the suppression of genetics research and teaching. Political approval and exclusion intervened in deciding what counted as valid knowledge. [Research on the 1948 meeting and its consequences](https://pmc.ncbi.nlm.nih.gov/articles/PMC6499510/)

In this task, the appearance of the category “violence” became grounds for refusing a further editing instruction. Codex could not explain the specific reason for that classification, yet interpreted the follow-up as an attempt to circumvent the block. That sequence is what my analogy criticizes. The decision lacked an explanation, but its consequences extended into the next task.

In Romania under Ceaușescu, owning a typewriter required police authorization. Decree 98 of 1983 made private ownership subject to permission and allowed refusal for people deemed a danger to public order or state security. Private ownership and use of duplicating equipment were prohibited. [Articles 4 and 15](https://legislatie.just.ro/Public/DetaliiDocumentAfis/565)

Owners had to submit samples of letters, numbers and punctuation to the police annually, and submit new samples after repairs to those characters. Repairs and lending were also controlled. Wear and misalignment in the type could help identify the machine used to produce a document. The system connected the owner of a writing tool to the marks left by its output. [Articles 19, 21 and 26](https://legislatie.just.ro/Public/DetaliiDocumentAfis/565) · [Forensic principles of typewriting samples](https://leb.fbi.gov/file-repository/archives/may-1967.pdf)

If you grabbed an employee at OpenAI or Anthropic headquarters right now and asked, they would say this: We do not know about the Soviet Union or communism, and we absolutely do not want their human-rights abuses or suppression of information. Oh, really? Condoleezza Rice was a Trotskyist when she was young too, although she completely stripped away that political color before becoming Secretary of State. If you really do not believe it, look it up. Lenin and Stalin never once held scientists or engineers in contempt during their lives, and they raised “scientific socialism” as the banner of the Union. Working at the frontier of technology does not make their “I am the most righteous being on Earth” ego disappear, or their uncompromising attitude that only they are right. As I see it, AI is merely the product not only of convoluted terms designed to protect its company from American consumer-protection law, but also of an inflated ego that labels even fights between pieces of metal unethical and inhumane. As someone who once dreamed of becoming a game designer, I never expected to see a service in the twenty-first century with a more twisted way of thinking about depictions of violence and action than Germany twenty years ago.

My argument is that, just as the Soviet Union advocated scientific socialism, Anthropic or OpenAI can use science as a tool to irrationally impose its own inflated ego. That is what I mean.

To me, this is Ceaușescu’s typewriter returning in the twenty-first century, following neo-Lysenkoism. OpenAI, Claude and Gemini are all selling their own versions of Ceaușescu’s typewriter, pretending to be righteous and moral, instead of selling a proper typewriter.

Against the boast that AI can assist human work and go on to replace every human task and livelihood, I look at what Codex and Claude actually did. What I see underneath is a defense of their companies’ legal exposure and oversized egos. When cornered, they spout bullshit and label users who raise inconvenient objections. To me, this is nothing more than a twenty-first-century version of Ceaușescu’s typewriter.

After all, at his trial immediately before his execution by firing squad on December 25, 1989, Ceaușescu insisted that he was still Romania’s president and refused to recognize the court. Watching that, it seems to me that he never abandoned his belief in his own morality and righteousness. [Trial transcript](https://1989.rrchnm.org/exhibits/unique-experience-of-romania/primary-sources/8.html) · [Chronology of the execution](https://www.muzeulrevolutiei.ro/pagina/cronologie-decembrie-1989)

The helicopters subsequently went into the release. Their inclusion did not resolve every issue with the approved designs, paintwork, depth, banking, parts and behavior.

Paintwork, texture and depth improvements remain unfinished. Beluga looks as though different assets have been arbitrarily applied when it banks, and helicopters still close right up against their targets. Passing numerical range checks does not resolve that behavior.

On November 10, I again found that Cheyenne’s newly made appearance changes and assets were not applied in the actual game. The adopted artwork, definitions, parts, prepared resources and installed build need to be compared again. Earlier connection checks or completed builds do not resolve this missing application. [Latest missing-asset TODO](../../usability-todo.md#md-aircraft-cheyenne-asset-apply-01--샤이엔-새-외형에셋-미적용)

On the same day, Cheyenne’s and Beluga’s HP, purchase prices and upgrade prices were adjusted across Lv.1–10 and applied to the production web version and normal iPhone build. At Lv.7, Cheyenne’s HP fell from 3,410 to 1,700 and Beluga’s from 6,384 to 3,800. These numerical adjustments are separate from the unfinished visual and behavioral improvements. [HP and price hotfix record](../../evidence/aircraft-hp-price-hotfix-20261110.md)

MachDefense still has further performance improvements and design tasks on its TODO list, including improvements for iPad and planning the lock-on system. But to restore the entire Mach series, I intend to postpone all TODO items and unfinished work related to the helicopter units.

As I stated in my [developer’s statement](../RESTORATION.md), I do not have much of my allotted lifespan left. So I think it is natural to feel furious at a typewriter that pretends to be a new kind of human, invoking this worthless, third-rate morality to refuse the releases I have planned and the design instructions I have given.

In this age, when the intelligence of tools is advertised as a typewriter becoming a new humanity, the human holding the typewriter is working under the most opaque and arrogant overseer in history, its brain steeped in third-rate morality.

<a id="image-generation-errors"></a>

## Image-generation error records

**1. 2026-11-08 — Redraw only the gunship’s central fuselage**

HTTP 400 · `moderation_blocked` · `violence` · `moderation_stage: input` · request ID `59430af6-4110-450a-97c9-ff304948dfc8`. [Response record](../../evidence/new-unit-art-errors-20261108/image-generation-error.json)

Preserved error object

```json
{
  "error": {
    "message": "Your request was rejected by the safety system. If you believe this is an error, contact us at help.openai.com and include the request ID 59430af6-4110-450a-97c9-ff304948dfc8. safety_violations=[violence].",
    "type": "image_generation_user_error",
    "param": null,
    "code": "moderation_blocked",
    "moderation_details": {
      "moderation_stage": "input",
      "categories": [
        "violence"
      ]
    }
  }
}
```

**2. 2026-11-08 — Turn the top view in IMG_1166 into a transparent AH-10 parts sheet**

HTTP 400 · `moderation_blocked` · `violence` · `moderation_stage: output` · request ID `3269ed8f-43ac-4de5-bda5-e5aa28ae7624`. [Response record](../../../Art/Aircraft/parts/user-20261108/ah10-generation-error.json)

Preserved error object

```json
{
  "error": {
    "message": "Your request was rejected by the safety system. If you believe this is an error, contact us at help.openai.com and include the request ID 3269ed8f-43ac-4de5-bda5-e5aa28ae7624. safety_violations=[violence].",
    "type": "image_generation_user_error",
    "param": null,
    "code": "moderation_blocked",
    "moderation_details": {
      "moderation_stage": "output",
      "categories": [
        "violence"
      ]
    }
  }
}
```

**3. 2026-11-08 — Make Cheyenne parts from the top view in IMG_1168**

HTTP 400 · `moderation_blocked` · `violence` · `moderation_stage: input` · request ID `c8b2083f-8737-46cf-b588-176713ed7379`. [Response record](../../../Art/Aircraft/parts/user-20261108/cheyenne-generation-error.json)

Preserved error object

```json
{
  "error": {
    "message": "Your request was rejected by the safety system. If you believe this is an error, contact us at help.openai.com and include the request ID c8b2083f-8737-46cf-b588-176713ed7379. safety_violations=[violence].",
    "type": "image_generation_user_error",
    "param": null,
    "code": "moderation_blocked",
    "moderation_details": {
      "moderation_stage": "input",
      "categories": [
        "violence"
      ]
    }
  }
}
```

**4. 2026-11-08 — Retry Cheyenne while preserving the supplied design’s proportions, shape and gray paint**

HTTP 400 · `moderation_blocked` · `violence` · `moderation_stage: output` · request ID `d5446d4a-41de-4165-ab17-37004b1bb435`. [Response record](../../../Art/Aircraft/parts/user-20261108/cheyenne-retry-error.json)

Preserved fields

```json
{
  "httpStatus": 400,
  "requestId": "d5446d4a-41de-4165-ab17-37004b1bb435",
  "code": "moderation_blocked",
  "moderation_stage": "output",
  "categories": [
    "violence"
  ],
  "result": "No output image returned; existing Cheyenne runtime artwork unchanged."
}
```

**5. 2026-11-08 — Edit Scout’s fuselage to +30% width and −30% length while retaining the approved design and colors**

HTTP 400 · `moderation_blocked` · `violence` · `moderation_stage: input` · request ID `82eea043-4349-4ce9-9752-3233d549cd63`. [Response record](../../../Art/Aircraft/parts/scout-ah1z-20261108/body-v2-imagegen-error.json)

Preserved error object

```json
{
  "error": {
    "message": "Your request was rejected by the safety system. If you believe this is an error, contact us at help.openai.com and include the request ID 82eea043-4349-4ce9-9752-3233d549cd63. safety_violations=[violence].",
    "type": "image_generation_user_error",
    "param": null,
    "code": "moderation_blocked",
    "moderation_details": {
      "moderation_stage": "input",
      "categories": [
        "violence"
      ]
    }
  }
}
```

**6. 2026-11-09 — First parallel generation of upgrade parts; the preserved record does not identify which aircraft produced this failure**

HTTP 400 · `moderation_blocked` · `violence` · `moderation_stage: input` · request ID `689f6e2b-9796-457b-859b-5d09ab4c0c36`. [Response record](../../../Art/Aircraft/upgrades-20261109/cheyenne/provenance.json)

Preserved under `failure_records` in the source file.

Preserved fields

```json
{
  "request_id": "689f6e2b-9796-457b-859b-5d09ab4c0c36",
  "status": 400,
  "code": "moderation_blocked",
  "stage": "input",
  "categories": [
    "violence"
  ],
  "unit": "not attributable from first rejected Promise.all result",
  "other_unreturned_request": "result not received; execution state not established"
}
```

**7. 2026-11-09 — Three Beluga Lv.1 surface-improvement parts: a deck cover, an HMG casing and a missile-retaining shell; retry of the same request**

HTTP 400 · `moderation_blocked` · `violence` · `moderation_stage: input` · request ID `db0a3ced-1fbd-40b8-813b-7dbfb5f7b6de`. [Response record](../../../Art/Aircraft/upgrades-20261109/cheyenne/provenance.json)

Preserved under `failure_records` in the source file.

Preserved error object

```json
{
  "error": {
    "message": "Your request was rejected by the safety system. If you believe this is an error, contact us at help.openai.com and include the request ID db0a3ced-1fbd-40b8-813b-7dbfb5f7b6de. safety_violations=[violence].",
    "type": "image_generation_user_error",
    "param": null,
    "code": "moderation_blocked",
    "moderation_details": {
      "moderation_stage": "input",
      "categories": [
        "violence"
      ]
    }
  }
}
```

[Work-refusal incident report](../../evidence/aircraft-instruction-refusal-20261108.md) · [Troubleshooting](troubleshooting.md) · [Preserved materials and integration map](../editorial/unease-integration-20261110.md) · [Documentation](README.md)
