// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/15/2020

#ifndef JACTORIO_INCLUDE_DATA_LOCAL_PARSER_H
#define JACTORIO_INCLUDE_DATA_LOCAL_PARSER_H
#pragma once

#include <string>
#include <unordered_map>

/**
 * Parses localization files found in data/__name__/local/<lang>.cfg <br>
 *
 * Only the currently selected language will be parsed.
 * Parsed data will be added to data_raw in data_manager
 */
namespace jactorio::data::local_parser
{
	// Define identifiers for supported languages
#define KEYS_DEF \
    KEY_DEF( en, "en" ), \
    KEY_DEF( fr, "fr" ), \
    KEY_DEF( ge, "ge" )

#define KEY_DEF( identifier, name )  identifier

	/**
	 * Supported languages, for the identifier string, use: language_identifier[static_cast<int>(language)]
	 */
	enum class language { KEYS_DEF };
#undef KEY_DEF

#define KEY_DEF( identifier, name )  { name }
	char const* const language_identifier[] = {KEYS_DEF};
#undef KEY_DEF

	/**
	 * Parses a .cfg file, will throw exceptions on error
	 * @param file_str File contents
	 * @param directory_prefix Added in front when searching for internal names : objectA -> __name__/objectA
	 */
	void parse(const std::string& file_str, const std::string& directory_prefix);

	/**
	 * Parses a .cfg file, does not throw
	 * @param file_str File contents
	 * @param directory_prefix Added in front when searching for internal names : objectA -> __name__/objectA
	 * @return non-zero if error occurred
	 */
	int parse_s(const std::string& file_str, const std::string& directory_prefix);
}

#endif //JACTORIO_INCLUDE_DATA_LOCAL_PARSER_H
