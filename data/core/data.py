import jactorioData as j

def addSprite(name, spritePath):
    return (j.Sprite(name)
                .load(spritePath)
                .group([j.spriteGroup.Terrain])
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


# Inventory selection cursor
(j.Item("inventory-selected-cursor")
    .sprite((j.Sprite()
        .load("core/graphics/hand.png")
        .group([j.spriteGroup.Gui])
    ))
)
