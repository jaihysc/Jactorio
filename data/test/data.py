import jactorioData as j

import test.subdir.test

def addProto(name, sprite_path):
    (j.Sprite(name)
        .load(sprite_path)
        .group(j.SpriteGroup.TERRAIN)
    )

addProto("test_tile", "test/graphics/test/test_tile.png")
addProto("test_tile1", "test/graphics/test/test_tile1.png")
addProto("test_tile2", "test/graphics/test/test_tile2.png")
addProto("test_tile3", "test/graphics/test/test_tile3.png")

(j.ContainerEntity("test_container")
    .tileWidth(3)
    .tileHeight(2)

    .sprite(
        j.Sprite()
            .load("test/graphics/test/test_tile.png")
            .group(j.SpriteGroup.TERRAIN)
    )
    .item(
        j.Item("test_container-item")
            .sprite(
                j.Sprite()
                    .load("test/graphics/test/test_tile.png")
                    .group(j.SpriteGroup.GUI)
            )
    )
)

(j.Localization()
    .identifier("test-blank-local")
    .fontPath("core/font/NotoSans-Regular.ttf")
    .fontSize(16)
)