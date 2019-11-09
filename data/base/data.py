import jactorio_data as jd

def add_proto(name, sprite_path):
    proto = jd.Prototype()
    proto.load_sprite(sprite_path)
    # The prototype name is automatically set
    jd.add(jd.category.Tile, name, proto)

add_proto("grass-1", "base/graphics/tiles/grass.png")
 