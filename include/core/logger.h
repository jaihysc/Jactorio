#ifndef CORE_LOGGER_CPP
#define CORE_LOGGER_CPP

#include <string>

namespace jactorio::core::logger
{
	enum log_severity
	{
		debug,
		info,
		warning,
		error,
		critical
	};

	/**
	 * Logs a message to console <br>
	 * Format: Timestamp [severity] - [group] message
	 */
	void log_message(log_severity severity, const std::string& group,
	                 const std::string& message);

	/** Generates a log message
    * Format: Timestamp [severity] - [group] message
	* @return The logged message as string
	*/
	std::string gen_log_message(log_severity severity, const std::string& group,
	                            const std::string& message);

	/** Logs a message to console
	* Converts log_severity to a string
	* @return The log severity as string
	*/
	std::string log_severity_str(log_severity severity);
}

#endif // CORE_LOGGER_CPP
