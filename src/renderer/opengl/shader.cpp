// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/15/2019

#include <GL/glew.h>

#include <vector>

#include "core/filesystem.h"

#include "renderer/opengl/shader.h"
#include "core/logger.h"
#include "renderer/opengl/error.h"

unsigned int jactorio::renderer::Shader::compile_shader(
	const std::string& filepath, const GLenum shader_type) {

	const auto path = core::filesystem::resolve_path(filepath);
	const std::string source = core::filesystem::read_file_as_str(path);

	if (source.empty()) {
		LOG_MESSAGE_f(error, "Shader compilation received empty string, type %d %s",
		              shader_type, path.c_str())
		return 0;
	}

	DEBUG_OPENGL_CALL(const unsigned int shader_id = glCreateShader(shader_type));
	const char* src = source.c_str();

	DEBUG_OPENGL_CALL(glShaderSource(shader_id, 1, &src, nullptr));
	DEBUG_OPENGL_CALL(glCompileShader(shader_id));

	// Error handling
	int result;
	DEBUG_OPENGL_CALL(glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result));

	if (result == GL_FALSE) {
		// Compilation failed
		int length;
		DEBUG_OPENGL_CALL(
			glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &length));

		char* message = new char[length];
		DEBUG_OPENGL_CALL(
			glGetShaderInfoLog(shader_id, length, &length, message));

		DEBUG_OPENGL_CALL(glDeleteShader(shader_id));

		LOG_MESSAGE_f(error, "Shader compilation failed, type %d %s\n%s",
		              shader_type, path.c_str(), message);

		delete[] message;

		return 0;
	}

	LOG_MESSAGE_f(info, "Shader compilation successful, type %d %s",
	              shader_type, path.c_str());

	return shader_id;
}


jactorio::renderer::Shader::Shader(
	const std::vector<Shader_creation_input>& inputs)
	: id_(0) {
	DEBUG_OPENGL_CALL(id_ = glCreateProgram());

	std::vector<unsigned int> shader_ids;
	for (const auto& input : inputs) {
		const unsigned int shader_id = compile_shader(
			input.filepath, input.shader_type);
		DEBUG_OPENGL_CALL(glAttachShader(id_, shader_id));
		shader_ids.push_back(shader_id);
	}

	DEBUG_OPENGL_CALL(glLinkProgram(id_));
	DEBUG_OPENGL_CALL(glValidateProgram(id_));

	// Shaders linked into program, no longer needed
	for (auto shader_id : shader_ids) {
		DEBUG_OPENGL_CALL(glDeleteShader(shader_id));
	}
}

jactorio::renderer::Shader::~Shader() {
	DEBUG_OPENGL_CALL(glDeleteProgram(id_));
}


void jactorio::renderer::Shader::bind() const {
	DEBUG_OPENGL_CALL(glUseProgram(id_));
}

void jactorio::renderer::Shader::unbind() {
	DEBUG_OPENGL_CALL(glUseProgram(0));
}


int jactorio::renderer::Shader::get_uniform_location(
	const std::string& name) const {
	DEBUG_OPENGL_CALL(
		const int location = glGetUniformLocation(id_, name.c_str()));

	if (location == -1) {
		LOG_MESSAGE_f(error, "Attempted to get location of uniform \"%s\" which does not exist",
		              name.c_str())
	}


	return location;
}


void jactorio::renderer::Shader::set_uniform_1i(const int& location, int v) {
	DEBUG_OPENGL_CALL(glUniform1i(location, v));
}

void jactorio::renderer::Shader::set_uniform_4f(
	const int& location, const float& v0, const float& v1, const float& v2,
	const float& v3) {

	DEBUG_OPENGL_CALL(glUniform4f(location, v0, v1, v2, v3));
}

void jactorio::renderer::Shader::set_uniform_mat_4f(
	const int& location, glm::mat4& mat) {
	DEBUG_OPENGL_CALL(glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]));
}
