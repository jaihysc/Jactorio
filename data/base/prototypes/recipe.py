import jactorioData as j
c = j.category

def addRecipeGroup(name, spritePath):
    return (j.add(c.RecipeGroup, name)
        .sprite((j.add(c.Sprite)
            .load(spritePath)
        ))
    )

g_logistics = addRecipeGroup("recipegroup-logicsics", "base/graphics/item-group/logistics.png")
g_production = addRecipeGroup("recipegroup-production", "base/graphics/item-group/production.png")
g_intermediate = addRecipeGroup("recipegroup-intermediate", "base/graphics/item-group/intermediate-products.png")
g_combat = addRecipeGroup("recipegroup-combat", "base/graphics/item-group/military.png")
