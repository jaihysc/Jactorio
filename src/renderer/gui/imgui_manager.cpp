// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "renderer/gui/imgui_manager.h"

#include <cassert>
#include <imgui.h>
#include <SDL.h>
#include <unordered_map>
#include <examples/imgui_impl_opengl3.h>
#include <examples/imgui_impl_sdl.h>

#include "jactorio.h"

#include "data/prototype/abstract_proto/entity.h"
#include "game/event/event.h"
#include "game/player/player_data.h"
#include "renderer/display_window.h"
#include "renderer/gui/gui_colors.h"
#include "renderer/gui/gui_menus.h"
#include "renderer/gui/gui_menus_debug.h"
#include "renderer/rendering/renderer.h"

// Inventory

using namespace jactorio;

const SpriteUvCoordsT* sprite_positions = nullptr;
unsigned int tex_id = 0;  // Assigned by openGL

void renderer::SetupCharacterData(RendererSprites& renderer_sprites) {
	sprite_positions = &renderer_sprites.GetSpritemap(data::Sprite::SpriteGroup::gui).spritePositions;
	tex_id           = renderer_sprites.GetTexture(data::Sprite::SpriteGroup::gui)->GetId();
}

renderer::MenuData renderer::GetMenuData() {
	return {*sprite_positions, tex_id};
}


void renderer::Setup(const DisplayWindow& display_window) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	// (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	io.IniFilename                       = nullptr;  // Disables imgui saving
	io.ConfigWindowsMoveFromTitleBarOnly = true;  //

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
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
	                    ImVec2(kGuiStyleWindowPaddingX, kGuiStyleWindowPaddingY));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
	                    ImVec2(kGuiStyleFramePaddingX, kGuiStyleFramePaddingY));

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

	LOG_MESSAGE(info, "Imgui initialized");
}

void DrawMenu(renderer::Menu menu,
              game::PlayerData& player_data, const data::PrototypeManager& data_manager,
              data::UniqueDataBase* unique_data = nullptr) {
	auto& gui_menu = renderer::menus[static_cast<int>(menu)];

	if (gui_menu.visible) {
		gui_menu.drawPtr(player_data, data_manager, nullptr, unique_data);
	}
}

void renderer::ImguiDraw(const DisplayWindow& display_window,
                         game::PlayerData& player_data, const data::PrototypeManager& data_manager,
                         game::EventData& event) {
	EXECUTION_PROFILE_SCOPE(imgui_draw_timer, "Imgui draw");

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(display_window.GetWindow());
	ImGui::NewFrame();

	// Has imgui handled a mouse or keyboard event?
	ImGuiIO& io             = ImGui::GetIO();
	input_mouse_captured    = io.WantCaptureMouse;
	input_keyboard_captured = io.WantCaptureKeyboard;

	// Make the font bigger
	// auto font = ImGetFont();
	// font->Scale = 1.f;
	// ImPushFont(font);
	// ImPopFont();

	DrawMenu(Menu::DebugMenu, player_data, data_manager);
	DebugMenuLogic(player_data, data_manager);

	// Draw gui for active entity
	// Do not draw character and recipe menu while in an entity menu

	bool drew_gui = false;

	auto* layer = player_data.GetActivatedLayer();
	if (layer != nullptr) {
		layer    = &layer->GetMultiTileTopLeft();
		drew_gui = static_cast<const data::Entity*>(layer->prototypeData.Get())->OnRShowGui(player_data, data_manager, layer);
		if (drew_gui) {
			SetVisible(Menu::CharacterMenu, false);
		}
		else {
			player_data.SetActivatedLayer(nullptr);
		}
	}

	if (!drew_gui) {
		DrawMenu(Menu::CharacterMenu, player_data, data_manager);
	}

	// Player gui
	CursorWindow(player_data, data_manager);
	CraftingQueue(player_data, data_manager);
	PickupProgressbar(player_data, data_manager);

	// Render
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

void renderer::ImguiTerminate() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	LOG_MESSAGE(info, "Imgui terminated");
}
