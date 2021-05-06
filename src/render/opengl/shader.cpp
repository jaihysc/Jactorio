// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <GL/glew.h>

#include "core/filesystem.h"
#include "core/logger.h"
#include "render/opengl/error.h"
#include "render/opengl/shader.h"
#include "render/renderer_exception.h"

using namespace jactorio;

render::Shader::Shader(const std::vector<ShaderCreationInput>& inputs) : id_(0) {
    DEBUG_OPENGL_CALL(id_ = glCreateProgram());

    std::vector<unsigned int> shader_ids;
    for (const auto& [filepath, shaderType] : inputs) {
        const unsigned int shader_id = CompileShader(filepath, shaderType);
        DEBUG_OPENGL_CALL(glAttachShader(id_, shader_id));
        shader_ids.push_back(shader_id);
    }

    LinkProgram();
    ValidateProgram();

    // Shaders linked into program, no longer needed
    for (auto shader_id : shader_ids) {
        DEBUG_OPENGL_CALL(glDetachShader(id_, shader_id));
        DEBUG_OPENGL_CALL(glDeleteShader(shader_id));
    }
}

render::Shader::~Shader() {
    DEBUG_OPENGL_CALL(glDeleteProgram(id_));
}


void render::Shader::Bind() const noexcept {
    DEBUG_OPENGL_CALL(glUseProgram(id_));
}

void render::Shader::Unbind() noexcept {
    DEBUG_OPENGL_CALL(glUseProgram(0));
}


int render::Shader::GetUniformLocation(const std::string& name) const noexcept {
    DEBUG_OPENGL_CALL(const int location = glGetUniformLocation(id_, name.c_str()));

    if (location == -1) {
        LOG_MESSAGE_F(error, "Attempted to get location of uniform \"%s\" which does not exist", name.c_str());
    }


    return location;
}

GLuint render::Shader::CompileShader(const std::string& filepath, const GLenum shader_type) noexcept {

    const std::string source = ReadFile(filepath);

    if (source.empty()) {
        LOG_MESSAGE_F(error, "Shader compilation received empty string, type %d %s", shader_type, filepath.c_str());
        return 0;
    }

    DEBUG_OPENGL_CALL(const GLuint shader_id = glCreateShader(shader_type));
    const char* src = source.c_str();

    DEBUG_OPENGL_CALL(glShaderSource(shader_id, 1, &src, nullptr));
    DEBUG_OPENGL_CALL(glCompileShader(shader_id));


    GLint result;
    DEBUG_OPENGL_CALL(glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result));

    if (result == GL_FALSE) {
        LOG_MESSAGE_F(error,
                      "Shader compilation failed, type %d %s\n%s",
                      shader_type,
                      filepath.c_str(),
                      GetShaderInfoLog(shader_id).data());
    }
    else {
        LOG_MESSAGE_F(info, "Shader compilation successful, type %d %s", shader_type, filepath.c_str());
    }

    return shader_id;
}

void render::Shader::LinkProgram() const noexcept {
    DEBUG_OPENGL_CALL(glLinkProgram(id_));

    GLint result;
    DEBUG_OPENGL_CALL(glGetProgramiv(id_, GL_LINK_STATUS, &result));

    if (result == GL_FALSE) {
        LOG_MESSAGE_F(error, "Shader program linkage failed\n%s", GetProgramInfoLog(id_).data());
    }
    else {
        LOG_MESSAGE(info, "Shader program linkage successful");
    }
}

void render::Shader::ValidateProgram() const {
    DEBUG_OPENGL_CALL(glValidateProgram(id_));

    GLint result;
    DEBUG_OPENGL_CALL(glGetProgramiv(id_, GL_VALIDATE_STATUS, &result));

    if (result == GL_FALSE) {
        throw RendererException("Shader program validation failed");
    }

    LOG_MESSAGE(info, "Shader program validated");
}

std::vector<char> render::Shader::GetShaderInfoLog(const GLuint shader_id) {
    GLint length;
    DEBUG_OPENGL_CALL(glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &length));

    std::vector<char> message;
    message.reserve(length);

    DEBUG_OPENGL_CALL(glGetShaderInfoLog(shader_id, length, &length, message.data()));

    return message;
}

std::vector<char> render::Shader::GetProgramInfoLog(const GLuint program) {
    GLint length;
    DEBUG_OPENGL_CALL(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length));

    std::vector<char> message;
    message.reserve(length);

    DEBUG_OPENGL_CALL(glGetProgramInfoLog(program, length, &length, message.data()));

    return message;
}
