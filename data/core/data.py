import core.prototypes.local

import jactorioData as j

def addSprite(name, spritePath):
    return (j.Sprite(name)
                .load(spritePath)
                .group(j.SpriteGroup.TERRAIN)
    )

#addSprite("menu-background", "core/graphics/menu/background-image.png")
#addSprite("menu-logo", "core/graphics/menu/background-image-logo.png")

addSprite("cursor-select", "core/graphics/cursor-select.png")
addSprite("cursor-invalid", "core/graphics/cursor-invalid.png")

# For transport line debug purposes
addSprite("arrow-up", "core/graphics/arrow-up.png")
addSprite("arrow-right", "core/graphics/arrow-right.png")
addSprite("arrow-down", "core/graphics/arrow-down.png")
addSprite("arrow-left", "core/graphics/arrow-left.png")

addSprite("rect-green", "core/graphics/rect-green.png")
addSprite("rect-aqua", "core/graphics/rect-aqua.png")
addSprite("rect-red", "core/graphics/rect-red.png")
addSprite("rect-pink", "core/graphics/rect-pink.png")
addSprite("rect-yellow", "core/graphics/rect-yellow.png")
addSprite("rect-grey", "core/graphics/rect-grey.png")


# Hard coded items shown in the inventory
def addItem(name, path):
    (j.Item(name)
        .sprite(j.Sprite()
            .load(path)
            .group(j.SpriteGroup.GUI)
        )
    )

addItem("inventory-selected-cursor", "core/graphics/hand.png")
addItem("reset", "core/graphics/reset.png")
