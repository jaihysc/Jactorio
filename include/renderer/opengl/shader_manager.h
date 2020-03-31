// 
// shader_manager.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 10/22/2019
// Last modified: 03/14/2020
// 

#ifndef JACTORIO_INCLUDE_RENDERER_OPENGL_SHADER_MANAGER_H
#define JACTORIO_INCLUDE_RENDERER_OPENGL_SHADER_MANAGER_H
#pragma once

#include <glm/glm.hpp>

namespace jactorio::renderer
{
	void set_mvp_uniform_location(int location);

	/**
	 * Sends MVP data to GPU
	 */
	void update_shader_mvp();
	const glm::mat4& get_mvp_matrix();

	void setg_model_matrix(const glm::mat4& matrix);
	void setg_view_matrix(const glm::mat4& matrix);
	void setg_projection_matrix(const glm::mat4& matrix);
}

#endif //JACTORIO_INCLUDE_RENDERER_OPENGL_SHADER_MANAGER_H
