import jactorioData as j

# An item will be created with the name {name}-item | With `wooden-chest`: `wooden-chest-item`


def addContainer(name, invSize, health, worldSprite, itemSprite):
    (j.ContainerEntity(name)
        .tileWidth(1)
        .tileHeight(1)

        .maxHealth(health)
        .inventorySize(invSize)

        .sprite(
            j.Sprite()
                .load(worldSprite)
                .group(j.SpriteGroup.ENTITY)
        )
        .item(
            j.Item(name + "-item")
                .sprite(
                    j.Sprite()
                        .load(itemSprite)
                        .group(j.SpriteGroup.GUI)
                )
        )
     )


addContainer("wooden-chest", 16, 100,
             "base/graphics/entity/wooden-chest/hr-wooden-chest.png",
             "base/graphics/entity/wooden-chest/wooden-chest.png")
addContainer("iron-chest", 32, 200,
             "base/graphics/entity/iron-chest/hr-iron-chest.png",
             "base/graphics/entity/iron-chest/iron-chest.png")
addContainer("steel-chest", 48, 350,
             "base/graphics/entity/steel-chest/hr-steel-chest.png",
             "base/graphics/entity/steel-chest/steel-chest.png")
