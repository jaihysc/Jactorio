import jactorioData as j


def addInserter(name, itemSprite,
                worldSprite, handSprite, armSprite,
                rotationSpeed, tileReach):
    (j.Inserter(name)
        .rotationSpeed(rotationSpeed)

        .rotatable(True)

        .tileReach(tileReach)

        .sprite(
            j.Sprite()
                .load(worldSprite)
                .group(j.SpriteGroup.TERRAIN)
                .sets(1)
                .frames(4)
                .strategy(j.FrameGenStrategy.TOP_SET)
        )
        .handSprite(
            j.Sprite()
                .load(handSprite)
                .group(j.SpriteGroup.TERRAIN)
        )
        .armSprite(
            j.Sprite()
                .load(armSprite)
                .group(j.SpriteGroup.TERRAIN)
        )

        .item(
            j.Item(name + "-item")
                .sprite(
                    j.Sprite()
                        .load(itemSprite)
                        .group(j.SpriteGroup.GUI)
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
