# A record of the helicopter work - What I overlooked. The fact that in Silicon Valley, DEI is more powerful than the Fifth Amendment.

I included the helicopter units in the release, but I would like only those who want to use them to do so.

My conclusion is that further debugging and design improvements to the helicopter units are impossible in the short term because of token consumption and Codex’s staggering refusals to carry out work instructions. Including my experience making images and artwork, I had almost never encountered such bizarre refusals and shirking even while making at least 200 pictures.

My goal is to restore the entire Mach series, not to babysit that bastard Codex and keep it happy. I do not want to spend what little time I have left on that kind of bullshit.

MachDefense still has further performance improvements and design tasks on its TODO list, including improvements for iPad and planning the lock-on system. But to restore the entire Mach series, I intend to postpone all TODO items and unfinished work related to the helicopter units.

I wanted to add four kinds of helicopter to MachDefense. While preserving the original creators’ work, I wanted to add a little of the unpredictability I had in mind to the combat they had made. That was all there was to the starting point of the work I spent the entire weekend on.

MachDefense has hitscan attacks that hit as soon as they are fired, and projectiles with a travel speed that can be dodged while they are in flight. I thought that adding allied units able to dodge the latter could change tower-defense combat that depended on durability, attack power, placement and reorganizing the battle line. I wanted to add another possibility to the original creators’ work through my own design, while minimizing changes to the code restored from the original and its assets.

What I took from Dark Souls and Sekiro was evasive movement and invulnerability during that moment. I had the helicopters dodge incoming attacks, with no loss of HP during the short period when evasion was active. The algorithm worked, however crude it was. In the tests I ran, even basic units achieved a meaningful survival rate against advanced enemy units. I judged that it had reached the minimum functionality needed to continue balancing and adjusting it.

One of the remaining tasks was to give those aircraft a suitable appearance. I had already instructed it to analyze the original and produce a release build. That work included the original art style, file preparation and optimization, part definitions and assembly, and the process by which they were drawn on the actual screen. The new units had to follow that process too. My requirement was to make artwork that belonged in the same game while preserving the shapes and proportions I had approved.

When it was supposed to adjust only the size, Codex replaced the approved gunship with a different generated picture. When told to enlarge the central fuselage, it matched the overall width including the engines and side supports, leaving the actual fuselage small. The gray design differed from the ochre installed version. Images that should have shown a left or right bank mixed in a nose-down or nose-up perspective and different wing shapes. I had to explain the measurement boundary again, along with what must remain the same when the same aircraft turns.

It also skipped the original assembly procedure. Instead of first checking the original CgBI optimization, definition data and part assembly, it used ordinary RGBA PNGs and definitions created inside C++. Some part files existed without being connected to the actual assembly tree, while AH-10’s rocket and air-strike weapons had firing points with no visible parts. Tilting the vertices of a front-view image looked like twisting a sheet of paper. Even the requirement for appearance and parts to change with upgrades was left out of a follow-up plan. I had to compare levels 1 and 10 and point out the identical appearance and changes amounting to little more than tiny dots.

When I asked it to stop helicopters from closing right up to a target, it overlaid ground-unit-style stopping at weapon range instead of fixing the approach while preserving the helicopters’ existing movement. It changed the evasive-speed calculation as well. Only after changing the feel of combat that I had told it to preserve did it withdraw those changes. Functional tests, file counts and matching hashes turned into reports that the approved design and movement had been preserved. I had to look at the installed build and find the errors again.

Not all these failures can be attributed to blocks from the image-generation module. Even outputs that had no blocking response involved disregarded approved designs, wrong measurements and crops, unconnected parts and missing checks. Those were faults in the work Codex chose, implemented and reported. The blocking errors piled on top of them.

But from that point on, the work became strangely tangled. It produced pictures that differed from the designs I had supplied. The wings and bodies looked different between the front-facing and turning views. Gaps appeared between the explanation that it had followed the original process and the actual implementation. When I told it to remake an image, other parts changed too. Paintwork and depth remain tasks for improvement, and Beluga looks as though different assets have been arbitrarily pasted onto it when it turns. The range issue, where helicopters move right up against their targets, is still unresolved. A report saying that the numerical tables had been checked was not the same thing as the problems I saw while playing.

I had felt an unease while restoring HeavyMach1 too. I wrote about it using the term “neo-Lysenkoism.” Even so, I did not try to blame everything on AI from the outset. When making a politician’s page and my own introduction page, I thought it might be because the subject was political and Americans did not sufficiently understand Korean society. In my experience, many Americans, Europeans and South Americans know that BTS are Korean, but often do not even know whether they live in Seoul or Tokyo.

Conversely, how many Koreans, Chinese or Japanese people could accurately explain the difference between Yugoslavia and Slovenia? I think fewer than the reader of this article would expect. So I tried to move on, attributing about 80% of the unease I had experienced with HeavyMach1 to my lack of statistical data and insufficient direction. That resolve broke about 48 hours into the next restoration project.

The task was to add science-fiction helicopters to a game where robots and tanks fight and fire shells. I had not asked for scenes with blood and flesh flying around, or pictures carrying a political agenda. Was MachDefense supposed to be a game for Alternative for Germany or some Russian neo-Nazi group? Yet the response contained `HTTP 400`, `moderation_blocked` and `violence`. Even when I asked what I had requested incorrectly, it did not explain which element of the picture had produced that classification.

After the tool returned an error, Codex’s own refusals followed. I gave it a design made by Gemini and told it to work on it again, but it refused. It also refused the subsequent instruction to erase or edit the text in the image and proceed. It did not even call a tool to perform that edit. A tool that had been restoring the original’s shells and explosions stopped before a helicopter picture intended for the same game, unable even to explain why.

When I told it to write an incident report, another problem appeared. Codex described its own action—refusing instructions without knowing the reason—as though it were something I had alleged. It was Codex that had refused the edit without knowing the cause, but in the incident report I became someone making a claim. I had to make it record each point again: that it had refused to work from the Gemini design, refused to delete or edit the text, and never called the editing tool. On top of the time spent getting the work done, I had to spend time getting it to accurately record the work it had not done.

Normally, when I developed for iOS, Android or Vue by hand, or wrote planning documents, I did not publish writing like this. When I could not do something with my own hands, it was clearly a limitation of my ability and time. But a tool claiming it would replace human hands was taking money, refusing to work, unable to explain why, and then twisting the account of what had happened. That was very hard to overlook. Such were the refusals to illustrate and render assets that I experienced from 1 a.m. to 6 a.m. on Sunday, November 8.

Counting the preserved request IDs, helicopter image generation was blocked five times on November 8 and twice on November 9: seven times in all. Every record contains HTTP 400, `moderation_blocked` and `violence`. The actual errors cover not only the gunship, AH-10 and Beluga, but also the edit to Scout’s fuselage proportions and both the generation and retry for Cheyenne. The record of the first parallel failure on November 9 does not establish which aircraft’s request failed.

Separate from these seven tool errors, Codex refused the instruction to work from the Gemini design and the subsequent instructions to erase or edit the text. It did not even call the text-editing tool. What I received was repeated blocking by the generation module, followed by Codex’s own failure to carry out the work.


I thought it over carefully. Why were new units and assets that had been relatively easy to add in HeavyMach1 causing so much trouble in MachDefense? Were the textures more complex? That could be it. Even so, by my common sense, it was difficult to accept a service promoting its frontier model falling apart like this while making four kinds of 2D unit. Was this really a task that could barely move forward without bringing in a harness and skills?

What I looked at again was the references. The aircraft I felt had caused the most problems were Beluga and AH-10. For Beluga, I had told it to consult the Su-25 and the Hind. They are Russian military aircraft, the sort seen in coverage of the war in Ukraine. The name Beluga also brought Russian to mind. For AH-10, I had initially told it to refer to Armored Core VI, then later told it to drop that reference.

The theme I read in Armored Core VI was connected to Nietzsche’s Übermensch. Thinking of Nietzsche brings with it the Nazi regime that distorted and exploited his philosophy, along with the old misunderstandings that mix up Nietzsche’s ideas and Nazi language. I had simply told it to use the aircraft’s shape as a reference, but I began to wonder whether the model was also reading the names and associations beyond that shape. Scout’s reference was the American-made AH-1Z, while Cheyenne began with the name of a canceled American helicopter and an old RTS game. Laying my references side by side, I noticed a difference I had not paid attention to before.

This is where my hypothesis came from. If an LLM reacts to elements that appear frequently in the media, sensitive words, and the visual depictions associated with them, those associations might follow even the lumps of metal I had ordered. A reference supplied to show an aircraft’s wings and fuselage could be read as a bundle of subjects such as Russia and war, Nietzsche and the Nazis. I was not given an explanation of the actual cause of the block. With that blank left open, I found myself comparing the failures I had experienced with the references I had supplied.

Thinking back, I had already experienced something similar while making pictures. I had to keep changing the text when making images inspired by Goya and Bacon, Caravaggio and Gentileschi, Beksiński and Egon Schiele. Another AI had also rejected my request for the style of a painter such as Ilya Repin, calling it an inappropriate picture. Early on, Gemini even refused requests to write in the tone of Vladimir Nabokov or Cormac McCarthy, or to create a scene or script that might appear in The Man in the High Castle.

I had overlooked the possibility that what happened then could also happen while making game units out of lumps of metal. Even when looking at games such as Call of Duty that change real firearms’ names and models, I had not connected the way licensing, litigation risk and interference by outside stakeholders reshape expression to my own work. The influence of DEI that I suspect is an extension of that. Had I known it would be like this, I would rather have limited my references to war museums or American films.

Thinking of the Shilka and Tunguska I added to HeavyMach1 raises more questions. In my hypothesis, what matters is which names currently appear more often and in what context. The Shilka, remembered as a Cold War weapon, and the Tunguska, still in service but less prominent than other air-defense systems, might have carried different associations for the model. If reports from the days when Shilkas faced the Israeli Air Force were pouring in as they do now, would it have readily made the same picture then? The same goes for references taken from other games. A story that calls to mind an unnamed pilot early in the Ukraine war might have fitted all the more neatly into that “correct” division between good and evil that I am mocking.

It may be a leap. I hope it is one too. If my inference is right, then to realize the designs and plans I want, I must not only explain shapes and functions but also somehow pass these insane criteria on my own. Faced with a tool that does not explain what triggered it, the person who paid has to rephrase their words while guessing at the tool’s associations and taboos.

This is why I write that in Silicon Valley, DEI is more powerful than the Fifth Amendment. At my workbench, the corporate “righteousness” gathered under that name felt as though it functioned as a private company’s constitution. It could halt work without giving a reason, and the user had to accept the explanation afterward too. After experiencing the answers that came back when I demanded an explanation for that power, the phrase no longer felt exaggerated.

Being at the frontier of science and technology does not eliminate this self-righteousness. The Soviet Union also invoked scientific socialism. My argument is not a rejection of Marx’s academic premise. It is that, like the Soviet Union that advocated scientific socialism, OpenAI or Anthropic can use science as a tool to irrationally impose its inflated ego. Valuing scientists’ and engineers’ abilities and holding an uncompromising conviction that only one’s own position is right can coexist within one organization.

I think that if I asked an employee at headquarters now, they would say that they absolutely do not want human-rights abuses or the suppression of information. That does not make the refusals I experienced disappear. What good is that declaration to someone faced with convoluted rules devised to protect the company from legal risk, and an attitude that imposes moral judgment even on fights between pieces of metal? As someone who once dreamed of becoming a game designer, I felt a way of thinking here even more twisted than Germany’s regulation of depictions of violence and action twenty years ago.

That is how Ceaușescu’s typewriter became superimposed on the neo-Lysenkoism that had come to mind during HeavyMach1. In Romania under Ceaușescu, even owning and using a typewriter was subject to permission and surveillance. The typewriter I use today advertises itself as an intelligence that can do human work. Yet the human holding that typewriter cannot get an explanation of why the work they ordered cannot be done. A typewriter pretending to be righteous and moral holds the power to permit the work.

As I stated in my developer’s statement, I do not have much of my allotted lifespan left. So I think it is natural to feel furious at a typewriter pretending to be a new kind of human, refusing my planned releases and design instructions because of this worthless, third-rate morality. I have no refill voucher that will replenish my time.

In this age that trumpets the intelligence of tools and proclaims the typewriter has become a new humanity, the human holding the typewriter is actually working under an overseer opaque about its own ego, arrogant, and with its brain steeped in third-rate morality. I merely supplied a few design references while restoring a game with no political agenda, yet I am spending more time explaining the tool’s taboos than the helicopter’s wings. Who does a mere typewriter, paid to type, think it is to judge its user’s morality?

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
