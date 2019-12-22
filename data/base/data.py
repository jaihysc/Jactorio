import jactorio_data as jd

def addTile(noiseLayer, endRange, name, spritePath, isWater = False):
    # Create Sprite
    sprite_proto = jd.Sprite(spritePath, jd.spriteGroup.Terrain)
    jd.add(jd.category.Sprite, name, sprite_proto)

    # Create tile
    tile_proto = jd.Tile(sprite_proto)
    tile_proto.isWater = isWater
    jd.add(jd.category.Tile, name, tile_proto)

    # Add Tile to NoiseLayer
    noiseLayer.addTile(endRange, tile_proto)

# Create NoiseLayer
baseTerrain = jd.NoiseLayer(-1, True)

addTile(baseTerrain, -0.5, "deep-water-1", "base/graphics/tiles/deep-water.png", True)
addTile(baseTerrain, -0.25, "water-1", "base/graphics/tiles/water.png", True)
addTile(baseTerrain, -0.2, "sand-1", "base/graphics/tiles/sand.png")
addTile(baseTerrain, 0.5, "grass-1", "base/graphics/tiles/grass.png")
addTile(baseTerrain, 1, "dirt-1", "base/graphics/tiles/dirt.png")

# Add NoiseLayer
jd.add(jd.category.NoiseLayer, "base-terrain", baseTerrain)


##########################################################
# Resources

def addResourceTile(
    noiseLayer, endRange, name, tileSpritePath, itemSpritePath):
    # Create Terrain Sprite
    sprite_proto = jd.Sprite(tileSpritePath, jd.spriteGroup.Terrain)
    jd.add(jd.category.Sprite, name, sprite_proto)

    # Item sprite
    item_sprite_proto = jd.Sprite(itemSpritePath)
    item_sprite_proto.group = jd.spriteGroup.Gui
    jd.add(jd.category.Sprite, name + "-item", item_sprite_proto)

    # Create tile
    tile_proto = jd.ResourceTile(sprite_proto)
    jd.add(jd.category.ResourceTile, name, tile_proto)

    # Add Tile to NoiseLayer
    noiseLayer.addTile(endRange, tile_proto)

# Only use the tips of the noise to ensure that the resources is in one big patch
coalLayer = jd.NoiseLayer(0.8, False)
coalLayer.tileDataCategory = jd.category.ResourceTile
coalLayer.octaveCount = 2
coalLayer.frequency = 0.3
coalLayer.persistence = 0.6

addResourceTile(coalLayer, 2, "coal", "base/graphics/resource/coal/coal-ore.png", "base/graphics/icon/coal.png")
jd.add(jd.category.NoiseLayer, "coal-layer", coalLayer)

copperLayer = jd.NoiseLayer(1, False)
copperLayer.tileDataCategory = jd.category.ResourceTile
copperLayer.octaveCount = 2
copperLayer.frequency = 0.5
copperLayer.persistence = 0.7

addResourceTile(copperLayer, 2, "copper", "base/graphics/resource/copper/copper-ore.png", "base/graphics/icon/copper.png")
jd.add(jd.category.NoiseLayer, "copper-layer", copperLayer)


# addNoiseLayerTile("iron", "base/graphics/resource/iron/iron-ore.png")
# addNoiseLayerTile("stone", "base/graphics/resource/stone/stone-ore.png")
# addNoiseLayerTile("uranium", "base/graphics/resource/uranium/uranium-ore.png")
#
# addNoiseLayerTile("crude-oil", "base/graphics/resource/oil/crude-oil.png")
