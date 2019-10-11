#ifndef CORE_FILE_SYSTEM_H
#define CORE_FILE_SYSTEM_H

namespace jactorio
{
	namespace core
	{
		class File_system
		{
			static std::string executing_directory_;

		public:
			static std::string get_executing_directory() {
				return executing_directory_;
			}

			/*!
			 * Replaces ~ at the beginning of the path with the executing directory \n
			 * If there is no ~ at the beginning of the path, nothing happens
			 */
			static std::string resolve_path(const std::string& path);

			/*!
			 * Saves directory \n
			 * Cuts off file names (some/where/executing.exe) becomes (some/where)
			 */
			static void set_executing_directory(const std::string& executing_directory);

			// Reads file from path as string
			// Returns NULL if path is invalid
			static std::string read_file_as_str(const std::string& path);
		};
	}
}

#endif // CORE_FILE_SYSTEM_H