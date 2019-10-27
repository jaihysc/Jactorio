#include "renderer/rendering/renderer_manager.h"

#include <vector>


// Renderer will buffer and draw the same items until the timestamp changes
float timestamp;
std::vector<jactorio::renderer::Render_data> render_data;


void jactorio::renderer::renderer_manager::add_draw_item(unsigned tl_x, unsigned tl_y,
                                                         unsigned int br_x, unsigned int br_y,
                                                         const std::string& sprite_internal_name) {


}

std::vector<jactorio::renderer::Render_data>* jactorio::renderer::renderer_manager::get_render_data() {
	return &render_data;
}
