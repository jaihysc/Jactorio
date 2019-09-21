#ifndef LOGGER_CPP
#define LOGGER_CPP

#include <string>

namespace logger
{
	enum log_severity
	{
		debug,
		info,
		warning,
		error,
		critical
	};
	
	/*! Logs a message to console	*/
	void log_message(log_severity severity, const std::string& group, const std::string& message);

	/*! Generates a log message
	* @return: The logged message as string
	*/
	std::string gen_log_message(log_severity severity, const std::string& group, const std::string& message);

	/*! Logs a message to console
	 * Converts log_severity to a string
	* @return: The log severity as string
	*/
	std::string log_severity_str(log_severity severity);
}

#endif // LOGGER_CPP