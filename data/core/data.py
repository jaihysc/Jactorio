import jactorio_data as jd

def add_sprite(name, sprite_path):
    return jd.add(jd.category.Sprite, name, jd.Sprite(sprite_path, jd.spriteGroup.Terrain))

def add_tile(name, sprite_path):
    return jd.add(jd.category.Tile, name, jd.Tile(add_sprite(name, sprite_path)))


add_sprite("menu-background", "core/graphics/menu/background-image.png")
add_sprite("menu-logo", "core/graphics/menu/background-image-logo.png")
 
add_tile("cursor-select", "core/graphics/cursor-select.png")
add_tile("cursor-invalid", "core/graphics/cursor-invalid.png")
