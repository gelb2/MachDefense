# 가로 타이틀 벼랑·하늘 연결 시안

내장 image_gen을 사용했다. 첫 후보와 경계 수정 후보는 생성 그림에 기체 조각이 남아 채택하지 않았다. 세 번째 생성본은 자연물 전용 배경이며, 원본 중앙 JPG를 별도로 합성한다.

## 원본에서 가로 외곽 생성

Use case: compositing / seamless horizontal outpainting.
Input is the ORIGINAL 640x960 title illustration. Extend ONLY horizontally into a 16:9 wide landscape. Place the unchanged full original in the exact central 37.5% of the output, full height. Match its painterly brushwork, subdued colors and level of detail.
The new sides must be exclusively NATURAL scenery continuous with the original: cliffs, overcast blue sky, cloud, smoke. No extra vehicles or extending cropped vehicles. No manmade shapes.
Critical geometry: This scene is seen from LOW DOWN INSIDE a deep narrow rocky gorge. At left and right, massive dark rock walls fill the lower TWO THIRDS. Their upper silhouettes start at the center joining edge around 32% image height and descend gently toward the far outer edges around 43% height. These are CLOSE dark rocky walls, NOT a distant open panoramic mountain valley. At midheight rock and smoke match the darkness of the cropped machines at the original edges WITHOUT making more machines. At bottom the entire side cliffs are nearly black brown, matching original. Do NOT add bright orange rivers, lava canyons or fires outside. Original center fire remains the only bright warm focal point.
Upper-left join has pale almost-white cloud at very top and gray-blue clouds below. Upper-right join has muted blue at very top, gray-white cloud below; match that EXACTLY. No bright saturated blue on sides. Preserve source lighting/contrast. Paint only the side extensions; original center remains precisely unchanged. Seamless sky/cloud/rock contours crossing joins, no straight vertical tonal boundaries or pasted-poster outline.
No additional mech, tank, plane, gun, leg, track, armor, building, wreck, lettering or logos. Do not complete vehicles. Do not change logo, objects, proportions or crop in original center. Output complete 16:9 landscape.

## 실제 합성 화면을 참조한 경계 수정

Edit this wide composite image. Fix ONLY the side scenery. The central original rectangle from x31.25% to x68.75% across the full height is LOCKED. Keep every central pixel, including its sky brightness and all figures and lettering, unchanged. DO NOT change overall crop or layout.
Remove the small dark mechanical protrusion just outside the central left edge around y30-37%; replace with NATURAL dark smoke/cloud and rough rock, no straight metal geometry. No machinery whatsoever outside the locked rectangle.
Remove the straight vertical seams where the original center meets the side scenery. At the RIGHT sky join, the new sky must EXACTLY match the lighter blue of the original center, with the existing cloud connected naturally. At the LEFT sky join, connect the pale upper cloud and the lower blue-gray cloud naturally across the boundary. At BOTH lower cliff joins (roughly y45% to bottom), match the original center's dark warm brown shadows by adjusting the OUTSIDE rocks immediately adjacent to the boundary, then fade the color adjustment naturally outward. The straight vertical edge must disappear into natural rock strata and smoke. Avoid a central poster appearance.
Leave central original rectangle unchanged. Paint corrections only outside it. Preserve mostly existing side rock composition; change only necessary color and seam continuity. No new objects, no vehicles, no buildings, no extra fires, no lettering. Output the same 16:9 wide composition.

## 자연물 배경 분리

Edit this picture into an ENVIRONMENT-ONLY background layer for compositing. REMOVE ALL machinery and vehicles: remove both the mech and tank, all guns and armor, every aircraft in the sky. REMOVE ALL lettering and logos. Replace them with matching NATURAL clouds, dark smoke, rough cliff rock. Keep the existing side cliffs, sky, clouds and their positions, lighting, palette, dimensions EXACTLY unchanged. Do not shift or recompose the environment. The background will be placed behind a separate unchanged original picture, so no mechanical subject or letter may remain in this environment layer. This is a painterly blue-gray cloudy sky above a deep black-brown rocky ravine, with warm orange light only deep in the center. Output same 16:9 image. Absolutely no machines, metal, buildings, ruins, text, logos or symbols anywhere.
