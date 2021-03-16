// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_CORE_LOGGER_H
#define JACTORIO_INCLUDE_CORE_LOGGER_H
#pragma once

#include <cstdint>
#include <cstdio>
#include <type_traits>

#define LOG_MESSAGE(severity__, format__) \
    jactorio::MakeLogMessage<jactorio::LogSeverity::severity__>(format__, FILENAME, __LINE__)

// Allows the message to contain a format, similar to printf
#define LOG_MESSAGE_F(severity__, format__, ...) \
    jactorio::MakeLogMessage<jactorio::LogSeverity::severity__>(format__, FILENAME, __LINE__, __VA_ARGS__)


namespace jactorio
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


    /// Relative path supported, call this after setting the executing directory
    void OpenLogFile();
    void CloseLogFile();

    /// Logs a message to console
    /// Format: Timestamp [severity] - [group] message
    void LogMessage(LogSeverity severity, const char* group, int line, const char* message);

    /// Converts log_severity to a string
    /// \return The log severity as string
    const char* LogSeverityStr(LogSeverity severity);

    /// Converts log_severity to a string with color
    /// \return The log severity as string
    const char* LogSeverityStrColored(LogSeverity severity);


    /// Creates a formatted log message if log level permits
    template <LogSeverity Severity, typename... Args, typename = std::common_type<Args...>>
    void MakeLogMessage(const char* format, const char* file, const int line, Args&&... args) {
        if constexpr (static_cast<int>(Severity) >=
#include "_config/log_level"
        ) {
            char buffer[kMaxLogMsgLength + 1];
            snprintf(buffer, kMaxLogMsgLength, format, args...);
            LogMessage(Severity, file, line, buffer);
        }
    }
} // namespace jactorio

#endif // JACTORIO_INCLUDE_CORE_LOGGER_H
