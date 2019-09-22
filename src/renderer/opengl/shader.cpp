#include <GL/glew.h>

#include <sstream>
#include <vector>

#include "core/file_system.h"

#include "renderer/opengl/shader.h"
#include "renderer/opengl/error.h"
#include "core/logger.h"

unsigned int Shader::compile_shader(const std::string& filepath, const GLenum shader_type) {
	const std::string source = read_file_as_str(filepath);
	
	if (source.empty()) {
		logger::log_message(logger::error, "OpenGL", "Shader compilation received empty shader string, ignoring");
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
		DEBUG_OPENGL_CALL(glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &length));

		char* message = new char[length];
		DEBUG_OPENGL_CALL(glGetShaderInfoLog(shader_id, length, &length, message));

		DEBUG_OPENGL_CALL(glDeleteShader(shader_id));

		std::ostringstream oss;
		oss << "Shader compilation failed: " << shader_type << "\n";
		oss << message;
		logger::log_message(logger::error, "OpenGL", oss.str());
		
		delete[] message;

		return 0;
	}

	std::ostringstream oss;
	oss << "Shader compilation successful: " << shader_type;
	logger::log_message(logger::debug, "OpenGL", oss.str());
	return shader_id;
}


Shader::Shader(const std::vector<Shader_creation_input>& inputs)
	: id_(0)
{
	DEBUG_OPENGL_CALL(id_ = glCreateProgram());

	std::vector<unsigned int> shader_ids;
	for (const auto& input : inputs) {
		const unsigned int shader_id = compile_shader(input.filepath, input.shader_type);
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

Shader::~Shader() {
	DEBUG_OPENGL_CALL(glDeleteProgram(id_));
}


void Shader::bind() const {
	DEBUG_OPENGL_CALL(glUseProgram(id_));
}

void Shader::unbind() {
	DEBUG_OPENGL_CALL(glUseProgram(0));
}


int Shader::get_uniform_location(const std::string& name) const {
	DEBUG_OPENGL_CALL(const int location = glGetUniformLocation(id_, name.c_str()));

	if (location == -1) {
		std::ostringstream oss;
		oss << "Attempted to get location of uniform: \"" << name << "\" which does not exist";
		logger::log_message(logger::error, "OpenGL", oss.str());
	}

	
	return location;
}


void Shader::set_uniform_1i(const int& location, int v) {
	DEBUG_OPENGL_CALL(glUniform1i(location, v));
}

void Shader::set_uniform_4f(
	const int& location, const float& v0, const float& v1, const float& v2, const float& v3) {
	
	DEBUG_OPENGL_CALL(glUniform4f(location, v0, v1, v2, v3));
}

void Shader::set_uniform_mat_4f(const int& location, glm::mat4& mat) {
	DEBUG_OPENGL_CALL(glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]));
}
