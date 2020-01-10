import jactorioData as j
c = j.category

def addContainer(name, invSize, health, worldSprite, itemSprite):
    container = j.add(c.ContainerEntity, name).tileWidth(1).tileHeight(1)
    container.maxHealth(health).inventorySize(invSize)

    container.sprite(
        j.add(c.Sprite).load(worldSprite).group(j.spriteGroup.Terrain))
    container.item(
        j.add(c.Item).sprite(
            j.add(c.Sprite).load(itemSprite).group(j.spriteGroup.Gui)))

addContainer("iron-chest", 32, 200, "base/graphics/entity/iron-chest/hr-iron-chest.png", "base/graphics/entity/iron-chest/iron-chest.png")

# Testing only, a big multi tile sprite
container = j.add(c.ContainerEntity, "big-chest").tileWidth(3).tileHeight(8)
container._maxHealth = 200
container._inventorySize = 32

container._sprite = j.add(c.Sprite).load("base/graphics/entity/iron-chest/hr-iron-chest.png").group(j.spriteGroup.Terrain)
container.item(j.add(c.Item).sprite(
    j.add(c.Sprite).load("base/graphics/entity/iron-chest/hr-iron-chest.png").group(j.spriteGroup.Gui))
)