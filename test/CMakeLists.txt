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
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

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
file(GLOB_RECURSE JACTORIO_SRC_FILES 
	${PROJECT_SOURCE_DIR}/src/game/*.cpp
	${PROJECT_SOURCE_DIR}/src/renderer/*.cpp
)

# jactorioTests .cpp files
add_executable (
	jactorioTest
	"jactorioTests.cpp"

	${JACTORIO_SRC_FILES}
)

# Headers include folder
target_include_directories (jactorioTest PRIVATE ${PROJECT_SOURCE_DIR}/include)

# GLFW for OpenGL
target_include_directories (jactorioTest PRIVATE ${PROJECT_SOURCE_DIR}/lib/glfw3/include)

target_link_libraries (jactorioTest ${PROJECT_SOURCE_DIR}/lib/glfw3/lib/glfw3.lib ${PROJECT_SOURCE_DIR}/lib/glfw3/lib/glfw3dll.lib)
target_link_libraries (jactorioTest opengl32.lib user32.lib gdi32.lib shell32.lib)  # Additional GLFW dependencies


# GTest
target_link_libraries(jactorioTest gtest_main)

# add_test(NAME example_test COMMAND example)
