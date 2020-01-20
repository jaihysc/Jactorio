import jactorioData as j
c = j.category

# An item will be created with the name {name}-item | With `wooden-chest`: `wooden-chest-item`
def addContainer(name, invSize, health, worldSprite, itemSprite):
    (j.add(c.ContainerEntity, name)
        .tileWidth(1)
        .tileHeight(1)

        .maxHealth(health)
        .inventorySize(invSize)
        
        .sprite((j.add(c.Sprite)
                .load(worldSprite)
                .group(j.spriteGroup.Terrain)
            )
        )
        .item((j.add(c.Item, name + "-item")
                .sprite((j.add(c.Sprite)
                        .load(itemSprite)
                        .group(j.spriteGroup.Gui)
                    )
                )
            )
        )
    )

addContainer("wooden-chest", 16, 100, "base/graphics/entity/wooden-chest/hr-wooden-chest.png", "base/graphics/entity/wooden-chest/wooden-chest.png")
addContainer("iron-chest", 32, 200,   "base/graphics/entity/iron-chest/hr-iron-chest.png",     "base/graphics/entity/iron-chest/iron-chest.png")
addContainer("steel-chest", 48, 350,  "base/graphics/entity/steel-chest/hr-steel-chest.png",   "base/graphics/entity/steel-chest/steel-chest.png")
