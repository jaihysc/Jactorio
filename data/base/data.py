import jactorioData as j
c = j.category
import data.base.prototypes.entity.containers

def addTile(noiseLayer, endRange, name, spritePath, isWater = False):
    # Sprite
    sprite = j.add(c.Sprite)
    sprite.load(spritePath)
    sprite.group = j.spriteGroup.Terrain

    # Tile
    tile = j.add(c.Tile)
    tile.sprite = sprite
    tile.isWater = isWater
    
    noiseLayer.addTile(endRange, tile)

# Create NoiseLayer
baseTerrain = j.add(c.NoiseLayer, "base-terrain")
baseTerrain.startVal(-1)
baseTerrain.normalize = True

addTile(baseTerrain, -0.5, "deep-water-1", "base/graphics/tiles/deep-water.png", True)
addTile(baseTerrain, -0.25, "water-1", "base/graphics/tiles/water.png", True)
addTile(baseTerrain, -0.2, "sand-1", "base/graphics/tiles/sand.png")
addTile(baseTerrain, 0.5, "grass-1", "base/graphics/tiles/grass.png")
addTile(baseTerrain, 1, "dirt-1", "base/graphics/tiles/dirt.png")


##########################################################
# Resources

def addResourceTile(
    noiseLayer, endRange, name, tileSpritePath, itemSpritePath):
    # Item sprite
    item = j.add(c.Item, name + "-item")
    itemSprite = j.add(c.Sprite)
    itemSprite.load(itemSpritePath)
    itemSprite.group = j.spriteGroup.Gui
    item.sprite = itemSprite

    # Add Tile to NoiseLayer
    resourceTile = j.add(c.ResourceTile)
    resourceTile.sprite = j.add(c.Sprite)
    resourceTile.sprite.load(tileSpritePath)
    resourceTile.sprite.group = j.spriteGroup.Terrain

    noiseLayer.addTile(endRange, resourceTile)

# Only use the tips of the noise to ensure that the resources is in one big patch
coalLayer = j.add(c.NoiseLayer, "coal-layer")
coalLayer.startVal(0.8)
coalLayer.normalize = False

coalLayer.tileDataCategory = j.category.ResourceTile
coalLayer.octaveCount = 2
coalLayer.frequency = 0.3
coalLayer.persistence = 0.6

addResourceTile(coalLayer, 2, "coal", "base/graphics/resource/coal/coal-ore.png", "base/graphics/icon/coal.png")

copperLayer = j.add(c.NoiseLayer, "copper-layer")
copperLayer.startVal(1)
copperLayer.normalize = False

copperLayer.tileDataCategory = j.category.ResourceTile
copperLayer.octaveCount = 2
copperLayer.frequency = 0.5
copperLayer.persistence = 0.7

addResourceTile(copperLayer, 2, "copper", "base/graphics/resource/copper/copper-ore.png", "base/graphics/icon/copper.png")

# addNoiseLayerTile("iron", "base/graphics/resource/iron/iron-ore.png")
# addNoiseLayerTile("stone", "base/graphics/resource/stone/stone-ore.png")
# addNoiseLayerTile("uranium", "base/graphics/resource/uranium/uranium-ore.png")
#
# addNoiseLayerTile("crude-oil", "base/graphics/resource/oil/crude-oil.png")
