#ifndef DATA_LOCAL_PARSER_H
#define DATA_LOCAL_PARSER_H

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
#define KEY_DEF( identifier, name )  identifier
#define KEYS_DEF \
    KEY_DEF( en, "en" ), \
    KEY_DEF( fr, "fr" ), \
    KEY_DEF( ge, "ge" )
	
	/**
	 * Enum below lists supported languages, for the identifier string, use: language_identifier
	 */
	enum language { KEYS_DEF };

#undef KEY_DEF
#define KEY_DEF( identifier, name )  { name }

	char const* const language_identifier[] = { KEYS_DEF };
	
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
	int parse_s(const std::string& file_str, const std::string& directory_prefix) noexcept;
}

#endif // DATA_LOCAL_PARSER_H
