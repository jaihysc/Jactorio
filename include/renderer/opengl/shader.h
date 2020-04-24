// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/15/2019

#ifndef JACTORIO_INCLUDE_RENDERER_OPENGL_SHADER_H
#define JACTORIO_INCLUDE_RENDERER_OPENGL_SHADER_H
#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>

#include "jactorio.h"

namespace jactorio::renderer
{
	struct Shader_creation_input
	{
		std::string filepath;
		GLenum shader_type;
	};

	class Shader
	{
		unsigned int id_;

		static unsigned int compile_shader(const std::string& filepath,
		                                   GLenum shader_type);

	public:
		explicit Shader(const std::vector<Shader_creation_input>& inputs);
		~Shader();

		Shader(const Shader& other) = delete;
		Shader(Shader&& other) noexcept = delete;
		Shader& operator=(const Shader& other) = delete;
		Shader& operator=(Shader&& other) noexcept = delete;
		void bind() const;
		static void unbind();

		J_NODISCARD int get_uniform_location(const std::string& name) const;
		static void set_uniform_1i(const int& location, int v);
		static void set_uniform_4f(const int& location, const float& v0,
		                           const float& v1, const float& v2,
		                           const float& v3);
		static void set_uniform_mat_4f(const int& get_uniform_location,
		                               glm::mat4& mat);
	};
}

#endif //JACTORIO_INCLUDE_RENDERER_OPENGL_SHADER_H
