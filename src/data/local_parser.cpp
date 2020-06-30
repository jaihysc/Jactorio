// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/15/2020

#include "data/local_parser.h"

#include <sstream>

#include "jactorio.h"

#include "data/data_exception.h"
#include "data/data_manager.h"

// Used to store data as its being parsed
std::string current_line_buffer;
std::string l_value;

// Keep track of line and char for better errors
unsigned int line_number = 1;
unsigned int char_number = 1;

bool in_l_val = false;  // Left of equals sign
bool in_r_val = false;  // Right of equals sign


/**
 * Call this when entering a new line to reset variables and buffers
 */
void ResetVariables() {
	char_number = 0;

	in_l_val = false;
	in_r_val = false;
	current_line_buffer.clear();
}

/**
 * Logs parsing error message and throws
 * @exception Data_exception Thrown when this function is called
 */
void ParseError(const std::string& message) {
	std::stringstream str_s;
	str_s << "Localization parse failed " << line_number << ":" << char_number << "\n" << message;
	LOG_MESSAGE_f(error, "%s", str_s.str().c_str());

	throw jactorio::data::DataException(str_s.str().c_str());
}


/**
 * Helper for parse, handles end of line actions
 */
void ParseEol(jactorio::data::DataManager& data_manager, const std::string& directory_prefix) {
	using namespace jactorio;

	// R val was not specified or empty
	if (in_l_val && !in_r_val)
		ParseError("expected '=' to switch to r-value, got newline");
	else if (in_r_val && current_line_buffer.empty())
		ParseError("expected r-value, got newline");

	// Have not entered l value yet
	if (!in_l_val)
		return;

	// Save R val, reset for next line

	// Generate internal name to search for
	std::stringstream str_s;
	str_s << "__" << directory_prefix << "__/" << l_value;

	bool found = false;
	for (auto& category : data_manager.dataRaw) {
		for (auto& prototype : category) {
			if (prototype.first == str_s.str()) {
				found = true;
				prototype.second->SetLocalizedName(current_line_buffer);

				LOG_MESSAGE_f(debug, "Registered local '%s' '%s'", str_s.str().c_str(), current_line_buffer.c_str());
				goto loop_exit;
			}
		}
	}
loop_exit:
	if (!found) {
		LOG_MESSAGE_f(warning, "Local option '%s' missing matching prototype internal name", str_s.str().c_str());
	}

	ResetVariables();
	line_number++;
}

void jactorio::data::LocalParse(DataManager& data_manager, const std::string& file_str, const std::string& directory_prefix) {
	line_number = 1;
	ResetVariables();

	for (char c : file_str) {
		char_number++;

		// Always skip tabs
		if (c == '\t')
			continue;
		// Skip whitespace when not in quotes
		if (c == ' ' && !in_l_val)
			continue;


		// End of a line
		if (c == '\n') {
			ParseEol(data_manager, directory_prefix);
			continue;
		}


		// Enter data if character is not whitespace or = (whitespace checked above)
		if (c != '=')
			in_l_val = true;

			// Already in R-val, encountered =
		else if (in_r_val) {
			ParseError("attempted to switch to r-value with '=' when already in r-value");
		}
			// Is (=), exit data and switch to second data field upon reaching =
		else if (in_l_val) {
			in_l_val = false;
			in_r_val = true;

			// Save L val
			l_value = current_line_buffer;
			current_line_buffer.clear();
			continue;
		}
			// (=) without first defining a l value
		else {
			ParseError("attempted to switch to r-value with '=' without defining a l value");
		}


		// Save characters into buffer
		if (in_l_val)
			current_line_buffer.push_back(c);
	}

	ParseEol(data_manager, directory_prefix);
}

int jactorio::data::LocalParseNoThrow(DataManager& data_manager, const std::string& file_str,
                                      const std::string& directory_prefix) {
	try {
		LocalParse(data_manager, file_str, directory_prefix);
	}
	catch (DataException&) {
		return 1;
	}

	return 0;
}
