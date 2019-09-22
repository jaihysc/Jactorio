#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace jactorio
{
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
}

#endif // CAMERA_H
