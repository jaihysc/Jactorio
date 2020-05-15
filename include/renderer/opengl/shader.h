// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/15/2019

#ifndef JACTORIO_INCLUDE_RENDERER_OPENGL_SHADER_H
#define JACTORIO_INCLUDE_RENDERER_OPENGL_SHADER_H
#pragma once

#include <string>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "jactorio.h"

namespace jactorio::renderer
{
	struct ShaderCreationInput
	{
		std::string filepath;
		GLenum shaderType;
	};

	class Shader
	{
		unsigned int id_;

		static unsigned int CompileShader(const std::string& filepath,
		                                  GLenum shader_type);

	public:
		explicit Shader(const std::vector<ShaderCreationInput>& inputs);
		~Shader();

		Shader(const Shader& other)                = delete;
		Shader(Shader&& other) noexcept            = delete;
		Shader& operator=(const Shader& other)     = delete;
		Shader& operator=(Shader&& other) noexcept = delete;
		void Bind() const;
		static void Unbind();

		J_NODISCARD int GetUniformLocation(const std::string& name) const;
		static void SetUniform1I(const int& location, int v);
		static void SetUniform4F(const int& location, const float& v0,
		                         const float& v1, const float& v2,
		                         const float& v3);
		static void SetUniformMat4F(const int& get_uniform_location,
		                            glm::mat4& mat);
	};
}

#endif //JACTORIO_INCLUDE_RENDERER_OPENGL_SHADER_H
