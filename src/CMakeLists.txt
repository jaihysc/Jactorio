# CMakeList.txt : CMake project for jactorio, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.8)


IF(CMAKE_BUILD_TYPE MATCHES Debug)
	message("Type: Debug")
    add_compile_definitions(JACTORIO_DEBUG_BUILD)  # Break on openGL error

ELSEIF(CMAKE_BUILD_TYPE MATCHES Release)
	message("Type: <<<<< Release build >>>>>")

ELSE()
  message(FATAL_ERROR "Unknown build type, specify with -D CMAKE_BUILD_TYPE=")
ENDIF()

message("Target: ${CMAKE_SYSTEM_NAME}")


# Copy data folder to executable directory
message(" => Copying files to output directory")
file(GLOB_RECURSE DATA_FILES 
	RELATIVE ${PROJECT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/data/*)
foreach(DATA_FILE ${DATA_FILES})
	# message(${DATA_FILE})
	configure_file(${PROJECT_SOURCE_DIR}/${DATA_FILE} ${DATA_FILE} COPYONLY)
endforeach(DATA_FILE)
message(" => DONE Copying files to output directory")



add_executable(
	jactorio 

	${JACTORIO_DIR}/jactorio.cpp
	${ADDITIONAL_BUILD_FILES}
	${JACTORIO_SRC_FILES}
)
# Headers
target_include_directories(jactorio PRIVATE ${JACTORIO_INCLUDE_FILES})

# Dependencies
if (WIN32)
	target_link_libraries(jactorio ${LIBS_WIN})
endif (WIN32)
if (UNIX)
    target_link_libraries(jactorio ${LIBS_UNIX})
endif (UNIX)