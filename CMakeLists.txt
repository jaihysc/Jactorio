#
# Targets:
# jactorioLib  | Lib, all library files used
# jactorioBase | Lib, all files excluding jactorio.cpp
#
# jactorio     | Executable
# jactorioTest | Google test Executable
#
cmake_minimum_required(VERSION 3.9)
include(CheckIPOSupported)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_POLICY_DEFAULT_CMP0069 NEW) # Enable IPO for all


project("jactorio")

option(JACTORIO_BUILD_TESTS "JACTORIO_BUILD_TESTS")





function(log_msg message)
	message(" - ${message}")
endfunction()

# Shows properties for a subdirectory
function(jactorio_show_properties)
	log_msg("Binary directory: ${CMAKE_CURRENT_BINARY_DIR}")
endfunction()

# Copies from data/ folder to output folder
function(jactorio_copy_data_files)
	set(JACTORIO_DATA_FOLDER data)

	log_msg("Copying files to output directory: ${CMAKE_CURRENT_BINARY_DIR}/${JACTORIO_DATA_FOLDER}")

	file(GLOB_RECURSE
		DATA_FILES
		RELATIVE ${PROJECT_SOURCE_DIR}
		${PROJECT_SOURCE_DIR}/${JACTORIO_DATA_FOLDER}/*
	)

	foreach(DATA_FILE ${DATA_FILES})
		configure_file(${PROJECT_SOURCE_DIR}/${DATA_FILE} ${DATA_FILE} COPYONLY)
	endforeach(DATA_FILE)

	log_msg("DONE Copying files to output directory")
endfunction()





log_msg("Target: ${CMAKE_SYSTEM_NAME}")


IF(CMAKE_BUILD_TYPE MATCHES Debug)
	log_msg("Type: Debug")
    add_compile_definitions(JACTORIO_DEBUG_BUILD)  # Break on openGL error

ELSEIF(CMAKE_BUILD_TYPE MATCHES Release OR CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
	log_msg("Type: <<< Release >>>")
	check_ipo_supported(RESULT result)

	if(result)
        log_msg("IPO: Available, Enabled")
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
	else()
		log_msg("IPO: ~Unavailable")
	endif()
ELSE()
  message(FATAL_ERROR "Unknown build type, specify with -D CMAKE_BUILD_TYPE=")
ENDIF()





# Ensure OpenGL exists
find_package(OpenGL REQUIRED)

if (OPENGL_FOUND)
	log_msg("Found OpenGL")
	log_msg("    OpenGL include dir   : ${OPENGL_INCLUDE_DIR}")
	log_msg("    OpenGL link libraries: ${OPENGL_gl_LIBRARY}")
else (OPENGL_FOUND)
	message(FATAL_ERROR "OpenGL not found")
endif()





# Dependencies build arguments
add_compile_definitions(GLEW_STATIC)

add_subdirectory("lib")

# ======================================== Defines
add_compile_definitions(JACTORIO_BUILD_TARGET_PLATFORM="${CMAKE_SYSTEM_NAME}")

# ======================================== Options
add_compile_definitions(JACTORIO_VERSION="0.1.0")
add_compile_definitions(JACTORIO_LOG_LEVEL=0)
# 0 - Debug, 1 - Info, 2 - Warning, 3 - Error, 4 - Critical, 5 - None

enable_testing()

add_subdirectory("src")
if (JACTORIO_BUILD_TESTS)
	add_subdirectory("test")
endif ()
