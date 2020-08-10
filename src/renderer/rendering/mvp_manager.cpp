// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "renderer/rendering/mvp_manager.h"

#include <glm/gtc/matrix_transform.hpp>

#include "core/math.h"
#include "renderer/opengl/shader_manager.h"

// VIEW

glm::vec3 camera_transform = glm::vec3(0, 0, 0);

glm::vec3* jactorio::renderer::GetViewTransform() {
	return &camera_transform;
}

void jactorio::renderer::UpdateViewTransform() {
	const glm::mat4 view_mat = translate(glm::mat4(1.f), camera_transform);
	SetgViewMatrix(view_mat);
}


// PROJECTION

glm::mat4 jactorio::renderer::ToProjMatrix(const unsigned short window_width,
                                           const unsigned short window_height,
                                           const float offset) {
	// Calculate aspect ratio scale based on "larger / smaller"
	float x_zoom_ratio = 1.f;
	float y_zoom_ratio = 1.f;
	if (window_width > window_height) {
		x_zoom_ratio = core::LossyCast<float>(window_width) / core::LossyCast<float>(window_height);
	}
	else {
		y_zoom_ratio = core::LossyCast<float>(window_height) / core::LossyCast<float>(window_width);
	}

	return glm::ortho(
		offset * x_zoom_ratio,
		core::LossyCast<float>(window_width) - offset * x_zoom_ratio,

		core::LossyCast<float>(window_height) - offset * y_zoom_ratio,
		offset * y_zoom_ratio,

		-1.f, 1.f);
}
