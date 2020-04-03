// 
// logger.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 10/15/2019
// Last modified: 04/03/2020
// 

#include "core/logger.h"

#include <string>
#include <iostream>
#include <fstream>
#include <ctime>

#include "core/filesystem.h"

std::ofstream log_file;
time_t start_time = clock();


void jactorio::core::logger::open_log_file(const std::string& path) {
	log_file.open(filesystem::resolve_path(path));
}

void jactorio::core::logger::close_log_file() {
	log_file.close();
}


std::string jactorio::core::logger::gen_log_message(const logSeverity severity, const std::string& group,
                                                    const int line, const std::string& message) {

	const float time = static_cast<float>(clock() - start_time) / CLOCKS_PER_SEC;

	char s[1100];
	snprintf(s, 1100 * sizeof(char), "\033[0m%10.3f %s [%s:%d] %s\n", time,
	         log_severity_str(severity).c_str(),
	         group.c_str(),
	         line,
	         message.c_str());

	return s;
}

void jactorio::core::logger::log_message(const logSeverity severity,
                                         const std::string& group,
                                         const int line, const std::string& message) {

	const auto msg = gen_log_message(severity, group, line, message);
	std::cout << msg;
	log_file << msg;
}

std::string jactorio::core::logger::log_severity_str(
	const logSeverity severity) {
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
