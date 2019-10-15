#include "data/startup/data_cfg_parser.h"

#include <vector>
#include <sstream>

#include "core/logger.h"

namespace logger = jactorio::core::logger;

void jactorio::data::Data_cfg_parser::log_error_message(
	const std::string& message) {
	std::stringstream str_s;

	str_s << line_number_ << ":" << char_number_ << " " << message << "\n" <<
		"\tBuffer contents: " << str_buffer_;

	log_message(logger::error,
	            "Data cfg parser", str_s.str());

}

void jactorio::data::Data_cfg_parser::
trim_trailing_whitespace(std::string& str) {
	const unsigned int end = str.find_last_not_of(' ') + 1;

	// Index of last character + 1 to become length
	str = str.substr(0, end);
}

void jactorio::data::Data_cfg_parser::forward_to_next_newline(
	unsigned& index, const std::string& file_contents) {
	while (index < file_contents.size() && file_contents[index] != '\n')
		index++;

	line_number_++;
	reset_variables();
}

void jactorio::data::Data_cfg_parser::reset_variables() {
	char_number_ = 0;

	in_data_ = false;
	in_second_field_ = false;
	has_parsed_data_type_ = false;
	str_buffer_.clear();
}


jactorio::data::data_type jactorio::data::Data_cfg_parser::
parse_cfg_data_type() const {
	if (str_buffer_ == "graphics")
		return data_type::graphics;
	if (str_buffer_ == "audio")
		return data_type::audio;

	return data_type::none;
}

jactorio::data::Cfg_data jactorio::data::Data_cfg_parser::parse(
	std::string cfg_file_contents,
	const std::string& current_directory_name) {
	// Input format:
	// path to file = internal-file-id
	// There should be no leading or trailing whitespace as it will be trimmed
	// 
	// Beginning of path of file determines data type
	//		graphics
	//		audio

	// Add ending newline character if it is missing one
	if (cfg_file_contents.back() != '\n')
		cfg_file_contents.push_back('\n');

	// Parsed from current line, not yet validated
	data_type parse_data_type;
	std::string parse_path;

	// Data of confirmed valid entries
	std::vector<data_type> data_types;
	std::vector<std::string> paths;
	std::vector<std::string> ids;

	for (unsigned int i = 0; i < cfg_file_contents.size(); ++i) {
		const char c = cfg_file_contents[i];
		char_number_++;

		// Always skip tabs
		if (c == '\t')
			continue;

		// End of a line of data
		if (c == '\n') {
			// Error checking - Discard the current line and data if there is an error
			bool error_occurred = false;
			if (!in_second_field_) {
				// Only warn if in data in case someone decides to leave whitespace
				if (in_data_)
					log_error_message(
						"Expected field \"internal name\", got newline");
				error_occurred = true;
			}
			else if (in_second_field_ && str_buffer_.size() <= 0) {
				log_error_message("Field \"internal name\" was empty");
				error_occurred = true;
			}

			if (!error_occurred) {
				trim_trailing_whitespace(str_buffer_);
				// Save data
				data_types.push_back(parse_data_type);
				paths.push_back(parse_path);
				ids.push_back(str_buffer_);
			}

			reset_variables();
			line_number_++;
			continue;
		}

		if (c == ' ') {
			// Skip whitespace when not in quotes
			if (!in_data_)
				continue;

			if (in_second_field_) {
				log_error_message(
					"Field \"internal name\" should not have any whitespace");
				forward_to_next_newline(i, cfg_file_contents);
				continue;
			}
		}

		// Enter data if character is not a whitespace or = (whitespace checked above)
		if (c != '=')
			in_data_ = true;
			// Exit data and switch to second data field upon reaching =
		else if (in_data_) {
			in_data_ = false;
			in_second_field_ = true;

			// Save path (1)
			trim_trailing_whitespace(str_buffer_);

			// Generate relative path
			parse_path = "~/data/";
			parse_path += current_directory_name;
			parse_path += "/";
			parse_path += str_buffer_;

			str_buffer_.clear();
		}

		// Save characters into buffer
		if (in_data_) {
			// Parse the data type based on the first folder name
			if (!has_parsed_data_type_ && !in_second_field_ &&
				c == '/') {
				has_parsed_data_type_ = true;

				parse_data_type = parse_cfg_data_type();
				if (parse_data_type == data_type::none) {
					log_error_message("Data type folder is invalid");
					forward_to_next_newline(i, cfg_file_contents);
					continue;
				}
			}
			str_buffer_.push_back(c);
		}
	}

	Cfg_data cfg_data;
	cfg_data.elements = new Cfg_data_element[paths.size()];
	cfg_data.count = paths.size();

	// Populate cfg_data elements with extracted data
	for (unsigned int i = 0; i < paths.size(); ++i) {
		cfg_data.elements[i].type = data_types[i];
		cfg_data.elements[i].path = paths[i];
		cfg_data.elements[i].id = ids[i];
	}

	return cfg_data;
}
