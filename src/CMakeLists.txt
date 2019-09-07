# CMakeList.txt : CMake project for jactorio, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)


# Add source to this project's executable.
add_executable (
	jactorio 
	"jactorio.cpp"

	"renderer/initialize.cpp"
)

# Headers include folder
target_include_directories (jactorio PRIVATE ${PROJECT_SOURCE_DIR}/include)

# GLFW for OpenGL
target_include_directories (jactorio PRIVATE ${PROJECT_SOURCE_DIR}/lib/glfw3/include)

target_link_libraries (jactorio ${PROJECT_SOURCE_DIR}/lib/glfw3/lib/glfw3.lib ${PROJECT_SOURCE_DIR}/lib/glfw3/lib/glfw3dll.lib)
target_link_libraries (jactorio opengl32.lib user32.lib gdi32.lib shell32.lib)  # Additional GLFW dependencies


# TODO: Add tests and install targets if needed.
