import jactorioData as j
c = j.category
import data.base.prototypes.entity.containers

def addTile(noiseLayer, endRange, name, spritePath, isWater = False):
    sprite = j.add(c.Sprite).load(spritePath).group(j.spriteGroup.Terrain)
    tile = j.add(c.Tile).sprite(sprite).isWater(isWater)
    
    noiseLayer.addTile(endRange, tile)

# Create NoiseLayer
baseTerrain = j.add(c.NoiseLayer, "base-terrain").startVal(-1).normalize(True)

addTile(baseTerrain, -0.5, "deep-water-1", "base/graphics/tiles/deep-water.png", True)
addTile(baseTerrain, -0.25, "water-1", "base/graphics/tiles/water.png", True)
addTile(baseTerrain, -0.2, "sand-1", "base/graphics/tiles/sand.png")
addTile(baseTerrain, 0.5, "grass-1", "base/graphics/tiles/grass.png")
addTile(baseTerrain, 1, "dirt-1", "base/graphics/tiles/dirt.png")


##########################################################
# Resources

def addResourceTile(
    noiseLayer, endRange, name, tileSpritePath, itemSpritePath):
    # Item for resource
    j.add(c.Item, name + "-item").sprite(
        j.add(c.Sprite).load(itemSpritePath).group(j.spriteGroup.Gui)
    )

    # Add resource tile to NoiseLayer
    noiseLayer.addTile(endRange, j.add(c.ResourceTile).sprite(
            j.add(c.Sprite).load(tileSpritePath).group(j.spriteGroup.Terrain)
        )
    )

# Only use the tips of the noise to ensure that the resources is in one big patch
coalLayer = j.add(c.NoiseLayer, "coal-layer").startVal(0.8).normalize(False)

coalLayer._tileDataCategory = j.category.ResourceTile
coalLayer._octaveCount = 2
coalLayer._frequency = 0.3
coalLayer._persistence = 0.6

addResourceTile(coalLayer, 2, "coal", "base/graphics/resource/coal/coal-ore.png", "base/graphics/icon/coal.png")

copperLayer = j.add(c.NoiseLayer, "copper-layer").startVal(1).normalize(False)

copperLayer._tileDataCategory = j.category.ResourceTile
copperLayer._octaveCount = 2
copperLayer._frequency = 0.5
copperLayer._persistence = 0.7

addResourceTile(copperLayer, 2, "copper", "base/graphics/resource/copper/copper-ore.png", "base/graphics/icon/copper.png")

# addNoiseLayerTile("iron", "base/graphics/resource/iron/iron-ore.png")
# addNoiseLayerTile("stone", "base/graphics/resource/stone/stone-ore.png")
# addNoiseLayerTile("uranium", "base/graphics/resource/uranium/uranium-ore.png")
#
# addNoiseLayerTile("crude-oil", "base/graphics/resource/oil/crude-oil.png")
