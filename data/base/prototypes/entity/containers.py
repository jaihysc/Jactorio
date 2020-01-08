import jactorioData as j
c = j.category

container = j.add(c.ContainerEntity, "chest").tileWidth(2).tileHeight(2)
container._maxHealth = 50
container._inventorySize = 14

container._sprite = j.add(c.Sprite).load("base/graphics/entity/iron-chest/hr-iron-chest.png").group(j.spriteGroup.Terrain)
container.item(j.add(c.Item).sprite(
    j.add(c.Sprite).load("base/graphics/entity/iron-chest/iron-chest.png").group(j.spriteGroup.Gui))
)