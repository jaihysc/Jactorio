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

# For debug purposes
addSprite("rect-up", "core/graphics/rect-up.png")
addSprite("rect-right", "core/graphics/rect-right.png")
addSprite("rect-down", "core/graphics/rect-down.png")
addSprite("rect-left", "core/graphics/rect-left.png")
addSprite("rect-stop", "core/graphics/rect-stop.png")

# Inventory selection cursor
(j.Item("inventory-selected-cursor")
    .sprite((j.Sprite()
        .load("core/graphics/hand.png")
        .group([j.spriteGroup.Gui])
    ))
)
