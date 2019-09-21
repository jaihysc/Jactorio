# CMakeList.txt : CMake project for jactorio, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)


# Copy data executable directory
file (COPY ${PROJECT_SOURCE_DIR}/data DESTINATION / )
# Shaders
configure_file(${PROJECT_SOURCE_DIR}/data/core/shaders/fs.frag data/core/shaders/fs.frag COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/data/core/shaders/vs.vert data/core/shaders/vs.vert COPYONLY)


set (
	IMGUI_FILES 
	${PROJECT_SOURCE_DIR}/lib/imgui/imgui/imgui.cpp
	${PROJECT_SOURCE_DIR}/lib/imgui/imgui/imgui_demo.cpp
	${PROJECT_SOURCE_DIR}/lib/imgui/imgui/imgui_draw.cpp
	${PROJECT_SOURCE_DIR}/lib/imgui/imgui/imgui_widgets.cpp
)

# Add source to this project's executable.
add_executable (
	jactorio 
	"jactorio.cpp"
	${IMGUI_FILES}

	"core/logger.cpp"
	"core/file_system/read.cpp"

	"renderer/camera.cpp"
	"renderer/render_main.cpp"
	"renderer/texture.cpp"

	"renderer/gui/imgui_manager.cpp"
	"renderer/gui/imgui_glfw.cpp"
	"renderer/gui/imgui_opengl3.cpp"

	"renderer/opengl/renderer.cpp"
	"renderer/opengl/error.cpp"
	"renderer/opengl/index_buffer.cpp"
	"renderer/opengl/shader.cpp"
	"renderer/opengl/shader_manager.cpp"
	"renderer/opengl/vertex_array.cpp"
	"renderer/opengl/vertex_buffer.cpp"
	"renderer/opengl/window_manager.cpp"
)


# Headers include folder
target_include_directories (jactorio PRIVATE ${PROJECT_SOURCE_DIR}/include)

# Additional libraries to be placed in lib/

# GLFW for OpenGL
target_include_directories (jactorio PRIVATE ${PROJECT_SOURCE_DIR}/lib/glfw3/include)

target_link_libraries (jactorio ${PROJECT_SOURCE_DIR}/lib/glfw3/lib/glfw3.lib)
target_link_libraries (jactorio opengl32.lib user32.lib gdi32.lib shell32.lib)  # Additional GLFW dependencies

# GLEW
add_compile_definitions (GLEW_STATIC)  # Indicates static linking

target_include_directories (jactorio PRIVATE ${PROJECT_SOURCE_DIR}/lib/glew-2.1.0/include)
target_link_libraries (jactorio ${PROJECT_SOURCE_DIR}/lib/glew-2.1.0/lib/Release/x64/glew32s.lib)
# stb
target_include_directories (jactorio PRIVATE ${PROJECT_SOURCE_DIR}/lib/stb/)
# glm
target_include_directories (jactorio PRIVATE ${PROJECT_SOURCE_DIR}/lib/glm/)
# imgui
target_include_directories (jactorio PRIVATE ${PROJECT_SOURCE_DIR}/lib/imgui/)


IF(CMAKE_BUILD_TYPE MATCHES Debug)
	message("---------- Debug build ----------")
    add_compile_definitions(JACTORIO_DEBUG_BUILD)  # Break on openGL error

ELSEIF(CMAKE_BUILD_TYPE MATCHES Release)
	message("<<<<<<<<<< Release build >>>>>>>>>>")

ELSE()
  message(FATAL_ERROR "Unknown build type")
ENDIF()