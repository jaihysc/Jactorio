#ifndef DATA_DATA_EXCEPTION_H
#define DATA_DATA_EXCEPTION_H

#include <sstream>
#include <stdexcept>

namespace jactorio::data
{
	// These are raised by prototype classes if an error occurred
	class Data_exception : public std::runtime_error
	{
		using std::runtime_error::runtime_error;
	};
}

#endif // DATA_DATA_EXCEPTION_H
