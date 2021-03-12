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

function(jactorio_copy_runtime_files)
	# Copies from data/ folder to output folder
	set(JACTORIO_DATA_FOLDER data)

	set(COPY_DESTINATION_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
	log_msg("Copying runtime files to: ${COPY_DESTINATION_DIR}")

	file(GLOB_RECURSE
		DATA_FILES
		RELATIVE ${PROJECT_SOURCE_DIR}
		${PROJECT_SOURCE_DIR}/${JACTORIO_DATA_FOLDER}/*
	)

	foreach(DATA_FILE ${DATA_FILES})
		configure_file(${PROJECT_SOURCE_DIR}/${DATA_FILE} ${COPY_DESTINATION_DIR}/${DATA_FILE} COPYONLY)
	endforeach(DATA_FILE)



	# Copy Python embed files over to executable directory. User does not need to download anything
	if (WIN32)
		set(GLOB_EXPRESSIONS
			python*.zip
			python*.dll
			vcruntime*.dll
		)

		foreach (GLOB_EXPRESSION ${GLOB_EXPRESSIONS})
			file(GLOB 
				 GLOBBED_FILES
				 RELATIVE ${Python_EMBED_DIR}
				 ${Python_EMBED_DIR}/${GLOB_EXPRESSION}
			)

			foreach(FILE ${GLOBBED_FILES})
				configure_file(${Python_EMBED_DIR}/${FILE} ${COPY_DESTINATION_DIR}/${FILE} COPYONLY)
			endforeach()
		endforeach ()
	endif()


	# Save Python versio used to compile, thus user need to download if not on Windows
	if (DEFINED Python_VERSION)
		file(WRITE ${COPY_DESTINATION_DIR}/python_version ${Python_VERSION})
	else()
		message(FATAL_ERROR "No Python version set")
	endif()


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


# Find Python for pybind11 and writing Python version
find_package(Python COMPONENTS Interpreter Development REQUIRED)

# Download python embed for Windows, Windows users do not need to manually install Python
if (WIN32)
	log_msg("Download Python ${Python_VERSION}")

	set(Python_EMBED_DIR ${CMAKE_BINARY_DIR}/python-embed/${Python_VERSION}/)

	file(MAKE_DIRECTORY ${Python_EMBED_DIR})
	file(DOWNLOAD
		 "https://www.python.org/ftp/python/${Python_VERSION}/python-${Python_VERSION}-embed-amd64.zip"
		 ${Python_EMBED_DIR}/embed-download.zip
		 TLS_VERIFY ON
	)
	execute_process(
		COMMAND ${CMAKE_COMMAND} -E tar xzf ${Python_EMBED_DIR}/embed-download.zip 
		WORKING_DIRECTORY ${Python_EMBED_DIR}
	)
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
