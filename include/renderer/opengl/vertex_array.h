#ifndef VERTEX_ARRAY_H
#define VERTEX_ARRAY_H

#include "renderer/opengl/vertex_buffer.h"

class Vertex_array
{
private:
	unsigned int id_ {};
	
public:
	Vertex_array();
	~Vertex_array();

	void add_buffer(const Vertex_buffer& vb, const unsigned span, unsigned location) const;
	
	void bind() const;
	static void unbind();
};

#endif // VERTEX_ARRAY_H
