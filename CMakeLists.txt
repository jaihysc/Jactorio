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

# Add preprocesor definition of FILENAME to pass the filename without directory path
# For each source file depended on by TARGET_NAME, where FILENAME is the name of the file.
function(define_filename_for_sources TARGET_NAME)
    get_target_property(SOURCE_FILES "${TARGET_NAME}" SOURCES)

    foreach(SOURCE_FILE ${SOURCE_FILES})
        get_filename_component(FILE_BASE_NAME "${SOURCE_FILE}" NAME)

        # Set the updated compile definitions on the source file.
        set_property(SOURCE "${SOURCE_FILE}" APPEND
					 PROPERTY COMPILE_DEFINITIONS "FILENAME=\"${FILE_BASE_NAME}\""
		)
    endforeach()
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





# Find Required packages
find_package(OpenGL REQUIRED)

if (OPENGL_FOUND)
	log_msg("Found OpenGL")
	log_msg("    OpenGL include dir   : ${OPENGL_INCLUDE_DIR}")
	log_msg("    OpenGL link libraries: ${OPENGL_gl_LIBRARY}")
else (OPENGL_FOUND)
	message(FATAL_ERROR "OpenGL not found")
endif()


find_package(Python COMPONENTS Interpreter Development REQUIRED)
find_package(Git REQUIRED)


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


# Save program configuration values into include/ to avoid recompiling entire project as with macros
log_msg("Write Jactorio compile time program configuration files")

function(jactorio_c_config_val PROP_NAME CONFIG_VAL)
	log_msg("${PROP_NAME} = ${CONFIG_VAL}")

	set(FILE_PATH ${CMAKE_SOURCE_DIR}/include/_config/${PROP_NAME})


	# Do not write if value has not changed, avoids recompilation
	set(WRITE_TO_PATH False)

	if (EXISTS ${FILE_PATH})
		file(READ ${FILE_PATH} EXISTING_CONFIG_VAL)

		if (NOT ${EXISTING_CONFIG_VAL} STREQUAL ${CONFIG_VAL})
			set(WRITE_TO_PATH True)
		endif ()
	else ()
		set(WRITE_TO_PATH True)
	endif ()

	if (WRITE_TO_PATH)
		file(WRITE ${FILE_PATH} ${CONFIG_VAL})
	endif ()

endfunction()


execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --always
			    OUTPUT_VARIABLE GIT_REPO_VERSION 
				OUTPUT_STRIP_TRAILING_WHITESPACE)

set(JACTORIO_BUILD_NUMBER 0) # 0 if no build number defined
if (DEFINED ENV{buildNumber})
	set(JACTORIO_BUILD_NUMBER $ENV{buildNumber})
endif ()

jactorio_c_config_val("version" "\"${GIT_REPO_VERSION}\"")
jactorio_c_config_val("log_level" 0) # 0 - Debug, 1 - Info, 2 - Warning, 3 - Error, 4 - Critical, 5 - None
jactorio_c_config_val("build_target_platform" \"${CMAKE_SYSTEM_NAME}\")
jactorio_c_config_val("build_number" ${JACTORIO_BUILD_NUMBER})


# Dependencies build arguments
add_compile_definitions(GLEW_STATIC)


#
add_subdirectory("lib")


enable_testing()

add_subdirectory("src")
if (JACTORIO_BUILD_TESTS)
	add_subdirectory("test")
endif ()
