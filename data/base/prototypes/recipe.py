import jactorioData as j

def addRecipeGroup(name, spritePath):
    return (j.RecipeGroup(name)
        .sprite((j.Sprite()
            .load(spritePath)
        ))
    )

g_logistics = addRecipeGroup("recipegroup-logicsics", "base/graphics/item-group/logistics.png")
g_production = addRecipeGroup("recipegroup-production", "base/graphics/item-group/production.png")
g_intermediate = addRecipeGroup("recipegroup-intermediate", "base/graphics/item-group/intermediate-products.png")
g_combat = addRecipeGroup("recipegroup-combat", "base/graphics/item-group/military.png")

# Be careful using _members for lists, any modifications to them is not known to the c++ side
g_logistics.recipeCategories(([j.RecipeCategory()
    .recipes(([
        j.Recipe()
            .ingredients([("__base__/coal-item", 10), ("__base__/copper-item", 5)])
            .product(("__base__/wooden-chest-item", 2))
            .craftingTime(0.1),
        j.Recipe()
            .ingredients([("__base__/wooden-chest-item", 1), ("__base__/copper-item", 5)])
            .product(("__base__/iron-chest-item", 1))
            .craftingTime(1),
    ]))
]))