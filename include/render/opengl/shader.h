// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDER_OPENGL_SHADER_H
#define JACTORIO_INCLUDE_RENDER_OPENGL_SHADER_H
#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "jactorio.h"

namespace jactorio::render
{
    struct ShaderCreationInput
    {
        std::string filepath;
        GLenum shaderType;
    };

    class Shader
    {
    public:
        /// \exception RendererException Shader program is invalid for use
        explicit Shader(const std::vector<ShaderCreationInput>& inputs);
        ~Shader();

        Shader(const Shader& other)     = delete;
        Shader(Shader&& other) noexcept = delete;
        Shader& operator=(const Shader& other) = delete;
        Shader& operator=(Shader&& other) noexcept = delete;

        void Bind() const noexcept;
        static void Unbind() noexcept;

        J_NODISCARD int GetUniformLocation(const std::string& name) const noexcept;
        static void SetUniform1I(const int& location, int v) noexcept;
        static void SetUniform4F(
            const int& location, const float& v0, const float& v1, const float& v2, const float& v3) noexcept;
        static void SetUniformMat4F(const int& location, glm::mat4& mat) noexcept;

    private:
        static GLuint CompileShader(const std::string& filepath, GLenum shader_type) noexcept;
        void LinkProgram() const noexcept;
        /// \exception RendererException Validation failed
        void ValidateProgram() const;

        /// Retrieves log from opengl, null terminator included
        static std::vector<char> GetShaderInfoLog(GLuint shader_id);
        /// Retrieves log from opengl, null terminator included
        static std::vector<char> GetProgramInfoLog(GLuint program);

        unsigned int id_;
    };
} // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_OPENGL_SHADER_H
