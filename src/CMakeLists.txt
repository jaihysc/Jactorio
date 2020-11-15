# CMakeList.txt : CMake project for jactorio, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.9)

log_msg("")
log_msg("[   Jactorio   ]")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_BINARY_DIR})

jactorio_show_properties()
jactorio_copy_data_files()


set(JACTORIO_DIR ${PROJECT_SOURCE_DIR}/src)
# ======================================== Header files .h
set(JACTORIO_INCLUDE_FILES
        ${PROJECT_SOURCE_DIR}/include
        )
# ======================================== END Header files .h
# ======================================== Source files .cpp
set(JACTORIO_SRC_FILES
        ${JACTORIO_DIR}/core/crash_handler.cpp
        ${JACTORIO_DIR}/core/execution_timer.cpp
        ${JACTORIO_DIR}/core/filesystem.cpp
        ${JACTORIO_DIR}/core/logger.cpp
        ${JACTORIO_DIR}/core/utility.cpp


        ${JACTORIO_DIR}/data/local_parser.cpp
        ${JACTORIO_DIR}/data/prototype_manager.cpp
        ${JACTORIO_DIR}/data/pybind_manager.cpp
        ${JACTORIO_DIR}/data/save_game_manager.cpp
        ${JACTORIO_DIR}/data/unique_data_manager.cpp


        ${JACTORIO_DIR}/game/input/input_manager.cpp
        ${JACTORIO_DIR}/game/input/mouse_selection.cpp

        ${JACTORIO_DIR}/game/logic/conveyor_controller.cpp
        ${JACTORIO_DIR}/game/logic/conveyor_segment.cpp
        ${JACTORIO_DIR}/game/logic/deferral_timer.cpp
        ${JACTORIO_DIR}/game/logic/inserter_controller.cpp
        ${JACTORIO_DIR}/game/logic/inventory_controller.cpp
        ${JACTORIO_DIR}/game/logic/item_logistics.cpp
        ${JACTORIO_DIR}/game/logic/placement_controller.cpp
        ${JACTORIO_DIR}/game/logic_loop.cpp

        ${JACTORIO_DIR}/game/player/keybind_manager.cpp
        ${JACTORIO_DIR}/game/player/player_data.cpp

        ${JACTORIO_DIR}/game/world/chunk.cpp
        ${JACTORIO_DIR}/game/world/chunk_tile.cpp
        ${JACTORIO_DIR}/game/world/chunk_tile_layer.cpp
        ${JACTORIO_DIR}/game/world/update_dispatcher.cpp
        ${JACTORIO_DIR}/game/world/world_data.cpp


        ${JACTORIO_DIR}/gui/components.cpp
        ${JACTORIO_DIR}/gui/imgui_manager.cpp
        ${JACTORIO_DIR}/gui/layout.cpp
        ${JACTORIO_DIR}/gui/main_menu.cpp
        ${JACTORIO_DIR}/gui/menus.cpp
        ${JACTORIO_DIR}/gui/menus_debug.cpp


        ${JACTORIO_DIR}/proto/assembly_machine.cpp
        ${JACTORIO_DIR}/proto/container_entity.cpp
        ${JACTORIO_DIR}/proto/inserter.cpp
        ${JACTORIO_DIR}/proto/item.cpp
        ${JACTORIO_DIR}/proto/mining_drill.cpp
        ${JACTORIO_DIR}/proto/recipe.cpp
        ${JACTORIO_DIR}/proto/recipe_group.cpp
        ${JACTORIO_DIR}/proto/sprite.cpp

        ${JACTORIO_DIR}/proto/abstract/conveyor.cpp
        ${JACTORIO_DIR}/proto/abstract/entity.cpp

        ${JACTORIO_DIR}/proto/interface/renderable.cpp


        ${JACTORIO_DIR}/render/opengl/error.cpp
        ${JACTORIO_DIR}/render/opengl/index_buffer.cpp
        ${JACTORIO_DIR}/render/opengl/mvp_manager.cpp
        ${JACTORIO_DIR}/render/opengl/shader.cpp
        ${JACTORIO_DIR}/render/opengl/texture.cpp
        ${JACTORIO_DIR}/render/opengl/vertex_array.cpp
        ${JACTORIO_DIR}/render/opengl/vertex_buffer.cpp

        ${JACTORIO_DIR}/render/display_window.cpp
        ${JACTORIO_DIR}/render/render_loop.cpp

        ${JACTORIO_DIR}/render/proto_renderer.cpp
        ${JACTORIO_DIR}/render/renderer.cpp
        ${JACTORIO_DIR}/render/renderer_layer.cpp
        ${JACTORIO_DIR}/render/spritemap_generator.cpp
        )
# ======================================== END Source files .cpp


# ======================================== Lib files .cpp
set(JACTORIO_LIB_BUILD_FILES
        ${PROJECT_SOURCE_DIR}/lib/imgui/imgui.cpp
        ${PROJECT_SOURCE_DIR}/lib/imgui/imgui_demo.cpp
        ${PROJECT_SOURCE_DIR}/lib/imgui/imgui_draw.cpp
        ${PROJECT_SOURCE_DIR}/lib/imgui/imgui_widgets.cpp
        ${PROJECT_SOURCE_DIR}/lib/imgui/examples/imgui_impl_opengl3.cpp
        ${PROJECT_SOURCE_DIR}/lib/imgui/examples/imgui_impl_sdl.cpp
        )

IF (CMAKE_SYSTEM_NAME MATCHES Windows)
    set(JACTORIO_LIB_BUILD_FILES
            ${JACTORIO_LIB_BUILD_FILES}
            ${PROJECT_SOURCE_DIR}/lib/StackWalker/Main/StackWalker/StackWalker.cpp
            )
ENDIF ()
# ======================================== END Lib files .cpp

IF (JACTORIO_BUILD_TESTS OR NOT JACTORIO_INTERPROCEDURAL_OPTIMIZATION)
    # library linked to for tests
    add_library(jactorioBase
            ${JACTORIO_SRC_FILES}
            ${JACTORIO_LIB_BUILD_FILES}
            )
    target_include_directories(jactorioBase PUBLIC ${JACTORIO_INCLUDE_FILES})
    target_link_libraries(jactorioBase jactorioLib)
ENDIF ()


# Jactorio Executable
IF (JACTORIO_INTERPROCEDURAL_OPTIMIZATION)
    # Must compile sources separately for IPO
    add_executable(jactorio
            ${JACTORIO_DIR}/jactorio.cpp
            ${JACTORIO_SRC_FILES}
            ${JACTORIO_LIB_BUILD_FILES}
            )
    set_property(TARGET jactorio PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

    target_link_libraries(jactorio jactorioLib)
    target_include_directories(jactorio PUBLIC ${JACTORIO_INCLUDE_FILES})
ELSE ()
    add_executable(jactorio
            ${JACTORIO_DIR}/jactorio.cpp
            )
    target_link_libraries(jactorio jactorioBase)
ENDIF ()

