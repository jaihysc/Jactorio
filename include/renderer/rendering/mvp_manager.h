#ifndef RENDERER_RENDERING_MVP_MANAGER_CPP
#define RENDERER_RENDERING_MVP_MANAGER_CPP

#include <glm/glm.hpp>

namespace jactorio::renderer::mvp_manager
{
	// Functions for the Model, View and Projection matrices
	// VIEW

	/**
	 * Modifying the returned pointer will change the location of the camera
	 * @return pointer to camera transform vector
	 */
	glm::vec3* get_view_transform();

	/**
	 * Sets the current view transform
	 */
	void update_view_transform();


	// PROJECTION

	struct Projection_tile_data
	{
		unsigned short tiles_x;
		unsigned short tiles_y;
	};

	/**
	 * Recalculates number of tiles X and Y <br>
	 * Upper left is (0, 0)
	 */
	Projection_tile_data projection_calculate_tile_properties(
		unsigned short tile_width, unsigned short window_width,
		unsigned short window_height);

	/**
	 * Converts Projection_tile_data from projection_calculate_tile_properties into a matrix <br>
	 * Guarantees a zoom of minimum of offset on all axis <br>
	 * May offset more in a certain axis to preserve aspect ratio
	 * @param window_width width of display area in pixels
	 * @param window_height Height of display area in pixels
	 * @param offset number of tiles horizontally to from the edge to hide (giving a zoom effect) <br>
	 * Vertical tiles is calculated based on ration of tiles in tile_data
	 */
	glm::mat4 to_proj_matrix(unsigned short window_width,
	                         unsigned short window_height, float offset);
};

#endif // RENDERER_RENDERING_MVP_MANAGER_CPP
