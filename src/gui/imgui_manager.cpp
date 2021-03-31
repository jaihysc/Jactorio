// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "gui/imgui_manager.h"

#include <examples/imgui_impl_opengl3.h>
#include <examples/imgui_impl_sdl.h>
#include <exception>
#include <imgui.h>

#include "jactorio.h"

#include "core/execution_timer.h"

#include "proto/abstract/entity.h"
#include "proto/localization.h"

#include "game/player/player.h"
#include "game/world/chunk_tile.h"

#include "gui/colors.h"
#include "gui/menu_data.h"
#include "gui/menus.h"
#include "gui/menus_debug.h"
#include "render/display_window.h"
#include "render/renderer.h"
#include "render/spritemap_generator.h"

// Inventory

using namespace jactorio;

const SpriteUvCoordsT* sprite_positions = nullptr;
unsigned int tex_id                     = 0; // Assigned by openGL

void gui::SetupCharacterData(render::RendererSprites& renderer_sprites) {
    sprite_positions = &renderer_sprites.GetSpritemap(proto::Sprite::SpriteGroup::gui).spritePositions;
    tex_id           = renderer_sprites.GetTexture(proto::Sprite::SpriteGroup::gui)->GetId();
}

void gui::Setup(const render::DisplayWindow& display_window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    // (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    io.IniFilename                       = nullptr; // Disables imgui saving
    io.ConfigWindowsMoveFromTitleBarOnly = true;    //

    // Setup Platform/Renderer bindings
    ImGui_ImplOpenGL3_Init();
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

void gui::LoadFont(const proto::Localization& localization) {
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

void gui::ImguiBeginFrame(const render::DisplayWindow& display_window) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(display_window.GetWindow());
    ImGui::NewFrame();
}

void gui::ImguiRenderFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void DrawMenu(gui::Menu menu, const render::GuiRenderer& g_rendr, proto::UniqueDataBase* unique_data = nullptr) {
    auto& gui_menu = gui::menus[static_cast<int>(menu)];

    if (gui_menu.visible) {
        gui_menu.drawPtr({g_rendr, nullptr, unique_data});
    }
}

void gui::ImguiDraw(const render::DisplayWindow& /*display_window*/,
                    GameWorlds& worlds,
                    game::Logic& logic,
                    game::Player& player,
                    const data::PrototypeManager& proto,
                    game::EventData& /*event*/) {
    EXECUTION_PROFILE_SCOPE(imgui_draw_timer, "Imgui draw");

    // Has imgui handled a mouse or keyboard event?
    ImGuiIO& io             = ImGui::GetIO();
    input_mouse_captured    = io.WantCaptureMouse;
    input_keyboard_captured = io.WantCaptureKeyboard;

    // Make the font bigger
    // auto font = ImGetFont();
    // font->Scale = 1.f;
    // ImPushFont(font);
    // ImPopFont();

    MenuData menu_data = {*sprite_positions, tex_id};
    const render::GuiRenderer g_rendr{worlds, logic, player, proto, menu_data};


    bool drew_gui = false;

    auto* tile = player.placement.GetActivatedTile();
    if (tile != nullptr) {
        drew_gui = tile->GetPrototype<proto::Entity>()->OnRShowGui(g_rendr, tile);
        if (drew_gui) {
            SetVisible(Menu::CharacterMenu, false);
        }
        else {
            player.placement.SetActivatedTile(nullptr);
        }
    }

    if (!drew_gui) {
        DrawMenu(Menu::CharacterMenu, g_rendr);
    }

    // Player gui
    DrawMenu(Menu::DebugMenu, g_rendr);
    DebugMenuLogic(worlds, logic, player, proto);

    CursorWindow(g_rendr);
    CraftingQueue(g_rendr);
    PickupProgressbar(g_rendr);
}

void gui::ImguiTerminate() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    LOG_MESSAGE(info, "Imgui terminated");
}
