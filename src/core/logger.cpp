#include "core/logger.h"

#include <string>
#include <sstream>
#include <iostream>
#include <ctime>


namespace logger
{
	time_t start_time = time(0);
	
	std::string gen_log_message(const log_severity severity, const std::string& group, const std::string& message) {
		std::ostringstream oss;

		oss << difftime(time(0), start_time) << " [" << log_severity_str(severity) << "] - <" << group << "> " << message << "\n";

		std::string msg = oss.str();
		return msg;
	}

	void log_message(const log_severity severity, const std::string& group, const std::string& message) {
		std::cout << gen_log_message(severity, group, message);
	}

	std::string log_severity_str(log_severity severity) {
		std::string severity_str;
		switch (severity) {
		case debug:
			severity_str = "Debug";
			break;
		case info:
			severity_str = "Info";
			break;
		case warning:
			severity_str = "Warning";
			break;
		case error:
			severity_str = "ERROR";
			break;
		case critical:
			severity_str = "CRITICAL";
			break;
		default:
			severity_str = "?";
		}
	
		return severity_str;
	}
}
