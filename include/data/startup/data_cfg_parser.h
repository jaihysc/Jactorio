#ifndef DATA_STARTUP_DATA_CFG_PARSER_H
#define DATA_STARTUP_DATA_CFG_PARSER_H

#include <string>

#include "data/data_manager.h"

namespace jactorio
{
	namespace data
	{
		struct Cfg_data_element
		{
			// unique identifier - E.G "grass-1"
			std::string id;
            data_type type;

			// Relative path from executable directory
			std::string path;
		};
		
		struct Cfg_data
		{
			Cfg_data_element* elements;
			unsigned int count;
		};

		// Parses config.cfg files in the data folder
		// TODO maybe this will become a lua interpreter one day, but this fulfills the requirements currently
		class Data_cfg_parser
		{
            // Used to store data as its being parsed
            std::string str_buffer_;

            // Keep track of line and char for better errors
            unsigned int line_number_ = 1;
            unsigned int char_number_ = 1;

            bool in_data_ = false;
            bool in_second_field_ = false;
            bool has_parsed_data_type_ = false;

			void log_error_message(const std::string& message);
			void trim_trailing_whitespace(std::string& str);

			/**
			 * Moves index to the next newline character, also calls reset_variables()
			 */
			void forward_to_next_newline(unsigned& index, const std::string& file_contents);

			/**
			 * Call this when entering a new line to reset variables and buffers
			 */
            void reset_variables();

			/**
			 * Converts string of a cfg_data_type into a enum
			 */
            [[nodiscard]] data_type parse_cfg_data_type() const;
			
		public:

			/**
			 * Parses a .cfg file found in the data/x folder <br>
			 * cfg_data.elements must be manually freed
			 *
			 * @param cfg_file_contents
			 * 
			 * @param current_directory_name The name of the directory where the cfg file is found,
			 * Used to generate the relative path
			 * 
			 * @return Data extracted from cfg_file_contents, paths will be relative paths (E.g ~/data/x/y/z)
			 */
			Cfg_data parse(std::string cfg_file_contents, const std::string& current_directory_name);
		};
	}
}

#endif // DATA_STARTUP_DATA_CFG_PARSER_H
