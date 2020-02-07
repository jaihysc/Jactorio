import jactorioData as j

def addBelt(name, iconSpritePath):
    (j.Item(name + "-item")
        .sprite((j.Sprite()
            .load(iconSpritePath)
            .group(j.spriteGroup.Gui)
        ))
    )

addBelt("transport-belt-basic", "base/graphics/icon/transport-belt.png")