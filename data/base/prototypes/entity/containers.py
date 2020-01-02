import jactorioData as j
c = j.category

container = j.add(c.ContainerEntity, "chest")
container._maxHealth = 50
container._inventorySize = 14

container._sprite = j.add(c.Sprite).load("base/graphics/entity/iron-chest/hr-iron-chest.png")
