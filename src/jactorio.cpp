#include "jactorio.h"

#include <thread>

#include "core/filesystem.h"
#include "data/data_manager.h"
#include "game/logic_loop.h"
#include "renderer/render_main.h"

void initialize_game() {
	using namespace jactorio;

	// Rendering + logic initialization
	LOG_MESSAGE(info, "1 - Data stage");

	core::Resource_guard data_manager_guard(&data::data_manager::clear_data);

	std::mutex m;
	std::thread logic_thread = std::thread(game::init_logic_loop, &m);
	std::thread renderer_thread = std::thread(renderer::render_init, &m);

	logic_thread.join();
	renderer_thread.join();
}

int main(int ac, char* av[]) {
	using namespace jactorio;

	core::filesystem::set_executing_directory(av[0]);

	// Log file
	core::Resource_guard log_guard(&core::logger::close_log_file);
	core::logger::open_log_file("~/log.txt");
	
	// Initial startup message
	LOG_MESSAGE_f(none, "%s | %s build, version: %s\n\n", 
	              JACTORIO_BUILD_TARGET_PLATFORM, BUILD_TYPE, JACTORIO_VERSION)

	initialize_game();
	
	LOG_MESSAGE(none, "goodbye!");
	return 0;
}
