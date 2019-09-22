# CMakeList.txt : CMake project for jactorio, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)


################## CONFIG    ###############

add_compile_definitions(JACTORIO_VERSION="0.1.0")
add_compile_definitions(JACTORIO_BUILD_TARGET_PLATFORM="${CMAKE_SYSTEM_NAME}")

################## ENDCONFIG ###############

IF(CMAKE_BUILD_TYPE MATCHES Debug)
	message("Type: Debug")
    add_compile_definitions(JACTORIO_DEBUG_BUILD)  # Break on openGL error

ELSEIF(CMAKE_BUILD_TYPE MATCHES Release)
	message("Type: <<<<< Release build >>>>>")

ELSE()
  message(FATAL_ERROR "Unknown build type, specify with -D CMAKE_BUILD_TYPE=")
ENDIF()

message("Target: ${CMAKE_SYSTEM_NAME}")


# Copy data executable directory
message(" => Copying files to output directory")
file (COPY ${PROJECT_SOURCE_DIR}/data DESTINATION / )
# Shaders
configure_file(${PROJECT_SOURCE_DIR}/data/core/shaders/fs.frag data/core/shaders/fs.frag COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/data/core/shaders/vs.vert data/core/shaders/vs.vert COPYONLY)

message(" => DONE Copying files to output directory")


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

# OpenGL dependencies
# GLFW
target_include_directories (jactorio PRIVATE ${PROJECT_SOURCE_DIR}/lib/glfw/include)
target_link_libraries (jactorio glfw)

# GLEW
add_compile_definitions (GLEW_STATIC)  # Indicates static linking
target_include_directories (jactorio PRIVATE ${PROJECT_SOURCE_DIR}/lib/glew-cmake/include)

if (WIN32)  # Platform dependent openGL libraries
	target_link_libraries (jactorio libglew_static glu32.lib opengl32.lib user32.lib gdi32.lib shell32.lib)
endif (WIN32)
if (UNIX)
    target_link_libraries (jactorio libglew_static libGLU.so libGL.so)    
endif (UNIX)

# stb
target_include_directories (jactorio PRIVATE ${PROJECT_SOURCE_DIR}/lib/stb/)
# glm
target_include_directories (jactorio PRIVATE ${PROJECT_SOURCE_DIR}/lib/glm/)
# imgui
target_include_directories (jactorio PRIVATE ${PROJECT_SOURCE_DIR}/lib/imgui/)