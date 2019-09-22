#include "renderer/camera.h"

#include "renderer/opengl/shader.h"
#include "renderer/opengl/shader_manager.h"

glm::vec3 camera_transform = glm::vec3(0, 0, 0);

glm::vec3* jactorio::get_camera_transform() {
	return &camera_transform;
}

void jactorio::update_camera_transform() {
	const glm::mat4 view_mat = glm::translate(glm::mat4(1.f), camera_transform);
	setg_view_matrix(view_mat);
}
