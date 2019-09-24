#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>

class Texture
{
private:
	unsigned int renderer_id_;
	int width_, height_, bytes_per_pixel_;
	
	std::string texture_filepath_;
	unsigned char* texture_buffer_;

public:
	Texture(const std::string& texture_filepath);
	~Texture();

	void bind(unsigned int slot = 0) const;
	void unbind() const;

	int get_width() const { return width_; }
	int get_height() const { return height_; }
};

#endif // TEXTURE_H
