// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_CORE_LOGGER_H
#define JACTORIO_INCLUDE_CORE_LOGGER_H
#pragma once

#include <string>
#include <type_traits>

// Damn Windows has to use backslashes for paths
// Cuts away paths, keeps only the filename
#ifdef _MSC_VER
#define FILENAME (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#endif

#ifdef __GNUC__
#include <cstring>
#define FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#undef _MSC_VER
#endif

// Logging macros
//
// Prefer calling LOG_MESSAGE to log a message over log_message()
#define LOG_MESSAGE(severity__, format__) \
    jactorio::core::MakeLogMessage<jactorio::core::LogSeverity::severity__>(format__, FILENAME, __LINE__)

// Allows the message to contain a format, similar to printf
#define LOG_MESSAGE_F(severity__, format__, ...) \
    jactorio::core::MakeLogMessage<jactorio::core::LogSeverity::severity__>(format__, FILENAME, __LINE__, __VA_ARGS__)


namespace jactorio::core
{
    constexpr char kLogFileName[] = "log.txt";

    /// Maximum number of characters in log message
    constexpr uint16_t kMaxLogMsgLength = 1000;

    enum class LogSeverity
    {
        debug = 0,
        info,
        warning,
        error,
        critical
    };


    ///
    /// Relative path supported, call this after setting the executing directory
    void OpenLogFile();
    void CloseLogFile();

    ///
    /// Logs a message to console
    /// Format: Timestamp [severity] - [group] message
    void LogMessage(LogSeverity severity, const std::string& group, int line, const std::string& message);

    ///
    /// Converts log_severity to a string
    /// \return The log severity as string
    std::string LogSeverityStr(LogSeverity severity);

    ///
    /// Converts log_severity to a string with color
    /// \return The log severity as string
    std::string LogSeverityStrColored(LogSeverity severity);


    ///
    /// Creates a formatted log message if log level permits
    template <LogSeverity Severity, typename... Args, typename = std::common_type<Args...>>
    void MakeLogMessage(const char* format, const char* file, const int line, Args&&... args) {
        if constexpr (static_cast<int>(Severity) >= JACTORIO_LOG_LEVEL) {
            char buffer[kMaxLogMsgLength + 1];
            snprintf(buffer, kMaxLogMsgLength, format, args...);
            LogMessage(Severity, file, line, buffer);
        }
    }
} // namespace jactorio::core

#endif // JACTORIO_INCLUDE_CORE_LOGGER_H
