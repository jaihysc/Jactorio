import jactorioData as j


def addBelt(name, iconSpritePath, worldSpritePath):
    (j.TransportBelt(name)
        .item(
            j.Item(name + "-item")
                .sprite(
                    j.Sprite()
                        .load(iconSpritePath)
                        .group(j.SpriteGroup.GUI)
                )
        )
        .sprite(
            j.Sprite()
                .load(worldSpritePath)
                .group(j.SpriteGroup.ENTITY)
                .frames(16)
                .sets(20)
                .trim(16)
        )
        .speed(0.05)
        .rotatable(True)
    )


addBelt("transport-belt-basic",
        "base/graphics/icon/transport-belt.png",
        "base/graphics/entity/transport-belt/transport-belt.png")
