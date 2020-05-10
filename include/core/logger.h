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
if constexpr(static_cast<int>(jactorio::core::LogSeverity::severity) >= JACTORIO_LOG_LEVEL) { \
	LogMessage(\
	jactorio::core::LogSeverity::severity, \
	FILENAME, \
	__LINE__, \
	message); \
}

// Allows the message to contain a format, similar to printf
#define LOG_MESSAGE_f(severity, format, ...)\
if constexpr(static_cast<int>(jactorio::core::LogSeverity::severity) >= JACTORIO_LOG_LEVEL) {\
char buffer[jactorio::core::kMaxLogMsgLength];\
snprintf(buffer, sizeof(char) * jactorio::core::kMaxLogMsgLength, format, __VA_ARGS__);\
LogMessage(\
	jactorio::core::LogSeverity::severity, \
	FILENAME, \
	__LINE__, \
	buffer);\
}


namespace jactorio::core
{
	/// Maximum number of characters in log message
	constexpr uint16_t kMaxLogMsgLength = 1000;

	enum class LogSeverity
	{
		debug = 0,
		info,
		warning,
		error,
		critical,
		none
	};

	///
	/// \brief Relative path supported, call this after setting the executing directory
	void OpenLogFile(const std::string& path);
	void CloseLogFile();

	///
	/// \brief Logs a message to console
	/// Format: Timestamp [severity] - [group] message
	void LogMessage(LogSeverity severity, const std::string& group,
	                int line, const std::string& message);

	///
	/// \brief Generates a log message
    /// Format: Timestamp [severity] - [group] message
	/// \return The logged message as string
	std::string GenLogMessage(LogSeverity severity, const std::string& group,
	                          int line, const std::string& message);

	///
	/// \brief Converts log_severity to a string
	/// \return The log severity as string
	std::string LogSeverityStr(LogSeverity severity);
}

#endif //JACTORIO_INCLUDE_CORE_LOGGER_H
