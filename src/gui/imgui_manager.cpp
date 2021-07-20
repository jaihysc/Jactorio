// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "gui/imgui_manager.h"

#include <backends/imgui_impl_sdl.h>
#include <imgui.h>

#include "jactorio.h"

#include "game/player/player.h"
#include "game/world/chunk_tile.h"
#include "game/world/world.h"
#include "gui/colors.h"
#include "gui/context.h"
#include "gui/menu_data.h"
#include "gui/menus.h"
#include "proto/abstract/conveyor.h"
#include "proto/abstract/entity.h"
#include "proto/inserter.h"
#include "proto/localization.h"
#include "render/display_window.h"
#include "render/imgui_renderer.h"
#include "render/proto_renderer.h"
#include "render/spritemap_generator.h"
#include "render/tile_renderer.h"

using namespace jactorio;

gui::ImGuiManager::~ImGuiManager() {
    if (hasInitRenderer_) {
        imRenderer.Terminate();
    }
    if (hasImGuiContext_) {
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        LOG_MESSAGE(info, "Imgui terminated");
    }
}

void gui::ImGuiManager::Init(const render::DisplayWindow& display_window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    hasImGuiContext_ = true;

    ImGuiIO& io = ImGui::GetIO();
    // (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    io.IniFilename                       = nullptr; // Disables imgui saving
    io.ConfigWindowsMoveFromTitleBarOnly = true;    //

    // Setup Platform/Renderer bindings
    imRenderer.Init();
    hasInitRenderer_ = true;
    ImGui_ImplSDL2_InitForOpenGL(display_window.GetWindow(), display_window.GetContext());

    // Factorio inspired Imgui style
    auto& style             = ImGui::GetStyle();
    style.WindowRounding    = 0.0f;
    style.ChildRounding     = 0.0f;
    style.FrameRounding     = 0.0f;
    style.GrabRounding      = 0.0f;
    style.PopupRounding     = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.TabRounding       = 0.0f;

    // Borders
    style.FrameBorderSize = 0.f;

    // Padding
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(kGuiStyleWindowPaddingX, kGuiStyleWindowPaddingY));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(kGuiStyleFramePaddingX, kGuiStyleFramePaddingY));

    // Window colors
    ImGui::PushStyleColor(ImGuiCol_Text, kGuiColText);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, kGuiColWindowBg);
    ImGui::PushStyleColor(ImGuiCol_TitleBg, kGuiColTitleBg);
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, kGuiColTitleBgActive);

    // Inventory boxes
    ImGui::PushStyleColor(ImGuiCol_FrameBg, kGuiColFrameBg);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, kGuiColFrameBgHover);
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, kGuiColFrameBgActive);

    // Buttons
    ImGui::PushStyleColor(ImGuiCol_Button, kGuiColButton);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, kGuiColButtonHover);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, kGuiColButtonActive);

    ImGui::PushStyleColor(ImGuiCol_CheckMark, kGuiColCheckmark);

    // The large horizontal row button like
    ImGui::PushStyleColor(ImGuiCol_Header, kGuiColHeader);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, kGuiColHeaderHover);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, kGuiColHeaderActive);

    // Menu tabs
    ImGui::PushStyleColor(ImGuiCol_Tab, kGuiColTab);
    ImGui::PushStyleColor(ImGuiCol_TabHovered, kGuiColTabHover);
    ImGui::PushStyleColor(ImGuiCol_TabActive, kGuiColTabActive);

    // Resize tab
    ImGui::PushStyleColor(ImGuiCol_ResizeGrip, kGuiColResizeGrip);
    ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered, kGuiColResizeGripHover);
    ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, kGuiColResizeGripActive);

    // Separators
    ImGui::PushStyleColor(ImGuiCol_Separator, kGuiColSeparator);
    ImGui::PushStyleColor(ImGuiCol_SeparatorHovered, kGuiColSeparatorHover);
    ImGui::PushStyleColor(ImGuiCol_SeparatorActive, kGuiColSeparatorActive);

    // Popup
    ImGui::PushStyleColor(ImGuiCol_PopupBg, kGuiColPopupBg);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {kGuiStyleItemSpacingX, kGuiStyleItemSpacingY});

    // Scrollbar
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, kGuiStyleScrollBarSize);

    LOG_MESSAGE(info, "Imgui initialized");
}

void gui::ImGuiManager::InitData(const render::Spritemap& spritemap, const render::Texture& texture) {
    spritePositions_ = &spritemap.GetTexCoords();
    texId_           = texture.GetId();
}

bool gui::ImGuiManager::LoadFont(const proto::Localization& localization) const {
    auto& io = ImGui::GetIO();

    const auto font_path = std::string(data::PrototypeManager::kDataFolder) + "/" + localization.fontPath;
    ImWchar glyph_ranges[]{1, 0xFFFF, 0};

    LOG_MESSAGE_F(
        info, "Loading '%s' %f font '%s'", localization.identifier.c_str(), localization.fontSize, font_path.c_str());

    if (io.Fonts->AddFontFromFileTTF(font_path.c_str(), localization.fontSize, nullptr, glyph_ranges) == nullptr) {
        throw std::runtime_error("Failed to load font " + font_path);
    }
    return io.Fonts->Build();
}

void gui::ImGuiManager::BeginFrame(const render::DisplayWindow& display_window) const {
    ImGui_ImplSDL2_NewFrame(display_window.GetWindow());
    ImGui::NewFrame();
    imRenderer.buffer.GlWriteBegin();
}

void gui::ImGuiManager::RenderFrame() const {
    imRenderer.buffer.GlWriteEnd();
    imRenderer.RenderWorld(texId_);

    ImGui::Render();
    imRenderer.RenderGui(ImGui::GetDrawData());
}

void gui::ImGuiManager::PrepareWorld(const game::World& world, const render::TileRenderer& renderer) const {
    // Save some performance by not rendering at far zooms
    constexpr auto min_conveyor_render_zoom = 0.6f;
    constexpr auto min_inserter_render_zoom = 0.8f;

    // Render extra tiles of tile margin off the screen
    // Since inserters, conveyor items are more than one tile, despite the point registered for logic updates
    // not visible, other parts may still be
    constexpr auto tile_margin  = 40;
    constexpr auto pixel_margin = SafeCast<int>(tile_margin * render::TileRenderer::tileWidth);

    const auto bottom_right = renderer.WorldCoordToBufferPos(
        renderer.ScreenPosToWorldCoord({SafeCast<int>(render::TileRenderer::GetWindowWidth()),
                                        SafeCast<int>(render::TileRenderer::GetWindowHeight())}));

    const auto top_left_cutoff     = Position2(SafeCast<float>(-pixel_margin), SafeCast<float>(-pixel_margin));
    const auto bottom_right_cutoff = Position2(SafeCast<float>(bottom_right.x + pixel_margin), //
                                               SafeCast<float>(bottom_right.y + pixel_margin));

    const auto origin = renderer.WorldCoordToBufferPos({0, 0});


    auto get_pixel_pos = [&origin](const WorldCoord& coord) {
        return Position2(SafeCast<float>(coord.x * SafeCast<int>(render::TileRenderer::tileWidth) + origin.x),
                         SafeCast<float>(coord.y * SafeCast<int>(render::TileRenderer::tileWidth) + origin.y));
    };
    auto is_visible = [&top_left_cutoff, &bottom_right_cutoff](const Position2<float>& pixel_pos) {
        if (pixel_pos.x < top_left_cutoff.x || pixel_pos.y < top_left_cutoff.y) {
            return false;
        }
        if (pixel_pos.x > bottom_right_cutoff.x || pixel_pos.y > bottom_right_cutoff.y) {
            return false;
        }
        return true;
    };

    if (renderer.GetZoom() >= min_conveyor_render_zoom) {
        for (auto [prototype, unique_data, coord] : world.LogicGet(game::LogicGroup::conveyor)) {
            const auto pixel_pos = get_pixel_pos(coord);
            if (!is_visible(pixel_pos)) {
                continue;
            }

            const auto* conveyor = SafeCast<const proto::ConveyorData*>(unique_data.Get());
            assert(conveyor != nullptr);

            PrepareConveyorSegmentItems(imRenderer.buffer, *spritePositions_, pixel_pos, *conveyor->structure);
        }
    }
    if (renderer.GetZoom() >= min_inserter_render_zoom) {
        for (auto& [prototype, unique_data, coord] : world.LogicGet(game::LogicGroup::inserter)) {
            const auto pixel_pos = get_pixel_pos(coord);
            if (!is_visible(pixel_pos)) {
                continue;
            }

            const auto* inserter      = SafeCast<const proto::Inserter*>(prototype.Get());
            const auto* inserter_data = SafeCast<const proto::InserterData*>(unique_data.Get());
            assert(inserter != nullptr);
            assert(inserter_data != nullptr);

            PrepareInserterParts(imRenderer.buffer, *spritePositions_, pixel_pos, *inserter, *inserter_data);
        }
    }
}

void gui::ImGuiManager::PrepareGui(GameWorlds& worlds,
                                   game::Logic& logic,
                                   game::Player& player,
                                   const data::PrototypeManager& proto,
                                   game::EventData& event) const {
    // Has imgui handled a mouse or keyboard event?
    ImGuiIO& io             = ImGui::GetIO();
    input_mouse_captured    = io.WantCaptureMouse;
    input_keyboard_captured = io.WantCaptureKeyboard;

    // Make the font bigger
    // auto font = ImGetFont();
    // font->Scale = 1.f;
    // ImPushFont(font);
    // ImPopFont();

    MenuData menu_data = {*spritePositions_, texId_};
    const Context context{worlds, logic, player, proto, menu_data};


    bool drew_gui = false;

    // Entity gui
    auto* tile = player.placement.GetActivatedTile();
    if (tile != nullptr) {
        drew_gui = tile->GetPrototype<proto::Entity>()->OnRShowGui(context, tile);
        if (drew_gui) {
            SetVisible(Menu::CharacterMenu, false);
        }
        else {
            player.placement.SetActivatedTile(nullptr);
        }
    }

    // Menus
    auto draw_menu = [](Menu menu, const Context& context, proto::UniqueDataBase* unique_data = nullptr) {
        auto& gui_menu = menus[static_cast<int>(menu)];

        if (gui_menu.visible) {
            gui_menu.drawPtr(context, nullptr, unique_data);
        }
    };

    if (!drew_gui) {
        draw_menu(Menu::CharacterMenu, context);
    }

    // Player gui
    draw_menu(Menu::DebugMenu, context);

    CursorWindow(context, nullptr, nullptr);
    CraftingQueue(context, nullptr, nullptr);
    PickupProgressbar(context, nullptr, nullptr);
}
