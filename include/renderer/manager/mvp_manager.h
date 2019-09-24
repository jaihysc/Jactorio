#ifndef MVP_MANAGER_CPP
#define MVP_MANAGER_CPP

#include <glm/glm.hpp>

namespace jactorio_renderer_mvp
{
	// VIEW
	
	/*!
	 * Modifying the returned pointer will change the location of the camera
	 * @return pointer to camera transform vector
	 */
	glm::vec3* get_camera_transform();

	/*!
	 * Sends the current camera transform to the GPU
	 * Uniform location for must be initialized first
	 */
	void update_camera_transform();

	// PROJECTION

	unsigned int get_max_tile_count_x();
	unsigned int get_max_tile_count_y();

	void set_proj_calculation_tile_width(unsigned int width);

	glm::mat4 get_proj_matrix();

	/*!_count
	 * Recalculates number of tiles X and Y
	 * based on width set by set_tile_width()
	 * Upper left is 0, 0
	 */
	void calculate_tile_properties();
}

#endif // MVP_MANAGER_CPP
