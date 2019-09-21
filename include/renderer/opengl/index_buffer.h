#ifndef INDEX_BUFFER_H
#define INDEX_BUFFER_H

class Index_buffer
{
private:
	unsigned int id_ {};
	unsigned int count_;
	
public:
	// Count is numerical count
	Index_buffer(const unsigned int* data, unsigned int count);
	~Index_buffer();

	void bind() const;
	static void unbind();

	unsigned int count() const;
};

#endif // INDEX_BUFFER_H
