// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <GL/glew.h>

#include "render/display_window.h"

#include <SDL.h>

#include "jactorio.h"

#include "config.h"
#include "core/loop_common.h"
#include "game/event/hardware_events.h"
#include "game/input/input_manager.h"
#include "game/input/mouse_selection.h"
#include "gui/imgui_manager.h"
#include "proto/sprite.h"
#include "render/render_loop.h"
#include "render/renderer.h"

using namespace jactorio;

/// Fullscreen
bool render::DisplayWindow::IsFullscreen() const {
    constexpr Uint32 fullscreen_flag = SDL_WINDOW_FULLSCREEN;
    return SDL_GetWindowFlags(sdlWindow_) & fullscreen_flag;
}

void render::DisplayWindow::SetFullscreen(const bool desired_fullscreen) {
    if (IsFullscreen() == desired_fullscreen)
        return;

    if (desired_fullscreen) {
        // backup window position and window size
        SDL_GetWindowPosition(sdlWindow_, &windowPos_[0], &windowPos_[1]);
        SDL_GetWindowSize(sdlWindow_, &windowSize_[0], &windowSize_[1]);

        // get resolution of monitor
        // switch to full screen
        SDL_SetWindowFullscreen(sdlWindow_, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
    else {
        // Exit fullscreen
        SDL_SetWindowFullscreen(sdlWindow_, 0);

        // restore last window size and position
        SDL_SetWindowPosition(sdlWindow_, windowPos_[0], windowPos_[1]);
        SDL_SetWindowSize(sdlWindow_, windowSize_[0], windowSize_[1]);
    }
}


render::DisplayWindow::~DisplayWindow() {
    if (glContextActive_) {
        Terminate();
    }
}

int render::DisplayWindow::Init(const int width, const int height) {
    LOG_MESSAGE(info, "Using SDL2 for window creation");

    // ======================================================================
    // Initialize SDL

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        LOG_MESSAGE(critical, "SDL initialization failed");
        goto sdl_error;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, kRequiredGlMajor);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, kRequiredGlMinor);

    // Window initialization
    sdlWindow_ = SDL_CreateWindow((std::string("Jactorio ") + CConfig::kVersion).c_str(), // window title
                                  SDL_WINDOWPOS_UNDEFINED,                                // initial x position
                                  SDL_WINDOWPOS_UNDEFINED,                                // initial y position
                                  width,                                                  // width, in pixels
                                  height,                                                 // height, in pixels
                                  SDL_WINDOW_OPENGL                                       // flags - see below
    );
    if (sdlWindow_ == nullptr) {
        SDL_Quit();
        LOG_MESSAGE(critical, "Error initializing window");
        goto sdl_error;
    }

    SDL_SetWindowResizable(sdlWindow_, SDL_TRUE);

    SDL_GetWindowPosition(sdlWindow_, &windowPos_[0], &windowPos_[1]);
    SDL_GetWindowSize(sdlWindow_, &windowSize_[0], &windowSize_[1]);

    // Set the Jactorio icon for the window
    {
        const auto icon = proto::Sprite("core/graphics/jactorio-64-64.png"); // <-- This will throw if it fails

        // Convert the loaded Image into a surface for setting the icon
        SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(icon.GetImage().buffer,
                                                        icon.GetImage().width,
                                                        icon.GetImage().height,
                                                        32,
                                                        sizeof(unsigned char) * 4 * icon.GetImage().width,
                                                        0x000000ff,
                                                        0x0000ff00,
                                                        0x00ff0000,
                                                        0xff000000);

        // See https://wiki.libsdl.org/SDL_CreateRGBSurfaceFrom#Remarks

        SDL_SetWindowIcon(sdlWindow_, surface);
        SDL_FreeSurface(surface);
    }

    sdlGlContext_ = SDL_GL_CreateContext(sdlWindow_);
    if (sdlGlContext_ == nullptr) {
        LOG_MESSAGE(critical, "Failed to create OpenGL Context");
        goto sdl_error;
    }

    // ======================================================================
    // Initialize GLEW

    if (glewInit() != GLEW_OK) {
        LOG_MESSAGE(critical, "GLEW initialization failed");
        goto gl_error;
    }

    // ======================================================================
    // Opengl Setup

    // Always Vsync
    if (SDL_GL_SetSwapInterval(1) == -1) {
        LOG_MESSAGE_F(critical, "Failed to set gl swap interval: %s", SDL_GetError());
        goto gl_error;
    }

    glContextActive_ = true;
    LOG_MESSAGE_F(info, "OpenGL initialized - OpenGL Version: %s", glGetString(GL_VERSION));
    return 0;


    // Error handling

sdl_error:
    if (sdlWindow_ != nullptr)
        SDL_DestroyWindow(sdlWindow_);
    SDL_Quit();
    return 2;

gl_error:
    SDL_GL_DeleteContext(sdlGlContext_);
    SDL_DestroyWindow(sdlWindow_);
    SDL_Quit();
    return 3;
}

int render::DisplayWindow::Terminate() {
    SDL_GL_DeleteContext(sdlGlContext_);
    SDL_DestroyWindow(sdlWindow_);
    SDL_Quit();

    glContextActive_ = false;

    LOG_MESSAGE(info, "SDL, OpenGL terminated");
    return 0;
}

/// Window

SDL_Window* render::DisplayWindow::GetWindow() const {
    return sdlWindow_;
}

SDL_GLContext render::DisplayWindow::GetContext() const {
    return sdlGlContext_;
}

bool render::DisplayWindow::WindowContextActive() const {
    return glContextActive_;
}

// ======================================================================
// Events

void HandleWindowEvent(render::Renderer& renderer, game::EventData& event, const SDL_Event& sdl_event) {
    switch (sdl_event.window.event) {
    case SDL_WINDOWEVENT_RESIZED:
    case SDL_WINDOWEVENT_SIZE_CHANGED:
        ChangeWindowSize(renderer, event, sdl_event.window.data1, sdl_event.window.data2);
        break;

    case SDL_WINDOWEVENT_SHOWN:
    case SDL_WINDOWEVENT_HIDDEN:
    case SDL_WINDOWEVENT_EXPOSED:
    case SDL_WINDOWEVENT_MOVED:
    case SDL_WINDOWEVENT_MINIMIZED:
    case SDL_WINDOWEVENT_MAXIMIZED:
    case SDL_WINDOWEVENT_RESTORED:
    case SDL_WINDOWEVENT_ENTER:
    case SDL_WINDOWEVENT_LEAVE:
    case SDL_WINDOWEVENT_FOCUS_GAINED:
    case SDL_WINDOWEVENT_FOCUS_LOST:
    case SDL_WINDOWEVENT_CLOSE:
    case SDL_WINDOWEVENT_TAKE_FOCUS:
    case SDL_WINDOWEVENT_HIT_TEST:
        break;

    default:
        LOG_MESSAGE_F(warning, "Window %d got unknown sdl_event %d", sdl_event.window.windowID, sdl_event.window.event);
        break;
    }
}

void render::DisplayWindow::HandleSdlEvent(ThreadedLoopCommon& common, const SDL_Event& sdl_event) const {
    switch (sdl_event.type) {
    case SDL_WINDOWEVENT:
        HandleWindowEvent(common.renderController->renderer, common.gameController.event, sdl_event);
        break;

    case SDL_QUIT:
        common.gameState = ThreadedLoopCommon::GameState::quit;
        break;

        // Keyboard events
    case SDL_KEYUP:
    case SDL_KEYDOWN:
    {
        // Remove num and caps modifiers, they will not be considered modifiers
        int i_keymod = static_cast<int>(SDL_GetModState());
        i_keymod |= KMOD_NUM;
        i_keymod |= KMOD_CAPS;
        i_keymod -= static_cast<int>(KMOD_NUM);
        i_keymod -= static_cast<int>(KMOD_CAPS);

        const auto keycode      = static_cast<SDL_KeyCode>(sdl_event.key.keysym.sym);
        const auto input_action = game::InputManager::ToInputAction(sdl_event.key.type, sdl_event.key.repeat);
        const auto keymod       = static_cast<SDL_Keymod>(i_keymod);

        game::InputManager::SetInput(keycode, input_action, keymod);

        common.gameController.event.Raise<game::KeyboardActivityEvent>(
            game::EventType::keyboard_activity, keycode, input_action, keymod);
        common.gameController.event.Raise<game::InputActivityEvent>(
            game::EventType::input_activity, keycode, input_action, keymod);
    } break;

        // Mouse events
    case SDL_MOUSEMOTION:
        game::MouseSelection::SetCursor({sdl_event.motion.x, sdl_event.motion.y});
        break;
    case SDL_MOUSEWHEEL:
        if (!gui::input_mouse_captured) {
            constexpr auto near_zoom_sensitivity = 0.01f;
            constexpr auto far_zoom_sensitivity  = 0.04f;
            constexpr auto near_threshold        = 0.85f;

            const auto current_zoom = common.renderController->renderer.GetZoom();
            auto sensitivity        = far_zoom_sensitivity;
            if (current_zoom >= near_threshold) {
                sensitivity = near_zoom_sensitivity;
            }

            common.renderController->renderer.SetZoom(current_zoom + SafeCast<float>(sdl_event.wheel.y) * sensitivity);
        }
        break;
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEBUTTONDOWN:
    {
        game::InputAction action = game::InputAction::key_down;

        // Convert to InputAction, only button down and up supported
        if (sdl_event.button.state == SDL_RELEASED)
            action = game::InputAction::key_up;

        game::MouseInput mouse_input;
        switch (sdl_event.button.button) {
        case SDL_BUTTON_LEFT:
            mouse_input = game::MouseInput::left;
            break;
        case SDL_BUTTON_MIDDLE:
            mouse_input = game::MouseInput::middle;
            break;
        case SDL_BUTTON_RIGHT:
            mouse_input = game::MouseInput::right;
            break;
        case SDL_BUTTON_X1:
            mouse_input = game::MouseInput::x1;
            break;
        case SDL_BUTTON_X2:
            mouse_input = game::MouseInput::x2;
            break;

        default:
            assert(false);
            break;
        }

        game::InputManager::SetInput(mouse_input, action, KMOD_NONE);

        common.gameController.event.Raise<game::MouseActivityEvent>(
            game::EventType::mouse_activity, mouse_input, action, KMOD_NONE);
        common.gameController.event.Raise<game::InputActivityEvent>(
            game::EventType::input_activity, mouse_input, action, KMOD_NONE);
    } break;

    default:
        break;
    }
}
