#include <iostream>
#include <thread>

#include "jactorio.h"

#include "renderer/render_main.h"
#include "core/filesystem.h"
#include "data/data_manager.h"
#include "core/logger.h"
#include "data/pybind/pybind_manager.h"

int main(int ac, char* av[]) {
	// Initial startup message
	std::cout << JACTORIO_BUILD_TARGET_PLATFORM << "  |  " << BUILD_TYPE << " build, version: " <<
		JACTORIO_VERSION << "\n\n\n";

	jactorio::core::filesystem::set_executing_directory(av[0]);

	{
		// Rendering + logic initialization
		log_message(jactorio::core::logger::info, "Jactorio", "1 - Data stage");

		jactorio::data::pybind_manager::py_interpreter_init();

		jactorio::data::data_manager::load_data(
			jactorio::core::filesystem::resolve_path("~/data")
		);

		std::thread renderer_thread = std::thread(jactorio::renderer::renderer_main);
		renderer_thread.join();
	}


	jactorio::data::pybind_manager::py_interpreter_terminate();

	std::cout << "goodbye!\n";
	return 0;
}
