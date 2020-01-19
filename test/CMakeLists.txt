# CMakeList.txt : CMake project for jactorio TESTS, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

message("")
message("~~~~~ [ jactorio tests ] ~~~~~")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR})
message("Binary directory: " ${CMAKE_CURRENT_BINARY_DIR})

# Copy data folder for certain tests
message(">> Copying files to output directory")
file(GLOB_RECURSE DATA_FILES 
	RELATIVE ${PROJECT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/data/*)
foreach(DATA_FILE ${DATA_FILES})
	# message(${DATA_FILE})
	configure_file(${PROJECT_SOURCE_DIR}/${DATA_FILE} ${DATA_FILE} COPYONLY)
endforeach(DATA_FILE)
message(">> DONE Copying files to output directory: " ${CMAKE_CURRENT_BINARY_DIR})

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
if(CMAKE_VERSION VERSION_LESS 2.8.11)
  include_directories("${gtest_SOURCE_DIR}/include")
endif()



# Add source to this project's executable.
add_executable (
	jactorioTest
	${JACTORIO_SRC_FILES}
	${ADDITIONAL_BUILD_FILES}

	${JACTORIO_TEST_FILES}
)

# GTest
target_link_libraries(jactorioTest gtest_main)


target_include_directories(jactorioTest PRIVATE ${JACTORIO_INCLUDE_FILES})

# Dependencies libraries
if (WIN32)
	target_link_libraries(jactorioTest ${LIBS_WIN})
endif (WIN32)
if (UNIX)
    target_link_libraries(jactorioTest ${LIBS_UNIX})
endif (UNIX)