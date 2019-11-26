import jactorio_data as jd

def add_proto(name, sprite_path):
    proto = jd.Sprite(sprite_path)
    jd.add(jd.category.Sprite, name, proto)

add_proto("menu-background", "core/graphics/menu/background-image.png")
add_proto("menu-logo", "core/graphics/menu/background-image-logo.png")
 