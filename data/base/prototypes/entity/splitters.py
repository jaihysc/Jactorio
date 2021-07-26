import jactorioData as j


def addSprite(spritePath):
    return (j.Sprite()
            .load(spritePath)
            .group(j.SpriteGroup.TERRAIN)
            .frames(8)
            .sets(4)
            )


def createSplitter(name, icon, spriteN, spriteE, spriteS, spriteW):
    (j.Splitter(name)
        .rotatable(True)

        .item(j.Item(name + "-item")
            .sprite(
                    j.Sprite()
                    .load(icon)
                    .group(j.SpriteGroup.GUI)
            )
        )
        .sprite(addSprite(spriteN))
        .spriteE(addSprite(spriteE))
        .spriteS(addSprite(spriteS))
        .spriteW(addSprite(spriteW))

        .tileWidth(2)
        .tileHeight(1)

        .speed(0.05)
    )


createSplitter("splitter-basic",
               "base/graphics/icon/splitter.png",
               "base/graphics/entity/splitter/hr-splitter-north.png",
               "base/graphics/entity/splitter/hr-splitter-east.png",
               "base/graphics/entity/splitter/hr-splitter-south.png",
               "base/graphics/entity/splitter/hr-splitter-west.png")
