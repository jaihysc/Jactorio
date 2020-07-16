// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "renderer/gui/imgui_manager.h"

#include <cassert>
#include <imgui.h>
#include <SDL.h>
#include <unordered_map>
#include <examples/imgui_impl_opengl3.h>
#include <examples/imgui_impl_sdl.h>

#include "jactorio.h"

#include "data/prototype/entity/entity.h"
#include "game/event/event.h"
#include "game/player/player_data.h"
#include "renderer/gui/gui_colors.h"
#include "renderer/gui/gui_menus.h"
#include "renderer/gui/gui_menus_debug.h"
#include "renderer/rendering/renderer.h"
#include "renderer/window/window_manager.h"

// Inventory

const std::unordered_map<unsigned, jactorio::core::QuadPosition>* sprite_positions = nullptr;
unsigned int tex_id                                                                = 0;  // Assigned by openGL

void jactorio::renderer::SetupCharacterData(RendererSprites& renderer_sprites) {
	sprite_positions = &renderer_sprites.GetSpritemap(data::Sprite::SpriteGroup::gui).spritePositions;
	tex_id           = renderer_sprites.GetTexture(data::Sprite::SpriteGroup::gui)->GetId();
}

jactorio::renderer::MenuData jactorio::renderer::GetMenuData() {
	return {*sprite_positions, tex_id};
}


// Errors
void jactorio::renderer::ShowErrorPrompt(const std::string& err_title,
                                         const std::string& err_message) {
	bool quit = false;

	SDL_Event e;
	while (!quit) {
		Renderer::GClear();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(GetWindow());
		ImGui::NewFrame();

		ImGui::SetNextWindowPos({0, 0}, 0, {0.5, 0.5});

		const ImGuiWindowFlags flags = 0;
		ImGui::Begin("Error", nullptr, flags);

		ImGui::TextWrapped("%s", err_title.c_str());
		ImGui::TextWrapped("%s", err_message.c_str());
		ImGui::NewLine();
		quit = ImGui::Button("Close");

		ImGui::End();

		if (e.type == SDL_QUIT)
			quit = true;

		// Render

		ImGui::Render();
		SDL_GL_SwapWindow(GetWindow());
		SDL_PollEvent(&e);
	}
}


void jactorio::renderer::Setup(SDL_Window* window) {
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
	ImGui_ImplSDL2_InitForOpenGL(window, GetContext());

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
	                    ImVec2(J_GUI_STYLE_WINDOW_PADDING_X, J_GUI_STYLE_WINDOW_PADDING_Y));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
	                    ImVec2(J_GUI_STYLE_FRAME_PADDING_X, J_GUI_STYLE_FRAME_PADDING_Y));

	// Window colors
	ImGui::PushStyleColor(ImGuiCol_Text, J_GUI_COL_TEXT);

	ImGui::PushStyleColor(ImGuiCol_WindowBg, J_GUI_COL_WINDOW_BG);
	ImGui::PushStyleColor(ImGuiCol_TitleBg, J_GUI_COL_TITLE_BG);
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, J_GUI_COL_TITLE_BG_ACTIVE);

	// Inventory boxes
	ImGui::PushStyleColor(ImGuiCol_FrameBg, J_GUI_COL_FRAME_BG);
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, J_GUI_COL_FRAME_BG_HOVER);
	ImGui::PushStyleColor(ImGuiCol_FrameBgActive, J_GUI_COL_FRAME_BG_ACTIVE);

	// Buttons
	ImGui::PushStyleColor(ImGuiCol_Button, J_GUI_COL_BUTTON);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, J_GUI_COL_BUTTON_HOVER);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, J_GUI_COL_BUTTON_ACTIVE);

	ImGui::PushStyleColor(ImGuiCol_CheckMark, J_GUI_COL_CHECKMARK);

	// The large horizontal row button like
	ImGui::PushStyleColor(ImGuiCol_Header, J_GUI_COL_HEADER);
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, J_GUI_COL_HEADER_HOVER);
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, J_GUI_COL_HEADER_ACTIVE);

	// Menu tabs
	ImGui::PushStyleColor(ImGuiCol_Tab, J_GUI_COL_TAB);
	ImGui::PushStyleColor(ImGuiCol_TabHovered, J_GUI_COL_TAB_HOVER);
	ImGui::PushStyleColor(ImGuiCol_TabActive, J_GUI_COL_TAB_ACTIVE);

	// Resize tab
	ImGui::PushStyleColor(ImGuiCol_ResizeGrip, J_GUI_COL_RESIZE_GRIP);
	ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered, J_GUI_COL_RESIZE_GRIP_HOVER);
	ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, J_GUI_COL_RESIZE_GRIP_ACTIVE);

	// Separators
	ImGui::PushStyleColor(ImGuiCol_Separator, J_GUI_COL_SEPARATOR);
	ImGui::PushStyleColor(ImGuiCol_SeparatorHovered, J_GUI_COL_SEPARATOR_HOVER);
	ImGui::PushStyleColor(ImGuiCol_SeparatorActive, J_GUI_COL_SEPARATOR_ACTIVE);

	// Popup
	ImGui::PushStyleColor(ImGuiCol_PopupBg, J_GUI_COL_POPUP_BG);

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {J_GUI_VAR_ITEM_SPACING_X, J_GUI_VAR_ITEM_SPACING_Y});

	LOG_MESSAGE(info, "Imgui initialized");
}

void DrawMenu(jactorio::renderer::Menu menu,
              jactorio::game::PlayerData& player_data, const jactorio::data::PrototypeManager& data_manager,
              jactorio::data::UniqueDataBase* unique_data = nullptr) {
	auto& gui_menu = jactorio::renderer::menus[static_cast<int>(menu)];

	if (gui_menu.visible) {
		gui_menu.drawPtr(player_data, data_manager, nullptr, unique_data);
	}
}

void jactorio::renderer::ImguiDraw(game::PlayerData& player_data, const data::PrototypeManager& data_manager,
                                   game::EventData& event) {
	EXECUTION_PROFILE_SCOPE(imgui_draw_timer, "Imgui draw");

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(GetWindow());
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
		drew_gui = static_cast<const data::Entity*>(layer->prototypeData)->OnRShowGui(player_data, data_manager, layer);
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

void jactorio::renderer::ImguiTerminate() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	LOG_MESSAGE(info, "Imgui terminated");
}
