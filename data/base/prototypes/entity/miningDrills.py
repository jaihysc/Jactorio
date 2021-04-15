import jactorioData as j


def addSprite(spritePath):
    return (j.Sprite()
                .load(spritePath)
                .frames(8)
                .sets(8)
            )


def createDrill(name, icon, spriteN, spriteE, spriteS, spriteW):
    (j.MiningDrill(name)
        .rotatable(True)
        .miningSpeed(1)

        .item(
            j.Item(name + "-item")
                .sprite(
                    j.Sprite()
                        .load(icon)
                )
        )
        .sprite(addSprite(spriteN))
        .spriteE(addSprite(spriteE))
        .spriteS(addSprite(spriteS))
        .spriteW(addSprite(spriteW))

        .tileWidth(3)
        .tileHeight(3)

        # resource output: up, right, down left
        #         <U>
        #     [0] [ ] [ ]
        # <L> [ ] [ ] [ ] <R>
        #     [ ] [ ] [ ]
        #         <D>
        .resourceOutput(j._OutputTile4Way(
            (
                (1, -1),
                (3, 1),
                (1, 3),
                (-1, 1)
            )
        ))
    )


createDrill("electric-mining-drill",
            "base/graphics/icon/electric-mining-drill.png",
            "base/graphics/entity/electric-mining-drill/electric-mining-drill-N.png",
            "base/graphics/entity/electric-mining-drill/electric-mining-drill-E.png",
            "base/graphics/entity/electric-mining-drill/electric-mining-drill-S.png",
            "base/graphics/entity/electric-mining-drill/electric-mining-drill-W.png")
