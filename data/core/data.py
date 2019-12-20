import jactorio_data as jd

def add_sprite(name, sprite_path):
    proto = jd.Sprite(sprite_path)
    jd.add(jd.category.Sprite, name, proto)
    return proto
def add_tile(name, sprite_path):
    sprite_proto = add_sprite(name, sprite_path)
    tile_proto = jd.Tile(sprite_proto)
    jd.add(jd.category.Tile, name, tile_proto)
    return tile_proto


add_sprite("menu-background", "core/graphics/menu/background-image.png")
add_sprite("menu-logo", "core/graphics/menu/background-image-logo.png")
 
add_tile("cursor-select", "core/graphics/cursor-select.png")
add_tile("cursor-invalid", "core/graphics/cursor-invalid.png")
