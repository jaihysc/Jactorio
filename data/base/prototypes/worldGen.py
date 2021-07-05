import jactorioData as j

def addTile(noiseLayer, endRange, name, spritePath, isWater = False):
    tile = (j.Tile(name)
                .sprite(
                    j.Sprite()
                        .load(spritePath)
                        .group(j.SpriteGroup.TERRAIN)
                )
                .isWater(isWater)
    )
    
    noiseLayer.add(endRange, tile)

# Create NoiseLayer
baseTerrain = (j.NoiseLayerTile("base-terrain")
                .startNoise(-1)
                .normalize(True)
)

addTile(baseTerrain, -0.5, "deep-water-1", "base/graphics/tiles/deep-water.png", True)
addTile(baseTerrain, -0.25, "water-1", "base/graphics/tiles/water.png", True)
addTile(baseTerrain, -0.2, "sand-1", "base/graphics/tiles/sand.png")
addTile(baseTerrain, 0.5, "grass-1", "base/graphics/tiles/grass.png")
addTile(baseTerrain, 1, "dirt-1", "base/graphics/tiles/dirt.png")


##########################################################
# Resources

def addResourceEntity(noiseLayer, endRange, name, tileSpritePath, itemSpritePath):
    # Item for resource
    product = (j.Item(name + "-item")
        .sprite(
            j.Sprite()
                .load(itemSpritePath)
                .group(j.SpriteGroup.GUI)
        )
    )

    resourceTile = (j.ResourceEntity()
                        .sprite(
                            j.Sprite()
                                .load(tileSpritePath)
                                .group(j.SpriteGroup.TERRAIN)
                        )
                        .item(product)
    )
    # Add resource tile to NoiseLayer
    noiseLayer.add(endRange, resourceTile)


# Only use the tips of the noise to ensure that the resources is in one big patch
coalLayer = (j.NoiseLayerEntity("coal-layer")
                .startNoise(0.8)
                .normalize(False)
                .richness(1000)
                .octaveCount(2)
                .frequency(0.3)
                .persistence(0.6)
)
addResourceEntity(coalLayer, 2, "coal", "base/graphics/resource/coal/coal-ore.png", "base/graphics/icon/coal.png")


copperLayer = (j.NoiseLayerEntity("copper-layer")
                .startNoise(1)
                .normalize(False)
                .richness(400)
                .octaveCount(2)
                .frequency(0.5)
                .persistence(0.7)
)
addResourceEntity(copperLayer, 2, "copper", "base/graphics/resource/copper/copper-ore.png", "base/graphics/icon/copper.png")

# addNoiseLayerTile("iron", "base/graphics/resource/iron/iron-ore.png")
# addNoiseLayerTile("stone", "base/graphics/resource/stone/stone-ore.png")
# addNoiseLayerTile("uranium", "base/graphics/resource/uranium/uranium-ore.png")
#
# addNoiseLayerTile("crude-oil", "base/graphics/resource/oil/crude-oil.png")
