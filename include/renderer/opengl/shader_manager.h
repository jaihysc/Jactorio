// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDERER_OPENGL_SHADER_MANAGER_H
#define JACTORIO_INCLUDE_RENDERER_OPENGL_SHADER_MANAGER_H
#pragma once

#include <glm/glm.hpp>

namespace jactorio::renderer
{
	void SetMvpUniformLocation(int location);

	///
	/// \brief Sends MVP data to GPU
	void UpdateShaderMvp();
	const glm::mat4& GetMvpMatrix();

	void SetgModelMatrix(const glm::mat4& matrix);
	void SetgViewMatrix(const glm::mat4& matrix);
	void SetgProjectionMatrix(const glm::mat4& matrix);
}

#endif //JACTORIO_INCLUDE_RENDERER_OPENGL_SHADER_MANAGER_H
