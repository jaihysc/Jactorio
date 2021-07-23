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

    struct ShaderSymbol
    {
        /// Looks for this value
        std::string name;
        /// Replaced with this
        std::string value;
    };

    class Shader
    {
    public:
        Shader() = default;
        ~Shader();

        Shader(const Shader& other)     = delete;
        Shader(Shader&& other) noexcept = delete;
        Shader& operator=(const Shader& other) = delete;
        Shader& operator=(Shader&& other) noexcept = delete;

        /// \param symbols Defines symbol for shader compilation, the symbol in shader programs will be replaced with
        /// value
        /// \exception RendererException Shader program is invalid for use
        void Init(const std::vector<ShaderCreationInput>& inputs, const std::vector<ShaderSymbol>& symbols = {});

        void Bind() const noexcept;
        static void Unbind() noexcept;

        J_NODISCARD int GetUniformLocation(const char* name) const noexcept;
        J_NODISCARD int GetAttribLocation(const char* name) const noexcept;

    private:
        static GLuint CompileShader(const std::string& filepath,
                                    GLenum shader_type,
                                    const std::vector<ShaderSymbol>& symbols) noexcept;
        void LinkProgram() const noexcept;
        /// \exception RendererException Validation failed
        void ValidateProgram() const;

        /// Retrieves log from opengl, null terminator included
        static std::vector<char> GetShaderInfoLog(GLuint shader_id);
        /// Retrieves log from opengl, null terminator included
        static std::vector<char> GetProgramInfoLog(GLuint program);

        unsigned int id_ = 0;
    };
} // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_OPENGL_SHADER_H
