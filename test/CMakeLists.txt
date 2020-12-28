log_msg("")
log_msg("[Jactorio test ]")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_BINARY_DIR})

jactorio_show_properties()
jactorio_copy_data_files()



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
add_compile_definitions(JACTORIO_BUILD_TEST)  # Disables warning when using test only functions

set(JACTORIO_TEST_DIR ${PROJECT_SOURCE_DIR}/test)
# ======================================== Test files .cpp
set(JACTORIO_TEST_FILES
	${JACTORIO_TEST_DIR}/core/convertTests.cpp
	${JACTORIO_TEST_DIR}/core/file_systemTests.cpp
	${JACTORIO_TEST_DIR}/core/loop_commonTests.cpp
	${JACTORIO_TEST_DIR}/core/mathTests.cpp
	${JACTORIO_TEST_DIR}/core/orientationTests.cpp
	${JACTORIO_TEST_DIR}/core/pointer_wrapperTests.cpp
	${JACTORIO_TEST_DIR}/core/resource_guardTests.cpp
	${JACTORIO_TEST_DIR}/core/utilityTests.cpp


	${JACTORIO_TEST_DIR}/data/local_parserTests.cpp
	${JACTORIO_TEST_DIR}/data/prototype_managerTests.cpp
	${JACTORIO_TEST_DIR}/data/pybind_managerTests.cpp
	${JACTORIO_TEST_DIR}/data/save_game_managerTests.cpp
	${JACTORIO_TEST_DIR}/data/unique_data_managerTests.cpp

	${JACTORIO_TEST_DIR}/data/cereal/serialization_typeTests.cpp
	${JACTORIO_TEST_DIR}/data/cereal/supportTests.cpp


	${JACTORIO_TEST_DIR}/game/eventTests.cpp
	${JACTORIO_TEST_DIR}/game/game_dataTests.cpp

	${JACTORIO_TEST_DIR}/game/input/input_managerTests.cpp
	${JACTORIO_TEST_DIR}/game/input/mouse_selectionTests.cpp

	${JACTORIO_TEST_DIR}/game/logic/conveyor_controllerTests.cpp
	${JACTORIO_TEST_DIR}/game/logic/conveyor_structTests.cpp
	${JACTORIO_TEST_DIR}/game/logic/deferral_timerTests.cpp
	${JACTORIO_TEST_DIR}/game/logic/conveyor_utilityTests.cpp
	${JACTORIO_TEST_DIR}/game/logic/inserter_controllerTests.cpp
	${JACTORIO_TEST_DIR}/game/logic/inventory_controllerTests.cpp
	${JACTORIO_TEST_DIR}/game/logic/item_logisticsTests.cpp
	${JACTORIO_TEST_DIR}/game/logic/logic_dataTests.cpp
	${JACTORIO_TEST_DIR}/game/logic/placement_controllerTests.cpp

	${JACTORIO_TEST_DIR}/game/player/keybind_managerTests.cpp
	${JACTORIO_TEST_DIR}/game/player/player_actionTests.cpp
	${JACTORIO_TEST_DIR}/game/player/player_dataTests_inventory.cpp
	${JACTORIO_TEST_DIR}/game/player/player_dataTests_placement.cpp
	${JACTORIO_TEST_DIR}/game/player/player_dataTests_recipe.cpp
	${JACTORIO_TEST_DIR}/game/player/player_dataTests_world.cpp

	${JACTORIO_TEST_DIR}/game/world/chunkTests.cpp
	${JACTORIO_TEST_DIR}/game/world/chunk_tileTests.cpp
	${JACTORIO_TEST_DIR}/game/world/chunk_tile_layerTests.cpp
	${JACTORIO_TEST_DIR}/game/world/overlay_elementTests.cpp
	${JACTORIO_TEST_DIR}/game/world/update_dispatcherTests.cpp
	${JACTORIO_TEST_DIR}/game/world/world_dataTests.cpp


	${JACTORIO_TEST_DIR}/proto/assembly_machineTests.cpp
	${JACTORIO_TEST_DIR}/proto/container_entityTests.cpp
	${JACTORIO_TEST_DIR}/proto/inserterTests.cpp
	${JACTORIO_TEST_DIR}/proto/mining_drillTests.cpp
	${JACTORIO_TEST_DIR}/proto/noise_layerTests.cpp
	${JACTORIO_TEST_DIR}/proto/prototype_typeTests.cpp
	${JACTORIO_TEST_DIR}/proto/recipeTests.cpp
	${JACTORIO_TEST_DIR}/proto/splitterTests.cpp
	${JACTORIO_TEST_DIR}/proto/spriteTests.cpp

	${JACTORIO_TEST_DIR}/proto/abstract/conveyorTests.cpp
	${JACTORIO_TEST_DIR}/proto/abstract/entityTests.cpp
	${JACTORIO_TEST_DIR}/proto/interface/renderableTests.cpp


	${JACTORIO_TEST_DIR}/render/mvp_managerTests.cpp
	${JACTORIO_TEST_DIR}/render/spritemap_generatorTests.cpp
)
# ======================================== END Test files .cpp

# Test executable
add_executable(jactorioTest
	${JACTORIO_TEST_DIR}/jactorioTests.cpp
	${JACTORIO_TEST_FILES}
)
target_link_libraries(jactorioTest
	gtest_main jactorioBase
)
target_include_directories(jactorioTest
	PUBLIC
	${JACTORIO_TEST_DIR}
)


# MSVC complains that it introduced reference to symbol _fltused which was previously compiled with /GL
# This can be ignored since _fltused just means that floating point was used
# https://stackoverflow.com/questions/1583196
if (MSVC)
	target_link_options(jactorioTest PRIVATE /INCLUDE:_fltused)
endif()
