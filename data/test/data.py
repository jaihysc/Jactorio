import jactorio_data as jd

def add_proto(name, sprite_path):
    proto = jd.Prototype()
    proto.load_sprite(sprite_path)
    # The prototype name is automatically set
    jd.add(jd.category.Tile, name, proto)

add_proto("test_tile", "test/graphics/test/test_tile.png")
add_proto("test_tile1", "test/graphics/test/test_tile1.png")
add_proto("test_tile2", "test/graphics/test/test_tile2.png")
add_proto("test_tile3", "test/graphics/test/test_tile3.png")
