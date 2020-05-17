import jactorioData as j

def addInserter(name, worldSprite, itemSprite):
    (j.Inserter(name)
        .pickupTime(0.1)
        .rotationSpeed(0.4)

        .sprite((j.Sprite()
                .load(worldSprite)
                .group([j.spriteGroup.Terrain])
            )
        )

        .item((j.Item(name + "-item")
                    .sprite((j.Sprite()
                            .load(itemSprite)
                            .group([j.spriteGroup.Terrain, j.spriteGroup.Gui])
                        )
                    )
                )
            )
    )

addInserter("basic-inserter", "base/graphics/entity/inserter/inserter-temp.png", "base/graphics/icon/inserter.png")