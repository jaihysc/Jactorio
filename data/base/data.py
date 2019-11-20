import jactorio_data as jd

def add_proto(name, sprite_path):
    proto = jd.Prototype()
    proto.load_sprite(sprite_path)
    # The prototype name is automatically set
    jd.add(jd.category.Tile, name, proto)

add_proto("deep-water-1", "base/graphics/tiles/deep-water.png")
add_proto("water-1", "base/graphics/tiles/water.png")
add_proto("shore-1", "base/graphics/tiles/shore.png")
add_proto("sand-1", "base/graphics/tiles/sand.png")
add_proto("grass-1", "base/graphics/tiles/grass.png")
add_proto("dirt-1", "base/graphics/tiles/dirt.png")
 