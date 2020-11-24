# Includes dependencies
log_msg("")
log_msg("[ Jactorio lib ]")

function(validate_dependency DEPENDENCY_PATH)
	file(GLOB RESULT ${CMAKE_SOURCE_DIR}/lib/${DEPENDENCY_PATH}/*)
	list(LENGTH RESULT RES_LEN)

	if((NOT EXISTS ${CMAKE_SOURCE_DIR}/lib/${DEPENDENCY_PATH}) OR (RES_LEN EQUAL 0))
		log_msg("Installing missing dependency ${DEPENDENCY_PATH}")

		execute_process(COMMAND git submodule update --init --recursive -- lib/${DEPENDENCY_PATH}
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
	else()
		log_msg("Found dependency: ${DEPENDENCY_PATH}")
	endif()
endfunction()





log_msg("Resolving dependencies...")
validate_dependency(glew-cmake)
validate_dependency(stb/stb)	# HEADER
validate_dependency(glm)		# HEADER
validate_dependency(imgui)	# NC
validate_dependency(pybind11)
validate_dependency(libnoise)
validate_dependency(SDL-mirror)
validate_dependency(decimal_for_cpp)
validate_dependency(StackWalker)
validate_dependency(backward-cpp)
validate_dependency(cereal)


# SDL, fixes unresolved symbols to memset, memcpy, __chkstk when using /GL (Whole program optimization)
if (MSVC)
	list(APPEND EXTRA_LIBS vcruntime msvcrt)
endif()


# Ones which need to be built
add_subdirectory("glew-cmake")
add_subdirectory("pybind11")
add_subdirectory("libnoise")
add_subdirectory("SDL-mirror")



set(JACTORIO_LIB_DIR ${PROJECT_SOURCE_DIR}/lib)
add_library(jactorioLib
		${JACTORIO_LIB_DIR}/null.cpp  # CMake requires at least 1 cpp file
)


# Platform link libraries
set(
	LIBS_COMMON

	libglew_static
	pybind11::embed
	libnoise
	SDL2main
	SDL2-static
)

if (WIN32)
	log_msg("Link Target: Windows")
	set(
		JACTORIO_LINK_LIBS

		${LIBS_COMMON}
		glu32.lib opengl32.lib user32.lib gdi32.lib shell32.lib
	)
elseif (APPLE)
	log_msg("Link Target: Apple")
    set(
		JACTORIO_LINK_LIBS

		${LIBS_COMMON}
		"-framework OpenGL"
	)
elseif (UNIX)
	log_msg("Link Target: UNIX (Not Apple)")
    set(
		JACTORIO_LINK_LIBS

		${LIBS_COMMON}
		libGLU.so libGL.so
	)

	# For backward-cpp
	find_library(LIBDW_DEV dw)
	if(LIBDW_DEV)
		set(
			JACTORIO_LINK_LIBS
			${JACTORIO_LINK_LIBS}
			dw
		)
		target_compile_definitions(jactorioLib PUBLIC BACKWARD_HAS_DW=1)
	endif()
endif (WIN32)


# ======================================== Lib HEADER files .h
set(JACTORIO_LIB_HEADER_FILES
	${PROJECT_SOURCE_DIR}/lib/glew-cmake/include
	${PROJECT_SOURCE_DIR}/lib/stb/
	${PROJECT_SOURCE_DIR}/lib/glm/
	${PROJECT_SOURCE_DIR}/lib/imgui/
	${PROJECT_SOURCE_DIR}/lib/pybind11/include
	${PROJECT_SOURCE_DIR}/lib/libnoise/include
	${PROJECT_SOURCE_DIR}/lib/SDL-mirror/include
	${PROJECT_SOURCE_DIR}/lib/decimal_for_cpp/include
	${PROJECT_SOURCE_DIR}/lib/StackWalker/Main
	${PROJECT_SOURCE_DIR}/lib/backward-cpp
	${PROJECT_SOURCE_DIR}/lib/cereal/include
)
# ======================================== END HEADER files .h

target_link_libraries(jactorioLib ${JACTORIO_LINK_LIBS})
target_include_directories(jactorioLib PUBLIC ${JACTORIO_LIB_HEADER_FILES})

