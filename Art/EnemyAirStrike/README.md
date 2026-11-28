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
| 채택 C안 래스터 | [MTD-C-raster.png](MTD-C-raster.png) | `8b7fdcfbc43ad6d2129710f8bff495a466fe3f1820c5e60c7cd9a2ec1bdaa288` |
| CgBI 준비본 | [MTD-C-source-cgbi.png](MTD-C-source-cgbi.png) | `e108e4c8d85f048b86aeeda4241a2476c61d997145cddcd91b3436be83f15a4f` |
| 전투용 RGBA | [EnemyAirStrikeMTD.png](../../Resources/Images/EnemyAirStrikeMTD.png) | `0b4f4064283c40d299c77689913534c2312668bd7fa6f1bfdbdf617c56740581` |

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
git show v0.8:Art/EnemyAirStrike/MTD-upper-surface.svg > /tmp/mtd-upper-surface.svg
cairosvg /tmp/mtd-upper-surface.svg -o /tmp/mtd-upper-surface.png
magick /tmp/mtd-upper-surface.png -trim +repage -resize 208x208 \
  -background none -gravity center -extent 256x256 PNG32:Resources/Images/EnemyAirStrikeMTD.png
```

iPhone Air 시뮬레이터에서 적기와 아군 공습기를 같은 전투 프레임에 그리는 테스트가 통과했다. 승인본 화면과 픽셀 차이는 적기 영역에 있었다. 수정 PNG는 빌드 및 테스트 번들의 파일과 SHA-256이 일치했다. [09번 승인 화면](../../docs/evidence/enemy-airstrike-20261127/ios-simulator-mtd-09-approved.png) · [상판 수정 화면](../../docs/evidence/enemy-airstrike-20261127/ios-simulator-mtd-painted.png)


## v0.8.1 조종석과 기체 표시 크기

기수가 오른쪽을 향한 원본 SVG에서 조종석 벡터만 중심 `x=293`을 기준으로 길이 방향에 0.5배를 적용했다. 게임 PNG의 조종석 중앙에서 금색 유리가 차지하는 가로 범위는 44픽셀에서 22픽셀이 됐다. 변환한 PNG는 이전과 같은 256×256 캔버스와 알파 윤곽을 사용한다.

공용 전투 렌더에서 적기의 표시 폭과 높이를 각각 0.5배로 그린다. 기체를 회전시키는 중심, 아군 공습의 배율, 빨간 조준점은 기존 값을 사용한다. [전투 렌더](../../Sources/Core/Presentation/WorldPresentation.cpp) · [전투 크기 검사](../../Tests/WorldSpecialTests.cpp).

## C안의 검은 도장과 짧은 기수

기수 축소 작업 당시 256×256 래스터에서 채택 C안의 조종석 바로 앞을 기준으로 기수를 절반으로 줄였다. 승인한 윤곽과 금줄을 래스터에 맞췄다. 래스터의 보이는 픽셀은 수정 승인본과 일치한다.

Apple `pngcrush -iphone`으로 래스터를 사전 곱셈 BGRA로 준비했다. [원작 아군 공습기 CgBI](../../Resources/OriginalImages/Airplane.png)와 같은 헤더·색좌표·픽셀 밀도·단일 IDAT 구조로 저장했다. [기존 준비 도구](../../tools/prepare_resources.py)의 PNG 디코딩, 알파 역산, RGBA 인코딩을 같은 순서로 적용했다. 투명도와 불투명 색, 금색 유리 픽셀은 준비 전후 동일하다. 반투명 경계에서 사전 곱셈 채널 값의 차이는 최대 1 미만이다.

전투에서는 아군기와 적기 모두 기존의 `AirStrike::appendSprite()`로 한 장을 그린다. 적기는 앞서 정한 절반 배율을 사용한다. [공습 스프라이트](../../Sources/Core/Simulation/SpecialAttack.cpp) · [전투 레이어](../../Sources/Core/Presentation/WorldPresentation.cpp)

iPhone Air와 Android 에뮬레이터, 오프라인 Web/PWA, Tauri의 치트 검사에서 최종 파일을 확인했다. iPhone에서는 아군기와 적기가 같은 전투 화면에 표시됐다. 다른 게임 자원과 공용 로드·렌더 파일은 기준 브랜치와 일치한다.

## v0.8.2 C안 적용

C안에서 기수를 줄이기 전 길이로 복구했다. 금색 유리와 256×256 캔버스, 공용 렌더 배율은 유지했다. 래스터에서 CgBI를 만든 뒤 아군기와 같은 규칙으로 전투용 RGBA를 준비했다.

## Su-33

사용자가 확인한 `su33-lux-lunae-concept.png`를 사용했다. 허브 미디어에 보존한 1,254×1,254 승인 원본의 SHA-256은 `eef737ab8e93357a11fffe6997500ae5ce4406adfb6e808a91dec7564ef5fefc`다. 그림을 다시 생성하거나 색을 바꾸지 않았다.

```sh
magick "$SU33_APPROVED_PNG" -crop 1084x736+86+259 +repage -filter Lanczos \
  -resize 210x210 -background none -gravity center -extent 256x256 \
  -strip PNG32:Art/EnemyAirStrike/Su33-raster.png
```

F-15와 같이 Apple `pngcrush -iphone -f 0`으로 사전 곱셈 BGRA를 만들었다. 원작 아군기의 색좌표와 픽셀 밀도 청크를 유지하고 IDAT를 한 개로 합쳤다. [기존 준비 도구](../../tools/prepare_resources.py)의 알파 역산과 RGBA 인코딩으로 전투용 PNG를 만들었다. 준비 전후 알파와 불투명 RGB는 일치한다. 반투명 픽셀의 사전 곱셈 RGB 반올림 오차는 최대 0.498이다.

| 자원 | SHA-256 |
|---|---|
| [전투 크기에 맞춘 래스터](Su33-raster.png) | `2c765e9f14eec11567b812c8bbbf2adb08db125ab2ffd370ad3f0288ec483f26` |
| [Apple CgBI 준비본](Su33-source-cgbi.png) | `cd78646015caf0d87cb5b4e5bd8b621099f2ee8438d858d18db85cc0ce76203d` |
| [전투용 RGBA](../../Resources/Images/EnemyAirStrikeSu33.png) | `ebf9ca07f5aca5e37471d52338bdab90e3240b401b1f8614eebf6c767f771dbd` |

전투용 이미지의 256×256 캔버스와 중앙 피벗 (128, 128)은 F-15와 같다. 알파 128 이상 외곽은 Su-33 208×141px, F-15 208×139px다. 우향 이미지의 날개 발사점은 왼쪽 (112, 90), 오른쪽 (112, 166)이며 캔버스의 y 좌표는 아래로 증가한다.
