# 적 공습기 F-15 STOL/MTD

[아트 조사와 제작안](../../docs/research/enemy-airstrike-art-20261127.md)에 참조와 전투 크기를 기록했다. 첫 시안은 원작 공습기 PNG를 ImageGen에 첨부하지 않고 텍스트로만 질감을 설명했다. 사용자가 화풍·도장·형상 모두를 지적해 외형을 다시 작업했다.

| 단계 | 파일 | SHA-256 |
|---|---|---|
| 첫 시안 원본 | [MTD-source.png](MTD-source.png) | `c10492a864f5bee37ba8b8d347312c7ef4f8b65ab4a4a9fcbf2f9c4f07202b58` |
| 수정 시안 원본 | [MTD-revised.png](MTD-revised.png) | `34d1b6f71a82d0be1915510dc72f723c689bfad4d93900442eae187fcac373eb` |
| 회녹색 수정 시안의 전투 소비본 | `9e12afc`의 `Resources/Images/EnemyAirStrikeMTD.png` | `7effc765c23adfb7e1b7454f5543dc05e443f448a2ede5db33a8925df048b35a` |
| 검은색·금색 수정 원본 | [MTD-black-gold.png](MTD-black-gold.png) | `04dfd9eb8df7d63b67feef17340ed74f13b44d511f9ee4f66ffdaa97f5f10107` |
| 검은색·금색 수정 시안의 전투 소비본 | `8d54723`의 `Resources/Images/EnemyAirStrikeMTD.png` | `fe0c215275cee8197d2e2453526b82a2351b2977fb2a32d48f1d11bac66d3c37` |
| 사용자 승인 09번 | [MTD-09-approved.png](MTD-09-approved.png) | `b530f83a5845b24c4c7c2ed97e0ddc65fdf41d754e33607ae0a40b1533ec53f5` |
| v0.8 상판 도장 원본과 게임 PNG | `v0.8`의 `MTD-upper-surface.svg` 및 `Resources/Images/EnemyAirStrikeMTD.png` | `4442d1501698dbe49f20c1ba945b5824bb0d40ffbdb8b6ec4a36cdd73e9e47d0` · `95662bc5d5cd8c262fb332300e030deaabb3381c4bb28c547d68216158f6bf1f` |
| v0.8.1 조종석 수정 원본과 게임 PNG | `v0.8.1`의 `MTD-upper-surface.svg` 및 `Resources/Images/EnemyAirStrikeMTD.png` | `79315ffbafec0579eb613a6c67af36081f6cff81ab462ae4c8ed495d0f171071` · `21171d69727ec0d518626cbe0b1c40666035731f7cc4dc39cddc1e0762902e88` |
| 사용자 채택 C안 | [MTD-C-approved.svg](MTD-C-approved.svg) | `361f363719678621440f8521104fac9d6d8136d22c2bbd4c20b93499cbcf16aa` |
| C안 기수 수정 원본 | [MTD-upper-surface.svg](MTD-upper-surface.svg) | `5957cdeb5d8a9f50a38e095e85aace04c301cfbf1f16c697f535d1e426091fb0` |
| C안 기수 수정 게임 PNG | [EnemyAirStrikeMTD.png](../../Resources/Images/EnemyAirStrikeMTD.png) | `006af798f2ecca544d8fc358306bb1ae4387a3d376f9ef991b0e5dbaae522cbe` |

첫 편집에는 첫 시안과 아래 네 장을 직접 첨부했다. 두 번째 편집에서는 수정 시안·공식 도장 사진·원작 공습기 PNG를 다시 첨부해 표식 위치를 고쳤다. 두 편집 모두 투명 배경을 사용했다.

| 참조 | 사용한 요소 | 첨부 파일 SHA-256 |
|---|---|---|
| [MachDefense Airplane.png](../../Resources/Images/Airplane.png) | 부드러운 면 명암, 낮은 채도, 마모 표현 | `931bd62d2f15c48b4262fad2ce27b5473ceca1bbd5e7f3d3fde742b65d2fc786` |
| [럭스루나에 공식 자료](https://www.bandainamcoent.com/news/ace-combat-8-wings-of-theve-game-overview) | 올리브 회색, 기수의 탁금색 선, 작은 꼬리 표식, 회녹색 유리 | `719a272f77439bce29e45cd775457d2d237c438630154773b27dc2cfc92c9eb5` |
| [NASA STOL/MTD 사진](https://www.nasa.gov/image-article/f-15-stol-mtd-arrives-dfrc/) | 카나드, 가는 복좌 캐노피와 STOL/MTD 사각 노즐 | `00d2018495ceeb71e68be132bb4ed8872e78c161c5c0719f560bc1a7b8c32566` |
| [NASA ACTIVE 상면 사진](https://www.nasa.gov/image-article/f-15b-during-active-project-flight/) | 위에서 보이는 주익과 꼬리의 비율. 후기형 둥근 노즐은 사용하지 않았다. | `f975b5c2dd227060439920298bbfec089fe42aa98dee2a0b06c38d3802ed463a` |

수정 원본은 1774×887 RGBA다. 알파 범위를 확인해 크롭한 뒤 균일하게 축소했다. 256×256 투명 캔버스의 중앙 피벗과 기존 렌더 배율을 사용한다.

```sh
magick Art/EnemyAirStrike/MTD-revised.png -crop 1376x845+179+21 +repage \
  -filter Lanczos -resize 212x212 -background none -gravity center \
  -extent 256x256 -strip PNG32:Resources/Images/EnemyAirStrikeMTD.png
```

[실제 편집 프롬프트](revision-prompts.txt).


## 검은색과 금색 도장 수정

회녹색 수정본을 편집 대상으로 사용했다. 헤비멕1에서 채택한 F-22와 MiG-31 PNG는 검은 동체와 금색 대비의 기준으로, MachDefense의 `Airplane.png`는 무광 표면과 부드러운 명암의 기준으로 첨부했다. 날개 전체를 금색으로 두르지 않고 금색 캐노피와 기수 양옆의 선, 꼬리 쪽 강조점을 사용했다. [실제 프롬프트](black-gold-prompt.txt).

- F-22 SHA-256 `8cf228dafd366849f50ce623901a1106dad35df4ccb451e1de6e346c79f6f082`
- MiG-31 SHA-256 `ef9351c5bb4112c0bc10befa489ae989d3b341e69da29a047c6570d3c468260d`

같은 크롭과 축소 조건을 적용했다. 이전·수정 생성 원본의 알파 128 이상 윤곽을 각 외곽 사각형에 맞춰 비교했을 때 교집합/합집합은 약 98.6%였다. 원본 크기와 큰 윤곽은 유지됐으며 외곽 픽셀까지 동일한 편집은 아니다.

```sh
magick Art/EnemyAirStrike/MTD-black-gold.png -crop 1376x845+179+21 +repage   -filter Lanczos -resize 212x212 -background none -gravity center   -extent 256x256 -strip PNG32:Resources/Images/EnemyAirStrikeMTD.png
```

## 09번 승인과 상판 도장 수정

사용자가 09번의 형상과 도장을 승인하고 상판 질감 수정을 지시했다. 각진 명암과 두꺼운 패널 경계를 걷어내고, 검은 무광 도장 위에 옅은 곡면 반사와 낮은 대비의 이음매를 그렸다. 실기 사진은 표면 명암의 참조로 사용했다. [NASA F-15 상판 사진](https://www.nasa.gov/image-article/f-15b-during-active-project-flight/)

ImageGen 편집 시안은 승인본과 외곽 겹침이 92.30%여서 제외했다. 승인 SVG의 외곽, 금색선, 캐노피, 배기구 경로와 색 정의를 유지하고 안쪽 상판 레이어를 수정했다. 두 게임 PNG 모두 256×256이고 알파 범위는 x 24–231, y 58–196이다. 알파 128 이상 외곽은 99.92% 겹쳤다. [편집 시안에 사용한 프롬프트](upper-surface-prompt.txt)

```sh
cairosvg Art/EnemyAirStrike/MTD-upper-surface.svg -o /tmp/mtd-upper-surface.png
magick /tmp/mtd-upper-surface.png -trim +repage -resize 208x208 \
  -background none -gravity center -extent 256x256 PNG32:Resources/Images/EnemyAirStrikeMTD.png
```

iPhone Air 시뮬레이터에서 적기와 아군 공습기를 같은 전투 프레임에 그리는 테스트가 통과했다. 승인본 화면과 픽셀 차이는 적기 영역에 있었다. 수정 PNG는 빌드 및 테스트 번들의 파일과 SHA-256이 일치했다. [09번 승인 화면](../../docs/evidence/enemy-airstrike-20261127/ios-simulator-mtd-09-approved.png) · [상판 수정 화면](../../docs/evidence/enemy-airstrike-20261127/ios-simulator-mtd-painted.png)


## v0.8.1 조종석과 기체 표시 크기

기수가 오른쪽을 향한 원본 SVG에서 조종석 벡터만 중심 `x=293`을 기준으로 길이 방향에 0.5배를 적용했다. 게임 PNG의 조종석 중앙에서 금색 유리가 차지하는 가로 범위는 44픽셀에서 22픽셀이 됐다. 변환한 PNG는 이전과 같은 256×256 캔버스와 알파 윤곽을 사용한다.

공용 전투 렌더에서 적기의 표시 폭과 높이를 각각 0.5배로 그린다. 기체를 회전시키는 중심, 아군 공습의 배율, 빨간 조준점은 기존 값을 사용한다. [전투 렌더](../../Sources/Core/Presentation/WorldPresentation.cpp) · [전투 크기 검사](../../Tests/WorldSpecialTests.cpp).

## C안의 검은 도장과 짧은 기수

사용자가 검은 도장 시안 C를 채택했다. 조종석 바로 앞 `x=316`을 기준으로 그 앞의 기수만 길이 방향으로 0.5배 줄였다. SVG를 세 배 크기로 그렸을 때 기준점부터 기수 끝까지 261픽셀에서 131픽셀로 줄었다. 256×256 게임 PNG는 같은 좌표에 그리며, 금색 유리를 포함한 기수 뒤쪽의 RGBA 픽셀은 채택 C안과 같다.

두 SVG는 C안의 원래 크롭 `1192x799+17+9`와 208픽셀 기준 폭으로 고정해 변환한다. C안에는 `v0.8.1` 게임 PNG의 알파 채널을 사용한다. 게임 PNG의 0~185열에는 채택 C안, 186열부터는 짧아진 기수를 그린 결과를 사용한다.

```sh
cairosvg Art/EnemyAirStrike/MTD-C-approved.svg -o /tmp/mtd-c.png
cairosvg Art/EnemyAirStrike/MTD-upper-surface.svg -o /tmp/mtd-short-nose.png
for art in mtd-c mtd-short-nose; do
  magick "/tmp/$art.png" -crop 1192x799+17+9 +repage -resize 208x208 \
    -background none -gravity center -extent 256x256 "PNG32:/tmp/$art-game.png"
done
git show v0.8.1:Resources/Images/EnemyAirStrikeMTD.png > /tmp/mtd-v081.png
magick /tmp/mtd-c-game.png -alpha off \
  \( /tmp/mtd-v081.png -alpha extract \) -compose CopyOpacity -composite PNG32:/tmp/mtd-c-approved.png
magick /tmp/mtd-c-approved.png -crop 186x256+0+0 +repage \
  \( /tmp/mtd-short-nose-game.png -crop 70x256+186+0 +repage \) \
  +append -strip PNG32:Resources/Images/EnemyAirStrikeMTD.png
```

정식 웹 빌드에서 11스테이지의 61.6초와 62.2초를 C안과 비교했다. 바뀐 화면 픽셀은 기수에만 있었다. iPhone Air 시뮬레이터에서도 새 PNG로 적 공습기와 아군 공습기를 같은 전투 화면에 그렸다.
