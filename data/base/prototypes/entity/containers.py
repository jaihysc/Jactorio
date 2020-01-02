import jactorioData as j
c = j.category

container = j.add(c.ContainerEntity, "chest")
container.maxHealth = 50
container.inventorySize = 14

sprite = j.add(c.Sprite)
sprite.load("base/graphics/entity/iron-chest/hr-iron-chest.png")
container.sprite = sprite
