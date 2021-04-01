// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "core/logger.h"

#include <ctime>
#include <fstream>
#include <iostream>

using namespace jactorio;

std::ofstream log_file;
time_t start_time = clock();

#ifdef _WIN32
#include <Windows.h>
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#endif

#define LABEL_DEBUG    "Debug   "
#define LABEL_INFO     "Info    "
#define LABEL_WARNING  "Warning "
#define LABEL_ERROR    "ERROR   "
#define LABEL_CRITICAL "CRITICAL"
#define LABEL_NONE     "        "

void jactorio::OpenLogFile() {
    log_file.open(kLogFileName);
}

void jactorio::CloseLogFile() {
    if (log_file.is_open())
        log_file.close();
}


constexpr char kLogFmt[] = "%10.3f %s [%s:%d] %s";

void jactorio::LogMessage(const LogSeverity severity, const char* group, const int line, const char* message) {

    const float time = static_cast<float>(clock() - start_time) / CLOCKS_PER_SEC;

    const uint64_t buf_count = kMaxLogMsgLength + sizeof(kLogFmt);

    char s[buf_count];

    // Console

    snprintf(s, buf_count * sizeof(char), kLogFmt, time, LogSeverityStrColored(severity), group, line, message);

#ifdef _WIN32
    std::cout << s << "\n";
#else
    std::cout << s << "\033[0m\n";
#endif

    // Log file

    snprintf(s, buf_count * sizeof(char), kLogFmt, time, LogSeverityStr(severity), group, line, message);

    log_file << s << "\n";
}

const char* jactorio::LogSeverityStr(const LogSeverity severity) {
    switch (severity) {
    case LogSeverity::debug:
        return LABEL_DEBUG;
    case LogSeverity::info:
        return LABEL_INFO;
    case LogSeverity::warning:
        return LABEL_WARNING;
    case LogSeverity::error:
        return LABEL_ERROR;
    case LogSeverity::critical:
        return LABEL_CRITICAL;
    default:
        return LABEL_NONE;
    }
}

const char* jactorio::LogSeverityStrColored(const LogSeverity severity) {
#ifdef _WIN32
    switch (severity) {
    case LogSeverity::debug:
        SetConsoleTextAttribute(hConsole, 8); // Gray
        return LABEL_DEBUG;
    case LogSeverity::info:
        return LABEL_INFO;
    case LogSeverity::warning:
        SetConsoleTextAttribute(hConsole, 14); // Yellow
        return LABEL_WARNING;
    case LogSeverity::error:
        SetConsoleTextAttribute(hConsole, 12); // Red
        return LABEL_ERROR;
    case LogSeverity::critical:
        SetConsoleTextAttribute(hConsole, 12); // Red
        return LABEL_CRITICAL;
    default:
        return LABEL_NONE;
    }
#else
    switch (severity) {
    case LogSeverity::debug:
        return "\033[1;90m" LABEL_DEBUG; // Gray
    case LogSeverity::info:
        return LABEL_INFO;
    case LogSeverity::warning:
        return "\033[1;33m" LABEL_WARNING; // Yellow
    case LogSeverity::error:
        return "\033[1;31m" LABEL_ERROR; // Red
    case LogSeverity::critical:
        return "\033[1;31m" LABEL_CRITICAL; // Red
    default:
        return LABEL_NONE;
    }
#endif
}
