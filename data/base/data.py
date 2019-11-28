import jactorio_data as jd

def addNoiseLayerTile(noiseLayer, endRange, name, spritePath):
    # Create Sprite
    sprite_proto = jd.Sprite(spritePath)
    jd.add(jd.category.Sprite, name, sprite_proto)

    # Create tile
    tile_proto = jd.Tile(sprite_proto)
    jd.add(jd.category.Tile, name, tile_proto)

    # Add Tile to NoiseLayer
    noiseLayer.addTile(endRange, tile_proto)

# Create NoiseLayer
noise_layer = jd.NoiseLayer(-1)
noise_layer.name = "base-terrain"

addNoiseLayerTile(noise_layer, -0.5, "deep-water-1", "base/graphics/tiles/deep-water.png")
addNoiseLayerTile(noise_layer, -0.25, "water-1", "base/graphics/tiles/water.png")
addNoiseLayerTile(noise_layer, -0.2, "sand-1", "base/graphics/tiles/sand.png")
addNoiseLayerTile(noise_layer, 0.5, "grass-1", "base/graphics/tiles/grass.png")
addNoiseLayerTile(noise_layer, 1, "dirt-1", "base/graphics/tiles/dirt.png")

# Add NoiseLayer
jd.add(jd.category.NoiseLayer, "base-terrain", noise_layer)


#  # Resources
#  # TODO, the resource generation will be configurable
# addNoiseLayerTile("coal", "base/graphics/resource/coal/coal-ore.png")
# addNoiseLayerTile("copper", "base/graphics/resource/copper/copper-ore.png")
# addNoiseLayerTile("iron", "base/graphics/resource/iron/iron-ore.png")
# addNoiseLayerTile("stone", "base/graphics/resource/stone/stone-ore.png")
# addNoiseLayerTile("uranium", "base/graphics/resource/uranium/uranium-ore.png")
#
# addNoiseLayerTile("crude-oil", "base/graphics/resource/oil/crude-oil.png")
