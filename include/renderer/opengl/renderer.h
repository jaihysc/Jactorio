#ifndef RENDERER_H
#define RENDERER_H

#include "renderer/opengl/index_buffer.h"
#include "renderer/opengl/vertex_array.h"

class Renderer
{
private:
public:
	Renderer();
	~Renderer();

	/*!
	 * Draws an object specified by va and ib, at position specified at translation
	 * Upper left corner is 0, 0
	 */
	void draw(const Vertex_array& va, const Index_buffer& ib, glm::vec3) const;
	void clear() const;
};

#endif // RENDERER_H