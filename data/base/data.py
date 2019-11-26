import jactorio_data as jd

def add_proto(name, sprite_path):
    sprite_proto = jd.Sprite(sprite_path)
    jd.add(jd.category.Sprite, name, sprite_proto)

    jd.add(jd.category.Tile, name, jd.Tile(sprite_proto))

add_proto("deep-water-1", "base/graphics/tiles/deep-water.png")
add_proto("water-1", "base/graphics/tiles/water.png")
add_proto("shore-1", "base/graphics/tiles/shore.png")
add_proto("sand-1", "base/graphics/tiles/sand.png")
add_proto("grass-1", "base/graphics/tiles/grass.png")
add_proto("dirt-1", "base/graphics/tiles/dirt.png")
 
 # Resources
 # TODO, the resource generation will be configurable
add_proto("coal", "base/graphics/resource/coal/coal-ore.png")
add_proto("copper", "base/graphics/resource/copper/copper-ore.png")
add_proto("iron", "base/graphics/resource/iron/iron-ore.png")
add_proto("stone", "base/graphics/resource/stone/stone-ore.png")
add_proto("uranium", "base/graphics/resource/uranium/uranium-ore.png")

add_proto("crude-oil", "base/graphics/resource/oil/crude-oil.png")

"""
noise_layer = jd.NoiseLayer()
noise_layer.name = "base-terrain"

proto = jd.Prototype()
proto.load_sprite(sprite_path)
jd.add(jd.category.Tile, "coal", proto)

noise_layer.add_tile(-1, -0.5, proto)

jd.add(jd.category.NoiseLayer, "base-terrain", noise_layer)
"""