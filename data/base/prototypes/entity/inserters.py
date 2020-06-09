import jactorioData as j


def addInserter(name, worldSprite, itemSprite, rotationSpeed, tileReach):
    (j.Inserter(name)
        .pickupTime(0.1)
        .rotationSpeed(rotationSpeed)

        .rotatable(True)

        .tileReach(tileReach)

        .sprite((j.Sprite()
                 .load(worldSprite)
                 .sets(4)
                 .frames(1)
                 .invertSetFrame(True)
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
            14.4,
            1)

addInserter("fast-inserter",
            "base/graphics/entity/fast-inserter/fast-inserter-platform.png",
            "base/graphics/icon/fast-inserter.png",
            20.8,
            1)

addInserter("long-handed-inserter",
            "base/graphics/entity/long-handed-inserter/long-handed-inserter-platform.png",
            "base/graphics/icon/long-handed-inserter.png",
            18.8,
            2)
