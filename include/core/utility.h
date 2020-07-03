// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 07/03/2020

#ifndef JACTORIO_CORE_UTILITY_H
#define JACTORIO_CORE_UTILITY_H
#pragma once

#include <algorithm>
#include <string>

namespace jactorio::core
{
	///
	/// \brief Converts provided string to lower case
	inline std::string StrToLower(const std::string& str) {
		std::string temp = str;
		std::transform(temp.begin(), temp.end(), temp.begin(), tolower);
		return temp;
	}

	///
	/// \brief Converts provided string to lower case
	inline std::string StrToUpper(const std::string& str) {
		std::string temp = str;
		std::transform(temp.begin(), temp.end(), temp.begin(), toupper);
		return temp;
	}
}

#endif // JACTORIO_CORE_UTILITY_H
