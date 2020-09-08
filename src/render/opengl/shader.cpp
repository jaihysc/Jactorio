// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <GL/glew.h>

#include <vector>

#include "core/filesystem.h"

#include "core/logger.h"
#include "render/opengl/error.h"
#include "render/opengl/shader.h"

unsigned int jactorio::render::Shader::CompileShader(const std::string& filepath, const GLenum shader_type) {

    const auto path          = core::ResolvePath(filepath);
    const std::string source = core::ReadFile(path);

    if (source.empty()) {
        LOG_MESSAGE_F(error, "Shader compilation received empty string, type %d %s", shader_type, path.c_str());
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

        LOG_MESSAGE_F(error, "Shader compilation failed, type %d %s\n%s", shader_type, path.c_str(), message);

        delete[] message;

        return 0;
    }

    LOG_MESSAGE_F(info, "Shader compilation successful, type %d %s", shader_type, path.c_str());

    return shader_id;
}


jactorio::render::Shader::Shader(const std::vector<ShaderCreationInput>& inputs) : id_(0) {
    DEBUG_OPENGL_CALL(id_ = glCreateProgram());

    std::vector<unsigned int> shader_ids;
    for (const auto& input : inputs) {
        const unsigned int shader_id = CompileShader(input.filepath, input.shaderType);
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

jactorio::render::Shader::~Shader() {
    DEBUG_OPENGL_CALL(glDeleteProgram(id_));
}


void jactorio::render::Shader::Bind() const {
    DEBUG_OPENGL_CALL(glUseProgram(id_));
}

void jactorio::render::Shader::Unbind() {
    DEBUG_OPENGL_CALL(glUseProgram(0));
}


int jactorio::render::Shader::GetUniformLocation(const std::string& name) const {
    DEBUG_OPENGL_CALL(const int location = glGetUniformLocation(id_, name.c_str()));

    if (location == -1) {
        LOG_MESSAGE_F(error, "Attempted to get location of uniform \"%s\" which does not exist", name.c_str());
    }


    return location;
}


void jactorio::render::Shader::SetUniform1I(const int& location, const int v) {
    DEBUG_OPENGL_CALL(glUniform1i(location, v));
}

void jactorio::render::Shader::SetUniform4F(
    const int& location, const float& v0, const float& v1, const float& v2, const float& v3) {

    DEBUG_OPENGL_CALL(glUniform4f(location, v0, v1, v2, v3));
}

void jactorio::render::Shader::SetUniformMat4F(const int& location, glm::mat4& mat) {
    DEBUG_OPENGL_CALL(glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]));
}
