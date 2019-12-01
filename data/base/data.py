import jactorio_data as jd

def addNoiseLayerTile(noiseLayer, endRange, name, spritePath, resourceTile, isWater = False):
    # Create Sprite
    sprite_proto = jd.Sprite(spritePath)
    jd.add(jd.category.Sprite, name, sprite_proto)

    # Create tile
    tile_proto = []
    if resourceTile:      
        tile_proto = jd.ResourceTile(sprite_proto)
        jd.add(jd.category.ResourceTile, name, tile_proto)
    else:
        tile_proto = jd.Tile(sprite_proto)
        tile_proto.isWater = isWater
        jd.add(jd.category.Tile, name, tile_proto)

    # Add Tile to NoiseLayer
    noiseLayer.addTile(endRange, tile_proto)

# Create NoiseLayer
baseTerrain = jd.NoiseLayer(-1, True)

addNoiseLayerTile(baseTerrain, -0.5, "deep-water-1", "base/graphics/tiles/deep-water.png", False, True)
addNoiseLayerTile(baseTerrain, -0.25, "water-1", "base/graphics/tiles/water.png", False, True)
addNoiseLayerTile(baseTerrain, -0.2, "sand-1", "base/graphics/tiles/sand.png", False)
addNoiseLayerTile(baseTerrain, 0.5, "grass-1", "base/graphics/tiles/grass.png", False)
addNoiseLayerTile(baseTerrain, 1, "dirt-1", "base/graphics/tiles/dirt.png", False)

# Add NoiseLayer
jd.add(jd.category.NoiseLayer, "base-terrain", baseTerrain)


##########################################################
# Resources

# Only use the tips of the noise to ensure that the resources is in one big patch
coalLayer = jd.NoiseLayer(0.8, False)
coalLayer.tileDataCategory = jd.category.ResourceTile
coalLayer.octaveCount = 2
coalLayer.frequency = 0.3
coalLayer.persistence = 0.6

addNoiseLayerTile(coalLayer, 2, "coal", "base/graphics/resource/coal/coal-ore.png", True)
jd.add(jd.category.NoiseLayer, "coal-layer", coalLayer)

copperLayer = jd.NoiseLayer(1, False)
copperLayer.tileDataCategory = jd.category.ResourceTile
copperLayer.octaveCount = 2
copperLayer.frequency = 0.5
copperLayer.persistence = 0.7

addNoiseLayerTile(copperLayer, 2, "copper", "base/graphics/resource/copper/copper-ore.png", True)
jd.add(jd.category.NoiseLayer, "copper-layer", copperLayer)


# addNoiseLayerTile("copper", "base/graphics/resource/copper/copper-ore.png")
# addNoiseLayerTile("iron", "base/graphics/resource/iron/iron-ore.png")
# addNoiseLayerTile("stone", "base/graphics/resource/stone/stone-ore.png")
# addNoiseLayerTile("uranium", "base/graphics/resource/uranium/uranium-ore.png")
#
# addNoiseLayerTile("crude-oil", "base/graphics/resource/oil/crude-oil.png")
