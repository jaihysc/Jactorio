import jactorioData as j

def addAssemblyMachine(name, iconSpritePath, worldSpritePath):
    (j.AssemblyMachine(name)
        .item(
            j.Item(name + "-item")
                .sprite(
                    j.Sprite()
                        .load(iconSpritePath)
                        .group(j.SpriteGroup.GUI)
                )
        )
        .sprite(
            j.Sprite()
                .load(worldSpritePath)
                .group(j.SpriteGroup.ENTITY)
                .frames(8)
                .sets(4)
                .trim(2)
        )
        .tileWidth(3)
        .tileHeight(3)
        
        .assemblySpeed(1)
        .rotatable(True)
    )


addAssemblyMachine("assembly-machine-1",
                    "base/graphics/icon/assembling-machine-1.png",
                    "base/graphics/entity/assembling-machine-1/assembling-machine-1.png")

addAssemblyMachine("assembly-machine-2",
                    "base/graphics/icon/assembling-machine-2.png",
                    "base/graphics/entity/assembling-machine-2/assembling-machine-2.png")

addAssemblyMachine("assembly-machine-3",
                    "base/graphics/icon/assembling-machine-3.png",
                    "base/graphics/entity/assembling-machine-3/assembling-machine-3.png")