#ifndef MVP_MANAGER_CPP
#define MVP_MANAGER_CPP

#include <glm/glm.hpp>

namespace jactorio
{
	namespace renderer
	{
		// Functions for the Model, View and Projection matrices
		
		class Mvp_manager
		{

			static glm::vec3 camera_transform_;
			
		public:
			// VIEW
		
			/*!
			 * Modifying the returned pointer will change the location of the camera
			 * @return pointer to camera transform vector
			 */
			static glm::vec3* get_view_transform();

			/*!
			 * Sends the current view transform to the GPU
			 * Uniform location for shader_manager must be initialized first
			 */
			static void update_view_transform();

			
			// PROJECTION

			struct Projection_tile_data
			{
				unsigned short tiles_x;
				unsigned short tiles_y;
			};

			/*! 
			 * Recalculates number of tiles X and Y \n
			 * based on width set by set_tile_width() \n
			 * Upper left is 0, 0
			 */
			static Projection_tile_data projection_calculate_tile_properties(unsigned short tile_width, unsigned short window_width, unsigned short window_height);

			/*!
			 * @param tile_data obtained from projection_calculate_tile_properties
			 * @param offset number of tiles horizontally to from the edge to hide (giving a zoom effect) \n
			 * Vertical tiles is calculated based on ration of tiles in tile_data
			 */
			static glm::mat4 to_proj_matrix(Projection_tile_data tile_data, float offset);
		};

	};
}

#endif // MVP_MANAGER_CPP
