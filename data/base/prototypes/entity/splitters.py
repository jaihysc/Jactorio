import jactorioData as j


def addSprite(spritePath):
    return (j.Sprite()
            .load(spritePath)
            .frames(8)
            .sets(4)
            )


def createSplitter(name, icon, spriteN, spriteE, spriteS, spriteW):
    (j.Splitter(name)
        .rotatable(True)
        .pickupTime(0.1)

        .item(j.Item(name + "-item")
            .sprite(
                    j.Sprite()
                    .load(icon)
            )
        )
        .sprite(addSprite(spriteN))
        .spriteE(addSprite(spriteE))
        .spriteS(addSprite(spriteS))
        .spriteW(addSprite(spriteW))

        .tileWidth(2)
        .tileHeight(1)
    )


createSplitter("splitter-basic",
               "base/graphics/icon/splitter.png",
               "base/graphics/entity/splitter/hr-splitter-north.png",
               "base/graphics/entity/splitter/hr-splitter-east.png",
               "base/graphics/entity/splitter/hr-splitter-south.png",
               "base/graphics/entity/splitter/hr-splitter-west.png")
