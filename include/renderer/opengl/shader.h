#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Shader_creation_input {
	std::string filepath;
	GLenum shader_type;
};

class Shader
{
private:
	unsigned int id_;

	static unsigned int compile_shader(const std::string& filepath, const GLenum shader_type);
	
public:
	explicit Shader(const std::vector<Shader_creation_input>& inputs);
	~Shader();

	void bind() const;
	static void unbind();

	int get_uniform_location(const std::string& name) const;
	static void set_uniform_1i(const int& location, int v);
	static void set_uniform_4f(const int& location, const float& v0, const float& v1, const float& v2, const float& v3);
	static void set_uniform_mat_4f(const int& get_uniform_location, glm::mat4& mat);
	
};

#endif // SHADER_H
