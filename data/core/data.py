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


# Inventory selection cursor
(j.Item("inventory-selected-cursor")
    .sprite((j.Sprite()
        .load("core/graphics/hand.png")
        .group([j.spriteGroup.Gui])
    ))
)
