import jactorioData as j


def addInserter(name, worldSprite, itemSprite, rotationSpeed):
    (j.Inserter(name)
        .pickupTime(0.1)
        .rotatable(True)
        .rotationSpeed(rotationSpeed)

        .sprite((j.Sprite()
                 .load(worldSprite)
                 .frames(4)
                 .sets(1)
                 .trim(2)
                 ))

        .item((j.Item(name + "-item")
               .sprite((j.Sprite()
                        .load(itemSprite)
                        ))
               ))
     )


addInserter("basic-inserter",
            "base/graphics/entity/inserter/inserter-platform.png",
            "base/graphics/icon/inserter.png",
            0.4)

addInserter("fast-inserter",
            "base/graphics/entity/fast-inserter/fast-inserter-platform.png",
            "base/graphics/icon/fast-inserter.png",
            14.4)
