// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_DATA_DATA_EXCEPTION_H
#define JACTORIO_INCLUDE_DATA_DATA_EXCEPTION_H
#pragma once

#include <stdexcept>

namespace jactorio::data
{
	// These are raised by prototype classes if an error occurred
	class DataException : public std::runtime_error
	{
		using std::runtime_error::runtime_error;
	};
}

#endif //JACTORIO_INCLUDE_DATA_DATA_EXCEPTION_H
