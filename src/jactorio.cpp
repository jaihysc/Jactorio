﻿// jactorio.cpp : Defines the entry point for the application.
//

#include <iostream>
#include "renderer/render_main.h"

#include "jactorio.h"
#include "core/file_system.h"

int main(int ac, char* av[])
{
	// Initial startup message
	std::cout << JACTORIO_BUILD_TARGET_PLATFORM << "  |  " << GET_BUILD_TYPE << " build, version: " << JACTORIO_VERSION << "\n\n\n";

	jactorio::core::File_system::set_executing_directory(av[0]);
	
	// Currently a sample openGL program
	renderer_main();

	std::cout << "goodbye!\n";
	return 0;
}
