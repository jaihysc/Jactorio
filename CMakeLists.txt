# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required(VERSION 3.8)

project("jactorio")

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

# Ones which need to be built
add_subdirectory("lib/glew-cmake")
add_subdirectory("lib/glfw")

message("========== Building jactorio...")
set(
	IMGUI_FILES 
	${PROJECT_SOURCE_DIR}/lib/imgui/imgui/imgui.cpp
	${PROJECT_SOURCE_DIR}/lib/imgui/imgui/imgui_demo.cpp
	${PROJECT_SOURCE_DIR}/lib/imgui/imgui/imgui_draw.cpp
	${PROJECT_SOURCE_DIR}/lib/imgui/imgui/imgui_widgets.cpp
)

set(JACTORIO_DIR ${PROJECT_SOURCE_DIR}/src)
##### Source files
set(JACTORIO_SRC_FILES 
	${IMGUI_FILES}

	${JACTORIO_DIR}/core/logger.cpp
	${JACTORIO_DIR}/core/file_system/read.cpp

	${JACTORIO_DIR}/renderer/camera.cpp
	${JACTORIO_DIR}/renderer/render_main.cpp
	${JACTORIO_DIR}/renderer/texture.cpp

	${JACTORIO_DIR}/renderer/gui/imgui_manager.cpp
	${JACTORIO_DIR}/renderer/gui/imgui_glfw.cpp
	${JACTORIO_DIR}/renderer/gui/imgui_opengl3.cpp

	${JACTORIO_DIR}/renderer/opengl/renderer.cpp
	${JACTORIO_DIR}/renderer/opengl/error.cpp
	${JACTORIO_DIR}/renderer/opengl/index_buffer.cpp
	${JACTORIO_DIR}/renderer/opengl/shader.cpp
	${JACTORIO_DIR}/renderer/opengl/shader_manager.cpp
	${JACTORIO_DIR}/renderer/opengl/vertex_array.cpp
	${JACTORIO_DIR}/renderer/opengl/vertex_buffer.cpp
	${JACTORIO_DIR}/renderer/opengl/window_manager.cpp
)
##### Include folders
set(JACTORIO_INCLUDE_FILES
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/lib/glfw/include
	${PROJECT_SOURCE_DIR}/lib/glew-cmake/include
	${PROJECT_SOURCE_DIR}/lib/stb/
	${PROJECT_SOURCE_DIR}/lib/glm/
	${PROJECT_SOURCE_DIR}/lib/imgui/
)

################## CONFIG    ###############

add_compile_definitions(JACTORIO_VERSION="0.1.0")
add_compile_definitions(JACTORIO_BUILD_TARGET_PLATFORM="${CMAKE_SYSTEM_NAME}")
add_compile_definitions(GLEW_STATIC)  # Indicates static linking

################## ENDCONFIG ###############

add_subdirectory("src")
add_subdirectory("test")
