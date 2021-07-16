// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "gui/imgui_manager.h"

#include <examples/imgui_impl_sdl.h>
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

void gui::ImGuiManager::LoadFont(const proto::Localization& localization) const {
    auto& io = ImGui::GetIO();

    const auto font_path = std::string(data::PrototypeManager::kDataFolder) + "/" + localization.fontPath;
    ImWchar glyph_ranges[]{1, 0xFFFF, 0};

    LOG_MESSAGE_F(
        info, "Loading '%s' %f font '%s'", localization.identifier.c_str(), localization.fontSize, font_path.c_str());

    if (io.Fonts->AddFontFromFileTTF(font_path.c_str(), localization.fontSize, nullptr, glyph_ranges) == nullptr) {
        throw std::runtime_error("Failed to load font " + font_path);
    }
    io.Fonts->Build();
}

void gui::ImGuiManager::BeginFrame(const render::DisplayWindow& display_window) const {
    imRenderer.buffer.vert.clear();
    imRenderer.buffer.idx.clear();
    ImGui_ImplSDL2_NewFrame(display_window.GetWindow());
    ImGui::NewFrame();
}

void gui::ImGuiManager::RenderFrame() const {
    imRenderer.Bind();
    imRenderer.RenderWorld(texId_);

    ImGui::Render();
    imRenderer.RenderGui(ImGui::GetDrawData());
}

void gui::ImGuiManager::PrepareWorld(const game::World& world, const render::TileRenderer& renderer) const {
    for (auto [prototype, unique_data, coord] : world.LogicGet(game::LogicGroup::conveyor)) {
        const auto pixel_pos   = renderer.WorldCoordToBufferPos(coord);
        const auto pixel_pos_f = Position2(LossyCast<float>(pixel_pos.x), LossyCast<float>(pixel_pos.y));

        const auto* conveyor = SafeCast<const proto::ConveyorData*>(unique_data.Get());

        PrepareConveyorSegmentItems(imRenderer.buffer, *spritePositions_, pixel_pos_f, *conveyor->structure);
    }
    for (auto& logic_object : world.LogicGet(game::LogicGroup::inserter)) {
        //
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
