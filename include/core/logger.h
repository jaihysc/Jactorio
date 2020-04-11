// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/15/2019

#ifndef JACTORIO_INCLUDE_CORE_LOGGER_H
#define JACTORIO_INCLUDE_CORE_LOGGER_H
#pragma once

#include <string>

// Damn Windows has to use backslashes for paths
// Cuts away paths, keeps only the filename
#ifdef _MSC_VER
#define FILENAME (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#undef __GNUC__
#endif

#ifdef __GNUC__
#include <cstring>
#define FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#undef _MSC_VER
#endif

// Logging macros
//
// Prefer calling LOG_MESSAGE to log a message over log_message()
#define LOG_MESSAGE(severity, message)\
if constexpr(static_cast<int>(jactorio::core::logger::logSeverity::severity) >= JACTORIO_LOG_LEVEL) { \
	log_message(\
	jactorio::core::logger::logSeverity::severity, \
	FILENAME, \
	__LINE__, \
	message); \
}

// Allows the message to contain a format, similar to printf
#define LOG_MESSAGE_f(severity, format, ...)\
if constexpr(static_cast<int>(jactorio::core::logger::logSeverity::severity) >= JACTORIO_LOG_LEVEL) {    \
char buffer[1000];\
snprintf(buffer, sizeof(char) * 1000, format, __VA_ARGS__);\
log_message(\
	jactorio::core::logger::logSeverity::severity, \
	FILENAME, \
	__LINE__, \
	buffer);\
}


namespace jactorio::core::logger
{
	enum class logSeverity
	{
		debug = 0,
		info,
		warning,
		error,
		critical,
		none
	};

	/**
	 * Relative path supported, call this after setting the executing directory
	 */
	void open_log_file(const std::string& path);
	void close_log_file();

	/**
	 * Logs a message to console <br>
	 * Format: Timestamp [severity] - [group] message
	 */
	void log_message(logSeverity severity, const std::string& group,
	                 int line, const std::string& message);

	/** Generates a log message
    * Format: Timestamp [severity] - [group] message
	* @return The logged message as string
	*/
	std::string gen_log_message(logSeverity severity, const std::string& group,
	                            int line, const std::string& message);

	/** Logs a message to console
	* Converts log_severity to a string
	* @return The log severity as string
	*/
	std::string log_severity_str(logSeverity severity);
}

#endif //JACTORIO_INCLUDE_CORE_LOGGER_H
