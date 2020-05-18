import jactorioData as j


def addInserter(name, worldSprite, itemSprite):
    (j.Inserter(name)
        .pickupTime(0.1)
        .rotationSpeed(0.4)

        .sprite(j.Sprite().load(worldSprite))

        .item((j.Item(name + "-item")
               .sprite((j.Sprite()
                        .load(itemSprite)
                        ))
               ))
     )


addInserter("basic-inserter", 
            "base/graphics/entity/inserter/inserter-temp.png",
            "base/graphics/icon/inserter.png")
