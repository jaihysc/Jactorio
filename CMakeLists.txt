# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project("jactorio")

option(JACTORIO_BUILD_TESTS "JACTORIO_BUILD_TESTS")

add_compile_definitions(JACTORIO_VERSION="0.1.0")
add_compile_definitions(JACTORIO_BUILD_TARGET_PLATFORM="${CMAKE_SYSTEM_NAME}")

############# Include folders .h
set(JACTORIO_INCLUDE_FILES
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/lib/glfw/include
	${PROJECT_SOURCE_DIR}/lib/glew-cmake/include
	${PROJECT_SOURCE_DIR}/lib/stb/
	${PROJECT_SOURCE_DIR}/lib/glm/
	${PROJECT_SOURCE_DIR}/lib/imgui/
	${PROJECT_SOURCE_DIR}/lib/SFML/include
	${PROJECT_SOURCE_DIR}/lib/pybind11/include
	${PROJECT_SOURCE_DIR}/lib/libnoise/include
)
#------------ Source files .cpp
set(JACTORIO_DIR ${PROJECT_SOURCE_DIR}/src)
set(JACTORIO_SRC_FILES 
	${JACTORIO_DIR}/core/logger.cpp
	${JACTORIO_DIR}/core/filesystem.cpp
	${JACTORIO_DIR}/core/loop_manager.cpp
	${JACTORIO_DIR}/core/debug/execution_timer.cpp

	${JACTORIO_DIR}/data/prototype/noise_layer.cpp

	${JACTORIO_DIR}/data/pybind/pybind_manager.cpp
	${JACTORIO_DIR}/data/data_manager.cpp

	${JACTORIO_DIR}/game/logic_loop.cpp
	${JACTORIO_DIR}/game/input/input_manager.cpp
	${JACTORIO_DIR}/game/world/chunk.cpp
	${JACTORIO_DIR}/game/world/tile.cpp
	${JACTORIO_DIR}/game/world/world_manager.cpp
	${JACTORIO_DIR}/game/world/world_generator.cpp

	${JACTORIO_DIR}/renderer/render_main.cpp
	${JACTORIO_DIR}/renderer/render_loop.cpp

	${JACTORIO_DIR}/renderer/gui/imgui_manager.cpp
	${JACTORIO_DIR}/renderer/gui/imgui_glfw.cpp
	${JACTORIO_DIR}/renderer/gui/imgui_opengl3.cpp

	${JACTORIO_DIR}/renderer/opengl/error.cpp
	${JACTORIO_DIR}/renderer/opengl/index_buffer.cpp
	${JACTORIO_DIR}/renderer/opengl/shader_manager.cpp
	${JACTORIO_DIR}/renderer/opengl/shader.cpp
	${JACTORIO_DIR}/renderer/opengl/texture.cpp
	${JACTORIO_DIR}/renderer/opengl/vertex_array.cpp
	${JACTORIO_DIR}/renderer/opengl/vertex_buffer.cpp

	${JACTORIO_DIR}/renderer/rendering/renderer_manager.cpp
	${JACTORIO_DIR}/renderer/rendering/world_renderer.cpp
	${JACTORIO_DIR}/renderer/rendering/mvp_manager.cpp
	${JACTORIO_DIR}/renderer/rendering/renderer.cpp
	${JACTORIO_DIR}/renderer/rendering/renderer_grid.cpp
	${JACTORIO_DIR}/renderer/rendering/spritemap_generator.cpp

	${JACTORIO_DIR}/renderer/window/window_manager.cpp
)

# >>>>>>>>>>> Test files .cpp
set(JACTORIO_TEST_DIR ${PROJECT_SOURCE_DIR}/test)
set(JACTORIO_TEST_FILES 
	${JACTORIO_TEST_DIR}/jactorioTests.cpp

	${JACTORIO_TEST_DIR}/core/file_systemTests.cpp
	${JACTORIO_TEST_DIR}/core/loggerTests.cpp
	${JACTORIO_TEST_DIR}/core/loop_managerTests.cpp
	${JACTORIO_TEST_DIR}/core/resource_guardTests.cpp

	${JACTORIO_TEST_DIR}/data/data_managerTests.cpp
	${JACTORIO_TEST_DIR}/data/pybind/pybind_managerTests.cpp

	${JACTORIO_TEST_DIR}/data/prototype/noise_layerTests.cpp
	${JACTORIO_TEST_DIR}/data/prototype/spriteTests.cpp

	${JACTORIO_TEST_DIR}/game/input/input_managerTests.cpp
	${JACTORIO_TEST_DIR}/game/world/chunkTests.cpp
	${JACTORIO_TEST_DIR}/game/world/world_managerTests.cpp

	${JACTORIO_TEST_DIR}/renderer/manager/mvp_managerTests.cpp
	${JACTORIO_TEST_DIR}/renderer/manager/renderer_managerTests.cpp

	${JACTORIO_TEST_DIR}/renderer/rendering/spritemap_generatorTests.cpp
	${JACTORIO_TEST_DIR}/renderer/rendering/renderer_gridTests.cpp

	${JACTORIO_TEST_DIR}/renderer/openglTests.cpp
)


############# Dependencies #############

function(validate_dependency DEPENDENCY_PATH)
	file(GLOB RESULT ${CMAKE_SOURCE_DIR}/${DEPENDENCY_PATH}/*)
	list(LENGTH RESULT RES_LEN)

    if((NOT EXISTS ${CMAKE_SOURCE_DIR}/${DEPENDENCY_PATH}) OR (RES_LEN EQUAL 0))
		message("Installing missing dependency ${DEPENDENCY_PATH}")

		execute_process(COMMAND git submodule update --init -- ${DEPENDENCY_PATH}
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    else()
		message("Found dependency ${DEPENDENCY_PATH}")
    endif()
endfunction()

message("========== Resolving dependencies...")
validate_dependency(lib/glew-cmake)
validate_dependency(lib/glfw)
validate_dependency(lib/stb/stb)	# HEADER
validate_dependency(lib/glm)		# HEADER
validate_dependency(lib/imgui/imgui)	# NC
validate_dependency(lib/SFML)
validate_dependency(lib/pybind11)
validate_dependency(lib/libnoise)


# Dependencies build arguments
add_compile_definitions(GLEW_STATIC)
set(SFML_STATIC_LIBRARIES TRUE)

# Ones which need to be built
add_subdirectory("lib/glew-cmake")
add_subdirectory("lib/glfw")
add_subdirectory("lib/SFML")
add_subdirectory("lib/pybind11")
add_subdirectory("lib/libnoise")



find_package(SFML 2.5 COMPONENTS graphics)


set(
	IMGUI_FILES 
	${PROJECT_SOURCE_DIR}/lib/imgui/imgui/imgui.cpp
	${PROJECT_SOURCE_DIR}/lib/imgui/imgui/imgui_demo.cpp
	${PROJECT_SOURCE_DIR}/lib/imgui/imgui/imgui_draw.cpp
	${PROJECT_SOURCE_DIR}/lib/imgui/imgui/imgui_widgets.cpp
)

# Additional files to compile
set(
	ADDITIONAL_BUILD_FILES
	${IMGUI_FILES}
)

# Platform link libraries
set(
	LIBS_WIN

	glfw libglew_static
	sfml-graphics
	glu32.lib opengl32.lib user32.lib gdi32.lib shell32.lib
	pybind11::embed
	libnoise
)
set(
	LIBS_UNIX

	glfw libglew_static
	sfml-graphics
	libGLU.so libGL.so
	pybind11::embed
	libnoise
)


message("========== Jactorio files")

add_subdirectory("src")
if (JACTORIO_BUILD_TESTS)
	add_subdirectory("test")
endif ()
