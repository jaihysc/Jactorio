// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "core/logger.h"

#include <ctime>
#include <fstream>
#include <iostream>
#include <string>

#include "core/filesystem.h"

std::ofstream log_file;
time_t start_time = clock();


void jactorio::core::OpenLogFile() {
    log_file.open(ResolvePath(kLogFileName));
}

void jactorio::core::CloseLogFile() {
    if (log_file.is_open())
        log_file.close();
}


constexpr char kLogFmt[] = "%10.3f %s [%s:%d] %s";

void jactorio::core::LogMessage(const LogSeverity severity,
                                const std::string& group,
                                const int line,
                                const std::string& message) {

    const float time = static_cast<float>(clock() - start_time) / CLOCKS_PER_SEC;

    const uint64_t buf_count = kMaxLogMsgLength + sizeof(kLogFmt);

    char s[buf_count];

    // Console

    snprintf(s,
             buf_count * sizeof(char),
             kLogFmt,
             time,
             LogSeverityStrColored(severity).c_str(),
             group.c_str(),
             line,
             message.c_str());

    std::cout << s << "\033[0m\n";

    // Log file

    snprintf(s,
             buf_count * sizeof(char),
             kLogFmt,
             time,
             LogSeverityStr(severity).c_str(),
             group.c_str(),
             line,
             message.c_str());

    log_file << s << "\n";
}

std::string jactorio::core::LogSeverityStr(const LogSeverity severity) {
    std::string severity_str;

    switch (severity) {
    case LogSeverity::debug:
        severity_str = "Debug   ";
        break;
    case LogSeverity::info:
        severity_str = "Info    ";
        break;
    case LogSeverity::warning:
        severity_str = "Warning ";
        break;
    case LogSeverity::error:
        severity_str = "ERROR   ";
        break;
    case LogSeverity::critical:
        severity_str = "CRITICAL";
        break;
    default:
        severity_str = "        ";
    }

    return severity_str;
}

std::string jactorio::core::LogSeverityStrColored(const LogSeverity severity) {
    std::string severity_str;

    switch (severity) {
    case LogSeverity::debug:
        severity_str = "\033[1;90mDebug   "; // Gray
        break;
    case LogSeverity::info:
        severity_str = "Info    ";
        break;
    case LogSeverity::warning:
        severity_str = "\033[1;33mWarning "; // Yellow
        break;
    case LogSeverity::error:
        severity_str = "\033[1;31mERROR   "; // Red
        break;
    case LogSeverity::critical:
        severity_str = "\033[1;31mCRITICAL"; // Red
        break;
    default:
        severity_str = "        ";
    }

    return severity_str;
}
