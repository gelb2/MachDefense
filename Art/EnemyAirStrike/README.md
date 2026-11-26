# 적 공습기 F-15 STOL/MTD

[아트 조사와 제작안](../../docs/research/enemy-airstrike-art-20261127.md)에 참조와 전투 크기를 기록했다. 첫 시안은 원작 공습기 PNG를 ImageGen에 첨부하지 않고 텍스트로만 질감을 설명했다. 사용자가 화풍·도장·형상 모두를 지적해 외형을 다시 작업했다.

| 단계 | 파일 | SHA-256 |
|---|---|---|
| 첫 시안 원본 | [MTD-source.png](MTD-source.png) | `c10492a864f5bee37ba8b8d347312c7ef4f8b65ab4a4a9fcbf2f9c4f07202b58` |
| 수정 시안 원본 | [MTD-revised.png](MTD-revised.png) | `34d1b6f71a82d0be1915510dc72f723c689bfad4d93900442eae187fcac373eb` |
| 전투 소비 | [EnemyAirStrikeMTD.png](../../Resources/Images/EnemyAirStrikeMTD.png) | `7effc765c23adfb7e1b7454f5543dc05e443f448a2ede5db33a8925df048b35a` |

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
