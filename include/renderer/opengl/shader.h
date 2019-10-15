#ifndef RENDERER_OPENGL_SHADER_H
#define RENDERER_OPENGL_SHADER_H

#include <GL/glew.h>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

		[[nodiscard]] int get_uniform_location(const std::string& name) const;
		static void set_uniform_1i(const int& location, int v);
		static void set_uniform_4f(const int& location, const float& v0,
		                           const float& v1, const float& v2,
		                           const float& v3);
		static void set_uniform_mat_4f(const int& get_uniform_location,
		                               glm::mat4& mat);
	};
}

#endif // RENDERER_OPENGL_SHADER_H
