#include "renderer/rendering/position_renderer.h"

#include "game/world/world_manager.h"

void jactorio::renderer::position_renderer::render_object_layers(Renderer* renderer, float player_x, float player_y) {
	// TODO I do not need to recalculate the centering, the calculations from tile_renderer can be reused

	auto& layer_1 = renderer->render_layer_object;
	auto& layer_2 = renderer->render_layer_object2;

	layer_1.clear();
	
	auto* chunk = game::world_manager::get_chunk(0, 0);
	if (chunk == nullptr)
		return;
	
	auto& thing = chunk->objects[0];
	for (auto& i : thing) {
		layer_1.push_back(Renderer_layer::Element(
			{{100, 100}, {2000, 1000}},
			{{0, 0}, {1, 1}}
		));
	}

	layer_1.g_buffer_bind();
	layer_1.g_update_data(true, true);
	renderer->g_draw();
}
