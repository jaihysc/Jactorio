# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("jactorio")

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
# Jactorio test and src directories
add_subdirectory ("src")
add_subdirectory ("test")
