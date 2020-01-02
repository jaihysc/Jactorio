import jactorioData as j
c = j.category

def addProto(name, sprite_path):
    j.add(c.Sprite, name).load(sprite_path).group(j.spriteGroup.Terrain)

addProto("test_tile", "test/graphics/test/test_tile.png")
addProto("test_tile1", "test/graphics/test/test_tile1.png")
addProto("test_tile2", "test/graphics/test/test_tile2.png")
addProto("test_tile3", "test/graphics/test/test_tile3.png")
