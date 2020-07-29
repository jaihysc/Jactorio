import jactorioData as j


def addInserter(name, itemSprite,
                worldSprite, handSprite, armSprite,
                rotationSpeed, tileReach):
    (j.Inserter(name)
        .pickupTime(0.1)
        .rotationSpeed(rotationSpeed)

        .rotatable(True)

        .tileReach(tileReach)

        .sprite(
            j.Sprite()
                .load(worldSprite)
                .sets(4)
                .frames(1)
                .invertSetFrame(True)
        )
        .handSprite(
            j.Sprite()
                .load(handSprite)
        )
        .armSprite(
            j.Sprite()
                .load(armSprite)
        )

        .item(
            j.Item(name + "-item")
                .sprite(
                    j.Sprite()
                        .load(itemSprite)
                )
        )
    )


addInserter("basic-inserter",
            "base/graphics/icon/inserter.png",
            worldSprite="base/graphics/entity/inserter/inserter-platform.png",
            handSprite="base/graphics/entity/inserter/inserter-hand-open.png",
            armSprite="base/graphics/entity/inserter/inserter-hand-base.png",
            rotationSpeed=14.4, tileReach=1)

addInserter("fast-inserter",
            "base/graphics/icon/fast-inserter.png",
            worldSprite="base/graphics/entity/fast-inserter/fast-inserter-platform.png",
            handSprite="base/graphics/entity/fast-inserter/fast-inserter-hand-open.png",
            armSprite="base/graphics/entity/fast-inserter/fast-inserter-hand-base.png",
            rotationSpeed=20.8, tileReach=1)

addInserter("long-handed-inserter",
            "base/graphics/icon/long-handed-inserter.png",
            worldSprite="base/graphics/entity/long-handed-inserter/long-handed-inserter-platform.png",
            handSprite="base/graphics/entity/long-handed-inserter/long-handed-inserter-hand-open.png",
            armSprite="base/graphics/entity/long-handed-inserter/long-handed-inserter-hand-base.png",
            rotationSpeed=18.8, tileReach=2)
