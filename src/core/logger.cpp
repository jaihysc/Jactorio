#include "core/logger.h"

#include <string>
#include <iostream>
#include <fstream>
#include <ctime>

#include "core/filesystem.h"

namespace
{
	std::ofstream log_file;
	time_t start_time = clock();
}


void jactorio::core::logger::open_log_file(const std::string& path) {
	log_file.open(filesystem::resolve_path(path));
}

void jactorio::core::logger::close_log_file() {
	log_file.close();
}


std::string jactorio::core::logger::gen_log_message(const log_severity severity, const std::string& group,
                                                    const int line, const std::string& message) {
	
	const float time = static_cast<float>(clock() - start_time) / CLOCKS_PER_SEC;

	char s[10000];
	snprintf(s, 10000 * sizeof(char), "%10.3f %s [%s:%d] %s\n", time,
	         log_severity_str(severity).c_str(), 
	         group.c_str(),
	         line,
	         message.c_str());
	
	return s;
}

void jactorio::core::logger::log_message(const log_severity severity,
                                         const std::string& group,
                                         const int line, const std::string& message) {

	const auto msg = gen_log_message(severity, group, line, message);
	std::cout << msg;
	log_file << msg;
}

std::string jactorio::core::logger::log_severity_str(
	const log_severity severity) {
	std::string severity_str;
	switch (severity) {
	case log_severity::debug:
		severity_str = "Debug   ";
		break;
	case log_severity::info:
		severity_str = "Info    ";
		break;
	case log_severity::warning:
		severity_str = "Warning ";
		break;
	case log_severity::error:
		severity_str = "ERROR   ";
		break;
	case log_severity::critical:
		severity_str = "CRITICAL";
		break;
	default:
		severity_str = "        ";
	}

	return severity_str;
}
