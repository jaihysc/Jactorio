import jactorioData as j

def addBelt(name, iconSpritePath, worldSpritePath):
    item = (j.Item(name + "-item")
        .sprite((j.Sprite()
            .load(iconSpritePath)
            .group(j.spriteGroup.Gui)
        ))
    )

    (j.TransportBelt(name)
        .item(item)
        .sprite((j.Sprite()
            .load(worldSpritePath)
            .group(j.spriteGroup.Terrain)
        ))
        .pickupTime(0.1)
    )
addBelt("transport-belt-basic", "base/graphics/icon/transport-belt.png", "base/graphics/entity/transport-belt/transport_belt_temp.png")