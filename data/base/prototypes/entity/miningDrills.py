import jactorioData as j


def addSprite(spritePath):
    return (j.Sprite()
                .load(spritePath)
                .group([j.spriteGroup.Terrain])
                .frames(8)
                .sets(8)
    )

def createDrill(name, icon, spriteN, spriteE, spriteS, spriteW):
    (j.MiningDrill(name)
        .rotatable(True)
        .pickupTime(0.1)
        
        .item((j.Item(name + "-item")
                    .sprite((j.Sprite()
                            .load(icon)
                            .group([j.spriteGroup.Terrain, j.spriteGroup.Gui])
                            ))
                ))
        .sprite(addSprite(spriteN))
        .spriteE(addSprite(spriteE))
        .spriteS(addSprite(spriteS))
        .spriteW(addSprite(spriteW))

        .tileWidth(3)
        .tileHeight(3)
    )

(createDrill("electric-mining-drill", 
    "base/graphics/icon/electric-mining-drill.png",
    "base/graphics/entity/electric-mining-drill/electric-mining-drill-N.png",
    "base/graphics/entity/electric-mining-drill/electric-mining-drill-E.png",
    "base/graphics/entity/electric-mining-drill/electric-mining-drill-S.png",
    "base/graphics/entity/electric-mining-drill/electric-mining-drill-W.png"
    )
)