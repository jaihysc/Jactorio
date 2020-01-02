import jactorioData as j
c = j.category

def addSprite(name, spritePath):
    return j.add(c.Sprite, name).load(spritePath).group(j.spriteGroup.Terrain)

def addTile(name, spritePath):
    return j.add(c.Tile, name).sprite(addSprite(name, spritePath))

addSprite("menu-background", "core/graphics/menu/background-image.png")
addSprite("menu-logo", "core/graphics/menu/background-image-logo.png")

addTile("cursor-select", "core/graphics/cursor-select.png")
addTile("cursor-invalid", "core/graphics/cursor-invalid.png")


# Inventory selection cursor
j.add(c.Item, "inventory-selected-cursor").sprite(
    j.add(c.Sprite).load("core/graphics/hand.png").group(j.spriteGroup.Gui)
)
