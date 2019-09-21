# CMakeList.txt : CMake project for jactorio TESTS, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

####################
# Google Test
# Download and unpack googletest at configure time
configure_file(${PROJECT_SOURCE_DIR}/test/CMakeLists.txt.in googletest-download/CMakeLists.txt)

execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )

if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()

execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )

if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set (gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
  include_directories("${gtest_SOURCE_DIR}/include")
endif()


####################
# Add source to this project's executable.

# From jactorio
set (JACTORIO_DIR ${PROJECT_SOURCE_DIR}/src/)

set (
	IMGUI_FILES 
	${PROJECT_SOURCE_DIR}/lib/imgui/imgui/imgui.cpp
	${PROJECT_SOURCE_DIR}/lib/imgui/imgui/imgui_demo.cpp
	${PROJECT_SOURCE_DIR}/lib/imgui/imgui/imgui_draw.cpp
	${PROJECT_SOURCE_DIR}/lib/imgui/imgui/imgui_widgets.cpp
)

add_executable (
	jactorioTest

	${IMGUI_FILES}

	${JACTORIO_DIR}core/logger.cpp
	${JACTORIO_DIR}core/file_system/read.cpp
	
	${JACTORIO_DIR}renderer/camera.cpp
	${JACTORIO_DIR}renderer/render_main.cpp
	${JACTORIO_DIR}renderer/texture.cpp

	${JACTORIO_DIR}renderer/gui/imgui_manager.cpp
	${JACTORIO_DIR}renderer/gui/imgui_glfw.cpp
	${JACTORIO_DIR}renderer/gui/imgui_opengl3.cpp

	${JACTORIO_DIR}renderer/opengl/renderer.cpp
	${JACTORIO_DIR}renderer/opengl/error.cpp
	${JACTORIO_DIR}renderer/opengl/shader.cpp
	${JACTORIO_DIR}renderer/opengl/shader_manager.cpp
	${JACTORIO_DIR}renderer/opengl/index_buffer.cpp
	${JACTORIO_DIR}renderer/opengl/vertex_buffer.cpp
	${JACTORIO_DIR}renderer/opengl/vertex_array.cpp
	${JACTORIO_DIR}renderer/opengl/window_manager.cpp
	
	"jactorioTests.cpp"

	"core/file_systemTests.cpp"
	"core/loggerTests.cpp"
	
	"renderer/openglTests.cpp"
	"renderer/shader_managerTests.cpp"
)

# Headers include folder
target_include_directories (jactorioTest PRIVATE ${PROJECT_SOURCE_DIR}/include)

# GLFW for OpenGL
target_include_directories (jactorioTest PRIVATE ${PROJECT_SOURCE_DIR}/lib/glfw3/include)

target_link_libraries (jactorioTest ${PROJECT_SOURCE_DIR}/lib/glfw3/lib/glfw3.lib)
target_link_libraries (jactorioTest opengl32.lib user32.lib gdi32.lib shell32.lib)  # Additional GLFW dependencies

# GLEW
add_compile_definitions (GLEW_STATIC)  # Indicates static linking

target_include_directories (jactorioTest PRIVATE ${PROJECT_SOURCE_DIR}/lib/glew-2.1.0/include)
target_link_libraries (jactorioTest ${PROJECT_SOURCE_DIR}/lib/glew-2.1.0/lib/Release/x64/glew32s.lib)
# stb
target_include_directories (jactorioTest PRIVATE ${PROJECT_SOURCE_DIR}/lib/stb/)
# glm
target_include_directories (jactorioTest PRIVATE ${PROJECT_SOURCE_DIR}/lib/glm/)
# imgui
target_include_directories (jactorioTest PRIVATE ${PROJECT_SOURCE_DIR}/lib/imgui/)

# GTest
target_link_libraries(jactorioTest gtest_main)

# add_test(NAME example_test COMMAND example)
