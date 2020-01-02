import jactorioData as j
c = j.category

def addSprite(name, spritePath):
    spriteProto = j.add(c.Sprite, name)
    spriteProto.load(spritePath)
    spriteProto.group = j.spriteGroup.Terrain
    return spriteProto

def addTile(name, spritePath):
    tileProto = j.add(c.Tile, name)
    tileProto.sprite = addSprite(name, spritePath)

addSprite("menu-background", "core/graphics/menu/background-image.png")
addSprite("menu-logo", "core/graphics/menu/background-image-logo.png")

addTile("cursor-select", "core/graphics/cursor-select.png")
addTile("cursor-invalid", "core/graphics/cursor-invalid.png")


# Inventory selection cursor
itemSprite = j.add(c.Sprite)
itemSprite.load("core/graphics/hand.png")
itemSprite.group = j.spriteGroup.Gui

itemCursor = j.add(c.Item, "inventory-selected-cursor")
itemCursor.sprite = itemSprite
