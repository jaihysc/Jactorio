// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/15/2019

#include "jactorio.h"

#include <thread>

#include "core/filesystem.h"
#include "game/logic_loop.h"
#include "renderer/render_main.h"

void InitializeGame() {
	using namespace jactorio;

	// Rendering + logic initialization
	LOG_MESSAGE(info, "1 - Data stage");

	std::thread logic_thread    = std::thread(game::InitLogicLoop);
	std::thread renderer_thread = std::thread(renderer::RenderInit);

	logic_thread.join();
	renderer_thread.join();
}

///
/// ENTRY POINT
/// 
int main(int ac, char* av[]) {
	using namespace jactorio;

	core::SetExecutingDirectory(av[0]);

	// Log file
	core::ResourceGuard log_guard(&core::CloseLogFile);
	core::OpenLogFile("~/log.txt");

	// Initial startup message
	LOG_MESSAGE_f(none, "%s | %s build, version: %s\n\n",
	              JACTORIO_BUILD_TARGET_PLATFORM, BUILD_TYPE, JACTORIO_VERSION)

	InitializeGame();

	LOG_MESSAGE(none, "goodbye!");
	return 0;
}
