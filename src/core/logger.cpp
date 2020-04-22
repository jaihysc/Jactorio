// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/15/2019

#include "core/logger.h"

#include <ctime>
#include <fstream>
#include <iostream>
#include <string>

#include "core/filesystem.h"

std::ofstream log_file;
time_t start_time = clock();


void jactorio::core::open_log_file(const std::string& path) {
	log_file.open(resolve_path(path));
}

void jactorio::core::close_log_file() {
	log_file.close();
}


constexpr char log_fmt[] = "\033[0m%10.3f %s [%s:%d] %s\n";

std::string jactorio::core::gen_log_message(const logSeverity severity,
                                            const std::string& group,
                                            const int line,
                                            const std::string& message) {

	const float time = static_cast<float>(clock() - start_time) / CLOCKS_PER_SEC;

	const uint64_t buf_count = max_log_msg_length + sizeof(log_fmt);

	char s[buf_count];
	snprintf(s,
	         buf_count * sizeof(char),
	         log_fmt,
	         time,
	         log_severity_str(severity).c_str(),
	         group.c_str(),
	         line,
	         message.c_str());

	return s;
}

void jactorio::core::log_message(const logSeverity severity,
                                 const std::string& group,
                                 const int line,
                                 const std::string& message) {
	const auto msg = gen_log_message(severity, group, line, message);

	std::cout << msg;
	log_file << msg;
}

std::string jactorio::core::log_severity_str(const logSeverity severity) {
	std::string severity_str;

	switch (severity) {
	case logSeverity::debug:
		severity_str = "\033[1;90mDebug   ";  // Gray
		break;
	case logSeverity::info:
		severity_str = "Info    ";
		break;
	case logSeverity::warning:
		severity_str = "\033[1;33mWarning ";  // Yellow
		break;
	case logSeverity::error:
		severity_str = "\033[1;31mERROR   ";  // Red
		break;
	case logSeverity::critical:
		severity_str = "\033[1;31mCRITICAL";  // Red
		break;
	default:
		severity_str = "        ";
	}

	return severity_str;
}
