# 적 공습기 F-15 STOL/MTD

[아트 조사와 제작안](../../docs/research/enemy-airstrike-art-20261127.md)에 형상·도장·질감 참조를 기록했다. 새 탑뷰 그림에는 STOL/MTD의 카나드, 올리브 회색 도장, 탁금색 선, 호박색 캐노피 반사를 적용했다. 형태는 [NASA STOL/MTD](https://www.nasa.gov/image-article/f-15-stol-mtd-arrives-dfrc/) 자료를 참고했다.

| 단계 | 파일 | SHA-256 |
|---|---|---|
| ImageGen 생성 원본 | [MTD-source.png](MTD-source.png) | `c10492a864f5bee37ba8b8d347312c7ef4f8b65ab4a4a9fcbf2f9c4f07202b58` |
| 전투 소비 | [EnemyAirStrikeMTD.png](../../Resources/Images/EnemyAirStrikeMTD.png) | `acc3e3fe6b529ba1c8702d98e0f165e38f0d847a060779e3bc49dfc2115c449b` |

생성 원본은 1536×1024 RGBA다. 알파 외곽을 확인하고 X 77부터 1403px 폭을 사용했다. 균일 축소해 폭 212px로 맞춘 뒤 256×256 투명 캔버스 중앙에 배치했다. 게임은 아군 공습기와 같은 스프라이트 배율·피벗·필터를 사용한다.

```sh
magick Art/EnemyAirStrike/MTD-source.png -crop 1403x1024+77+0 +repage \
  -filter Lanczos -resize 212x212 -background none -gravity center \
  -extent 256x256 -strip PNG32:Resources/Images/EnemyAirStrikeMTD.png
```
