//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	DOOM graphics stuff for SDL.
//

#include <cstring>

#include "SDL.h"
#include "SDL_opengl.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include "icon.cpp"

#include "crispy.hpp"

#include "lump.hpp"
#include "config.h"
#include "d_loop.hpp"
#include "deh_str.hpp"
#include "doomtype.hpp"
#include "i_input.hpp"
#include "i_joystick.hpp"
#include "i_system.hpp"
#include "i_timer.hpp"
#include "i_video.hpp"
#include "m_argv.hpp"
#include "m_config.hpp"
#include "m_misc.hpp"
#include "tables.hpp"
#include "v_diskicon.hpp"
#include "v_video.hpp"
#include "z_zone.hpp"

int SCREENWIDTH;
int SCREENHEIGHT;
[[maybe_unused]] int SCREENHEIGHT_4_3;
int HIRESWIDTH; // [crispy] non-widescreen SCREENWIDTH
int DELTAWIDTH; // [crispy] horizontal widescreen offset

// These are (1) the window (or the full screen) that our game is rendered to
// and (2) the renderer that scales the texture (see below) into this window.

static SDL_Window *  screen;
static SDL_Renderer *renderer;

// Window title

static const char *window_title = "";

// These are (1) the 320x200x8 paletted buffer that we draw to (i.e. the one
// that holds I_VideoBuffer), (2) the 320x200x32 RGBA intermediate buffer that
// we blit the former buffer to, (3) the intermediate 320x200 texture that we
// load the RGBA buffer to and that we render into another texture (4) which
// is upscaled by an integer factor UPSCALE using "nearest" scaling and which
// in turn is finally rendered to screen using "linear" scaling.

#ifndef CRISPY_TRUECOLOR
static SDL_Surface *screenbuffer = nullptr;
#endif
static SDL_Surface *argbbuffer       = nullptr;
static SDL_Texture *texture          = nullptr;
static SDL_Texture *texture_upscaled = nullptr;

#ifndef CRISPY_TRUECOLOR
static SDL_Rect blit_rect = {
    0,
    0,
    MAXWIDTH,
    MAXHEIGHT
};
#endif

static uint32_t pixel_format;

// palette

#ifdef CRISPY_TRUECOLOR
static SDL_Texture * curpane = nullptr;
static SDL_Texture * redpane = nullptr;
static SDL_Texture * yelpane = nullptr;
static SDL_Texture * grnpane = nullptr;
static int           pane_alpha;
static unsigned int  rmask, gmask, bmask, amask; // [crispy] moved up here
static const uint8_t blend_alpha = 0xa8;
extern pixel_t *     colormaps; // [crispy] evil hack to get FPS dots working as in Vanilla
#else
static SDL_Color palette[256];
#endif
static bool palette_to_set;

// display has been set up?

static bool initialized = false;

// disable mouse?

static bool nomouse  = false;
int            usemouse = 1;

// Save screenshots in PNG format.

int png_screenshots = 1; // [crispy]

// SDL display number on which to run.

int video_display = 0;

// Screen width and height, from configuration file.

int window_width  = 800;
int window_height = 600;

// Fullscreen mode, 0x0 for SDL_WINDOW_FULLSCREEN_DESKTOP.

int fullscreen_width = 0, fullscreen_height = 0;

// Maximum number of pixels to use for intermediate scale buffer.

static int max_scaling_buffer_pixels = 16000000;

static int actualheight;

// Time to wait for the screen to settle on startup before starting the
// game (ms)

static int startup_delay = 1000;

// Grab the mouse? (int type for config code). nograbmouse_override allows
// this to be temporarily disabled via the command line.

static int     grabmouse            = true;
static bool nograbmouse_override = false;

// If true, we display dots at the bottom of the screen to
// indicate FPS.

static bool display_fps_dots;

// If this is true, the screen is rendered but not blitted to the
// video buffer.

static bool noblit;

// Callback function to invoke to determine whether to grab the
// mouse pointer.

static grabmouse_callback_t grabmouse_callback = nullptr;

// Does the window currently have focus?

static bool window_focused = true;

// Window resize state.

static bool      need_resize = false;
static unsigned int last_resize_time;
constexpr auto RESIZE_DELAY = 500;

static i_video_t i_video_s = {
    .video_driver = const_cast<char *>(""),
    .screenvisible = true,

    .vanilla_keyboard_mapping = 0,
    .screensaver_mode = false,
    .usegamma = 0,
    .I_VideoBuffer = nullptr,

    .screen_width = 0,
    .screen_height = 0,
    .fullscreen = true,
    .aspect_ratio_correct = true,
    .integer_scaling = false,
    .vga_porch_flash = false,
    .force_software_renderer = false,

    .window_position = const_cast<char *>("center"),

    .joywait = 0,
};
i_video_t *const g_i_video_globals = &i_video_s;

static bool MouseShouldBeGrabbed()
{
    // never grab the mouse when in screensaver mode

    if (g_i_video_globals->screensaver_mode)
        return false;

    // if the window doesn't have focus, never grab it

    if (!window_focused)
        return false;

    // always grab the mouse when full screen (dont want to
    // see the mouse pointer)

    if (g_i_video_globals->fullscreen)
        return true;

    // Don't grab the mouse if mouse input is disabled

    if (!usemouse || nomouse)
        return false;

    // if we specify not to grab the mouse, never grab

    if (nograbmouse_override || !grabmouse)
        return false;

    // Invoke the grabmouse callback function to determine whether
    // the mouse should be grabbed

    if (grabmouse_callback != nullptr)
    {
        return grabmouse_callback();
    }
    else
    {
        return true;
    }
}

void I_SetGrabMouseCallback(grabmouse_callback_t func)
{
    grabmouse_callback = func;
}

// Set the variable controlling FPS dots.

void I_DisplayFPSDots(bool dots_on)
{
    display_fps_dots = dots_on;
}

static void SetShowCursor(bool show)
{
    if (!g_i_video_globals->screensaver_mode)
    {
        // When the cursor is hidden, grab the input.
        // Relative mode implicitly hides the cursor.
        SDL_SetRelativeMouseMode(static_cast<SDL_bool>(!show));
        SDL_GetRelativeMouseState(nullptr, nullptr);
    }
}

void I_ShutdownGraphics()
{
    if (initialized)
    {
        SetShowCursor(true);

        SDL_QuitSubSystem(SDL_INIT_VIDEO);

        initialized = false;
    }
}


//
// I_StartFrame
//
void I_StartFrame()
{
    // er?
}

// Adjust window_width / window_height variables to be an an aspect
// ratio consistent with the aspect_ratio_correct variable.
static void AdjustWindowSize()
{
    if (g_i_video_globals->aspect_ratio_correct || g_i_video_globals->integer_scaling)
    {
        if (window_width * actualheight <= window_height * SCREENWIDTH)
        {
            // We round up window_height if the ratio is not exact; this leaves
            // the result stable.
            window_height = (window_width * actualheight + SCREENWIDTH - 1) / SCREENWIDTH;
        }
        else
        {
            window_width = window_height * SCREENWIDTH / actualheight;
        }
    }
}

static void HandleWindowEvent(SDL_WindowEvent *event)
{
    int i;

    switch (event->event)
    {
#if 0 // SDL2-TODO
        case SDL_ACTIVEEVENT:
            // need to update our focus state
            UpdateFocus();
            break;
#endif
    case SDL_WINDOWEVENT_EXPOSED:
        palette_to_set = true;
        break;

    case SDL_WINDOWEVENT_RESIZED:
        need_resize      = true;
        last_resize_time = SDL_GetTicks();
        break;

        // Don't render the screen when the window is minimized:

    case SDL_WINDOWEVENT_MINIMIZED:
        g_i_video_globals->screenvisible = false;
        break;

    case SDL_WINDOWEVENT_MAXIMIZED:
    case SDL_WINDOWEVENT_RESTORED:
        g_i_video_globals->screenvisible = true;
        break;

        // Update the value of window_focused when we get a focus event
        //
        // We try to make ourselves be well-behaved: the grab on the mouse
        // is removed if we lose focus (such as a popup window appearing),
        // and we dont move the mouse around if we aren't focused either.

    case SDL_WINDOWEVENT_FOCUS_GAINED:
        window_focused = true;
        break;

    case SDL_WINDOWEVENT_FOCUS_LOST:
        window_focused = false;
        break;

        // We want to save the user's preferred monitor to use for running the
        // game, so that next time we're run we start on the same display. So
        // every time the window is moved, find which display we're now on and
        // update the video_display config variable.

    case SDL_WINDOWEVENT_MOVED:
        i = SDL_GetWindowDisplayIndex(screen);
        if (i >= 0)
        {
            video_display = i;
        }
        break;

    default:
        break;
    }
}

static bool ToggleFullScreenKeyShortcut(SDL_Keysym *sym)
{
    Uint16 flags = (KMOD_LALT | KMOD_RALT);
#if defined(__MACOSX__)
    flags |= (KMOD_LGUI | KMOD_RGUI);
#endif
    return (sym->scancode == SDL_SCANCODE_RETURN || sym->scancode == SDL_SCANCODE_KP_ENTER) && (sym->mod & flags) != 0;
}

static void I_ToggleFullScreen()
{
    unsigned int flags = 0;

    // TODO: Consider implementing fullscreen toggle for SDL_WINDOW_FULLSCREEN
    // (mode-changing) setup. This is hard because we have to shut down and
    // restart again.
    if (fullscreen_width != 0 || fullscreen_height != 0)
    {
        return;
    }

    g_i_video_globals->fullscreen = !g_i_video_globals->fullscreen;

    if (g_i_video_globals->fullscreen)
    {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    SDL_SetWindowFullscreen(screen, flags);

    if (!g_i_video_globals->fullscreen)
    {
        AdjustWindowSize();
        SDL_SetWindowSize(screen, window_width, window_height);
    }
}

void I_GetEvent()
{
    extern void I_HandleKeyboardEvent(SDL_Event * sdlevent);
    extern void I_HandleMouseEvent(SDL_Event * sdlevent);
    SDL_Event   sdlevent;

    SDL_PumpEvents();

    while (SDL_PollEvent(&sdlevent))
    {
        switch (sdlevent.type)
        {
        case SDL_KEYDOWN:
            if (ToggleFullScreenKeyShortcut(&sdlevent.key.keysym))
            {
                I_ToggleFullScreen();
                break;
            }
            // deliberate fall-though
            [[fallthrough]];

        case SDL_KEYUP:
            I_HandleKeyboardEvent(&sdlevent);
            break;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEWHEEL:
            if (usemouse && !nomouse && window_focused)
            {
                I_HandleMouseEvent(&sdlevent);
            }
            break;

        case SDL_QUIT:
            if (g_i_video_globals->screensaver_mode)
            {
                I_Quit();
            }
            else
            {
                event_t event;
                event.type = ev_quit;
                D_PostEvent(&event);
            }
            break;

        case SDL_WINDOWEVENT:
            if (sdlevent.window.windowID == SDL_GetWindowID(screen))
            {
                HandleWindowEvent(&sdlevent.window);
            }
            break;

        default:
            break;
        }
    }
}

//
// I_StartTic
//
void I_StartTic()
{
    if (!initialized)
    {
        return;
    }

    I_GetEvent();

    if (usemouse && !nomouse && window_focused)
    {
        I_ReadMouse();
    }

    if (static_cast<int>(g_i_video_globals->joywait) < I_GetTime())
    {
        I_UpdateJoystick();
    }
}


//
// I_UpdateNoBlit
//
void I_UpdateNoBlit()
{
    // what is this?
}

static void UpdateGrab()
{
    static bool currently_grabbed = false;
    bool        grab;

    grab = MouseShouldBeGrabbed();

    if (g_i_video_globals->screensaver_mode)
    {
        // Hide the cursor in screensaver mode

        SetShowCursor(false);
    }
    else if (grab && !currently_grabbed)
    {
        SetShowCursor(false);
    }
    else if (!grab && currently_grabbed)
    {
        int screen_w, screen_h;

        SetShowCursor(true);

        // When releasing the mouse from grab, warp the mouse cursor to
        // the bottom-right of the screen. This is a minimally distracting
        // place for it to appear - we may only have released the grab
        // because we're at an end of level intermission screen, for
        // example.

        SDL_GetWindowSize(screen, &screen_w, &screen_h);
        SDL_WarpMouseInWindow(screen, screen_w - 16, screen_h - 16);
        SDL_GetRelativeMouseState(nullptr, nullptr);
    }

    currently_grabbed = grab;
}

static void LimitTextureSize(int *w_upscale, int *h_upscale)
{
    SDL_RendererInfo rinfo;
    int              orig_w, orig_h;

    orig_w = *w_upscale;
    orig_h = *h_upscale;

    // Query renderer and limit to maximum texture dimensions of hardware:
    if (SDL_GetRendererInfo(renderer, &rinfo) != 0)
    {
        I_Error("CreateUpscaledTexture: SDL_GetRendererInfo() call failed: %s",
            SDL_GetError());
    }

    while (*w_upscale * SCREENWIDTH > rinfo.max_texture_width)
    {
        --*w_upscale;
    }
    while (*h_upscale * SCREENHEIGHT > rinfo.max_texture_height)
    {
        --*h_upscale;
    }

    if ((*w_upscale < 1 && rinfo.max_texture_width > 0) || (*h_upscale < 1 && rinfo.max_texture_height > 0))
    {
        I_Error("CreateUpscaledTexture: Can't create a texture big enough for "
                "the whole screen! Maximum texture size %dx%d",
            rinfo.max_texture_width, rinfo.max_texture_height);
    }

    // We limit the amount of texture memory used for the intermediate buffer,
    // since beyond a certain point there are diminishing returns. Also,
    // depending on the hardware there may be performance problems with very
    // huge textures, so the user can use this to reduce the maximum texture
    // size if desired.

    if (max_scaling_buffer_pixels < SCREENWIDTH * SCREENHEIGHT)
    {
        I_Error("CreateUpscaledTexture: max_scaling_buffer_pixels too small "
                "to create a texture buffer: %d < %d",
            max_scaling_buffer_pixels, SCREENWIDTH * SCREENHEIGHT);
    }

    while (*w_upscale * *h_upscale * SCREENWIDTH * SCREENHEIGHT
           > max_scaling_buffer_pixels)
    {
        if (*w_upscale > *h_upscale)
        {
            --*w_upscale;
        }
        else
        {
            --*h_upscale;
        }
    }

    if (*w_upscale != orig_w || *h_upscale != orig_h)
    {
        printf("CreateUpscaledTexture: Limited texture size to %dx%d "
               "(max %d pixels, max texture size %dx%d)\n",
            *w_upscale * SCREENWIDTH, *h_upscale * SCREENHEIGHT,
            max_scaling_buffer_pixels,
            rinfo.max_texture_width, rinfo.max_texture_height);
    }
}

static void CreateUpscaledTexture(bool force)
{
    int        w, h;
    int        h_upscale, w_upscale;
    static int h_upscale_old, w_upscale_old;

    SDL_Texture *new_texture, *old_texture;

    // Get the size of the renderer output. The units this gives us will be
    // real world pixels, which are not necessarily equivalent to the screen's
    // window size (because of highdpi).
    if (SDL_GetRendererOutputSize(renderer, &w, &h) != 0)
    {
        I_Error("Failed to get renderer output size: %s", SDL_GetError());
    }

    // When the screen or window dimensions do not match the aspect ratio
    // of the texture, the rendered area is scaled down to fit. Calculate
    // the actual dimensions of the rendered area.

    if (w * actualheight < h * SCREENWIDTH)
    {
        // Tall window.

        h = w * actualheight / SCREENWIDTH;
    }
    else
    {
        // Wide window.

        w = h * SCREENWIDTH / actualheight;
    }

    // Pick texture size the next integer multiple of the screen dimensions.
    // If one screen dimension matches an integer multiple of the original
    // resolution, there is no need to overscale in this direction.

    w_upscale = (w + SCREENWIDTH - 1) / SCREENWIDTH;
    h_upscale = (h + SCREENHEIGHT - 1) / SCREENHEIGHT;

    // Minimum texture dimensions of 320x200.

    if (w_upscale < 1)
    {
        w_upscale = 1;
    }
    if (h_upscale < 1)
    {
        h_upscale = 1;
    }

    LimitTextureSize(&w_upscale, &h_upscale);

    // Create a new texture only if the upscale factors have actually changed.

    if (h_upscale == h_upscale_old && w_upscale == w_upscale_old && !force)
    {
        return;
    }

    h_upscale_old = h_upscale;
    w_upscale_old = w_upscale;

    // Set the scaling quality for rendering the upscaled texture to "linear",
    // which looks much softer and smoother than "nearest" but does a better
    // job at downscaling from the upscaled texture to screen.

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    new_texture = SDL_CreateTexture(renderer,
        pixel_format,
        SDL_TEXTUREACCESS_TARGET,
        w_upscale * SCREENWIDTH,
        h_upscale * SCREENHEIGHT);

    old_texture      = texture_upscaled;
    texture_upscaled = new_texture;

    if (old_texture != nullptr)
    {
        SDL_DestroyTexture(old_texture);
    }
}

// [AM] Fractional part of the current tic, in the half-open
//      range of [0.0, 1.0).  Used for interpolation.
fixed_t fractionaltic;

//
// I_FinishUpdate
//
void I_FinishUpdate()
{
    static int lasttic;
    int        tics;
    int        i;

    if (!initialized)
        return;

    if (noblit)
        return;

    if (need_resize)
    {
        if (SDL_GetTicks() > last_resize_time + RESIZE_DELAY)
        {
            int flags;
            // When the window is resized (we're not in fullscreen mode),
            // save the new window size.
            flags = static_cast<int>(SDL_GetWindowFlags(screen));
            if ((flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == 0)
            {
                SDL_GetWindowSize(screen, &window_width, &window_height);

                // Adjust the window by resizing again so that the window
                // is the right aspect ratio.
                AdjustWindowSize();
                SDL_SetWindowSize(screen, window_width, window_height);
            }
            CreateUpscaledTexture(false);
            need_resize    = false;
            palette_to_set = true;
        }
        else
        {
            return;
        }
    }

    UpdateGrab();

#if 0 // SDL2-TODO
    // Don't update the screen if the window isn't visible.
    // Not doing this breaks under Windows when we alt-tab away 
    // while fullscreen.

    if (!(SDL_GetAppState() & SDL_APPACTIVE))
        return;
#endif

    // draws little dots on the bottom of the screen

    if (display_fps_dots)
    {
        i       = I_GetTime();
        tics    = i - lasttic;
        lasttic = i;
        if (tics > 20) tics = 20;

        for (i = 0; i < tics * 4; i += 4)
#ifndef CRISPY_TRUECOLOR
            g_i_video_globals->I_VideoBuffer[(SCREENHEIGHT - 1) * SCREENWIDTH + i] = 0xff;
#else
            g_i_video_globals->I_VideoBuffer[(SCREENHEIGHT - 1) * SCREENWIDTH + i] = colormaps[0xff];
#endif
        for (; i < 20 * 4; i += 4)
#ifndef CRISPY_TRUECOLOR
            g_i_video_globals->I_VideoBuffer[(SCREENHEIGHT - 1) * SCREENWIDTH + i] = 0x0;
#else
            g_i_video_globals->I_VideoBuffer[(SCREENHEIGHT - 1) * SCREENWIDTH + i] = colormaps[0x0];
#endif
    }

    // [crispy] [AM] Real FPS counter
    {
        static int lastmili;
        static int fpscount;
        int        mili;

        fpscount++;

        i    = static_cast<int>(SDL_GetTicks());
        mili = i - lastmili;

        // Update FPS counter every second
        if (mili >= 1000)
        {
            crispy->fps = (fpscount * 1000) / mili;
            fpscount    = 0;
            lastmili    = i;
        }
    }

    // Draw disk icon before blit, if necessary.
    V_DrawDiskIcon();

#ifndef CRISPY_TRUECOLOR
    if (palette_to_set)
    {
        SDL_SetPaletteColors(screenbuffer->format->palette, palette, 0, 256);
        palette_to_set = false;

        if (g_i_video_globals->vga_porch_flash)
        {
            // "flash" the pillars/letterboxes with palette changes, emulating
            // VGA "porch" behaviour (GitHub issue #832)
            SDL_SetRenderDrawColor(renderer, palette[0].r, palette[0].g,
                palette[0].b, SDL_ALPHA_OPAQUE);
        }
    }

    // Blit from the paletted 8-bit screen buffer to the intermediate
    // 32-bit RGBA buffer that we can load into the texture.

    SDL_LowerBlit(screenbuffer, &blit_rect, argbbuffer, &blit_rect);
#endif

    // Update the intermediate texture with the contents of the RGBA buffer.

    SDL_UpdateTexture(texture, nullptr, argbbuffer->pixels, argbbuffer->pitch);

    // Make sure the pillarboxes are kept clear each frame.

    SDL_RenderClear(renderer);

    if (crispy->smoothscaling)
    {
        // Render this intermediate texture into the upscaled texture
        // using "nearest" integer scaling.

        SDL_SetRenderTarget(renderer, texture_upscaled);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);

        // Finally, render this upscaled texture to screen using linear scaling.

        SDL_SetRenderTarget(renderer, nullptr);
        SDL_RenderCopy(renderer, texture_upscaled, nullptr, nullptr);
    }
    else
    {
        SDL_SetRenderTarget(renderer, nullptr);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    }

#ifdef CRISPY_TRUECOLOR
    if (curpane)
    {
        SDL_SetTextureAlphaMod(curpane, pane_alpha);
        SDL_RenderCopy(renderer, curpane, nullptr, nullptr);
    }
#endif

    // Draw!

    SDL_RenderPresent(renderer);

    // [AM] Figure out how far into the current tic we're in as a fixed_t.
    if (crispy->uncapped)
    {
        fractionaltic = I_GetTimeMS() * TICRATE % 1000 * FRACUNIT / 1000;
    }

    // Restore background and undo the disk indicator, if it was drawn.
    V_RestoreDiskBackground();
}


//
// I_ReadScreen
//
void I_ReadScreen(pixel_t *scr)
{
    std::memcpy(scr, g_i_video_globals->I_VideoBuffer, static_cast<unsigned long>(SCREENWIDTH * SCREENHEIGHT) * sizeof(*scr));
}


//
// I_SetPalette
//
// [crispy] intermediate gamma levels
uint8_t **gamma2table = nullptr;
void   I_SetGammaTable()
{
    int i;

    gamma2table = static_cast<uint8_t **>(malloc(9 * sizeof(*gamma2table)));

    // [crispy] 5 original gamma levels
    for (i = 0; i < 5; i++)
    {
        gamma2table[2 * i] = const_cast<uint8_t *>(gammatable[i]);
    }

    // [crispy] 4 intermediate gamma levels
    for (i = 0; i < 4; i++)
    {
        int j;

        gamma2table[2 * i + 1] = static_cast<uint8_t *>(malloc(256 * sizeof(**gamma2table)));

        for (j = 0; j < 256; j++)
        {
            gamma2table[2 * i + 1][j] = static_cast<uint8_t>((gamma2table[2 * i][j] + gamma2table[2 * i + 2][j]) / 2);
        }
    }
}

#ifndef CRISPY_TRUECOLOR
void I_SetPalette(uint8_t *doompalette)
{
    int i;

    // [crispy] intermediate gamma levels
    if (!gamma2table)
    {
        I_SetGammaTable();
    }

    for (i = 0; i < 256; ++i)
    {
        // Zero out the bottom two bits of each channel - the PC VGA
        // controller only supports 6 bits of accuracy.

        // [crispy] intermediate gamma levels
        int red      = gamma2table[g_i_video_globals->usegamma][*doompalette++] & ~3;
        palette[i].r = static_cast<Uint8>(red);
        int green    = gamma2table[g_i_video_globals->usegamma][*doompalette++] & ~3;
        palette[i].g = static_cast<Uint8>(green);
        int blue     = gamma2table[g_i_video_globals->usegamma][*doompalette++] & ~3;
        palette[i].b = static_cast<Uint8>(blue);
    }

    palette_to_set = true;
}

// Given an RGB value, find the closest matching palette index.

int I_GetPaletteIndex(int r, int g, int b)
{
    int best, best_diff, diff;
    int i;

    best      = 0;
    best_diff = INT_MAX;

    for (i = 0; i < 256; ++i)
    {
        diff = (r - palette[i].r) * (r - palette[i].r)
               + (g - palette[i].g) * (g - palette[i].g)
               + (b - palette[i].b) * (b - palette[i].b);

        if (diff < best_diff)
        {
            best      = i;
            best_diff = diff;
        }

        if (diff == 0)
        {
            break;
        }
    }

    return best;
}
#else
void I_SetPalette(int palette)
{
    switch (palette)
    {
    case 0:
        curpane = nullptr;
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
        curpane    = redpane;
        pane_alpha = 0xff * palette / 9;
        break;
    case 9:
    case 10:
    case 11:
    case 12:
        curpane    = yelpane;
        pane_alpha = 0xff * (palette - 8) / 8;
        break;
    case 13:
        curpane    = grnpane;
        pane_alpha = 0xff * 125 / 1000;
        break;
    default:
        I_Error("Unknown palette: %d!\n", palette);
        break;
    }
}
#endif

//
// Set the window title
//

void I_SetWindowTitle(const char *title)
{
    window_title = title;
}

//
// Call the SDL function to set the window title, based on
// the title set with I_SetWindowTitle.
//

void I_InitWindowTitle()
{
    char *buf;

    buf = M_StringJoin(window_title, " - ", PACKAGE_STRING, nullptr);
    SDL_SetWindowTitle(screen, buf);
    free(buf);
}

// Set the application icon

void I_InitWindowIcon()
{
    const unsigned int * const_ptr = icon_data;
    auto * ptr = const_cast<unsigned int *>(const_ptr);
    SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(reinterpret_cast<void*>(ptr), icon_w, icon_h,
        32, icon_w * 4, static_cast<Uint32>(0xff << 24), 0xff << 16,
        0xff << 8, 0xff << 0);
    SDL_SetWindowIcon(screen, surface);
    SDL_FreeSurface(surface);
}

// Set video size to a particular scale factor (1x, 2x, 3x, etc.)

static void SetScaleFactor(int factor)
{
    // Pick 320x200 or 320x240, depending on aspect ratio correct

    window_width  = factor * SCREENWIDTH;
    window_height = factor * actualheight;
    g_i_video_globals->fullscreen    = false;
}

void I_GraphicsCheckCommandLine()
{
    int i;

    //!
    // @category video
    // @vanilla
    //
    // Disable blitting the screen.
    //

    noblit = M_CheckParm("-noblit");

    //!
    // @category video
    //
    // Don't grab the mouse when running in windowed mode.
    //

    nograbmouse_override = M_ParmExists("-nograbmouse");

    // default to fullscreen mode, allow override with command line
    // nofullscreen because we love prboom

    //!
    // @category video
    //
    // Run in a window.
    //

    if (M_CheckParm("-window") || M_CheckParm("-nofullscreen"))
    {
        g_i_video_globals->fullscreen = false;
    }

    //!
    // @category video
    //
    // Run in fullscreen mode.
    //

    if (M_CheckParm("-fullscreen"))
    {
        g_i_video_globals->fullscreen = true;
    }

    //!
    // @category video
    //
    // Disable the mouse.
    //

    nomouse = M_CheckParm("-nomouse") > 0;

    //!
    // @category video
    // @arg <x>
    //
    // Specify the screen width, in pixels. Implies -window.
    //

    i = M_CheckParmWithArgs("-width", 1);

    if (i > 0)
    {
        window_width = std::atoi(myargv[i + 1]);
        g_i_video_globals->fullscreen   = false;
    }

    //!
    // @category video
    // @arg <y>
    //
    // Specify the screen height, in pixels. Implies -window.
    //

    i = M_CheckParmWithArgs("-height", 1);

    if (i > 0)
    {
        window_height = std::atoi(myargv[i + 1]);
        g_i_video_globals->fullscreen    = false;
    }

    //!
    // @category video
    // @arg <WxY>
    //
    // Specify the dimensions of the window. Implies -window.
    //

    i = M_CheckParmWithArgs("-geometry", 1);

    if (i > 0)
    {
        int w, h, s;

        s = sscanf(myargv[i + 1], "%ix%i", &w, &h);
        if (s == 2)
        {
            window_width  = w;
            window_height = h;
            g_i_video_globals->fullscreen    = false;
        }
    }

    //!
    // @category video
    //
    // Don't scale up the screen. Implies -window.
    //

    if (M_CheckParm("-1"))
    {
        SetScaleFactor(1);
    }

    //!
    // @category video
    //
    // Double up the screen to 2x its normal size. Implies -window.
    //

    if (M_CheckParm("-2"))
    {
        SetScaleFactor(2);
    }

    //!
    // @category video
    //
    // Double up the screen to 3x its normal size. Implies -window.
    //

    if (M_CheckParm("-3"))
    {
        SetScaleFactor(3);
    }
}

// Check if we have been invoked as a screensaver by xscreensaver.

void I_CheckIsScreensaver()
{
    char *env;

    env = getenv("XSCREENSAVER_WINDOW");

    if (env != nullptr)
    {
        g_i_video_globals->screensaver_mode = true;
    }
}

static void SetSDLVideoDriver()
{
    // Allow a default value for the SDL video driver to be specified
    // in the configuration file.

    if (strcmp(g_i_video_globals->video_driver, "") != 0)
    {
        char *env_string;

        env_string = M_StringJoin("SDL_VIDEODRIVER=", g_i_video_globals->video_driver, nullptr);
        putenv(env_string);
        free(env_string);
    }
}

// Check the display bounds of the display referred to by 'video_display' and
// set x and y to a location that places the window in the center of that
// display.
static void CenterWindow(int *x, int *y, int w, int h)
{
    SDL_Rect bounds;

    if (SDL_GetDisplayBounds(video_display, &bounds) < 0)
    {
        fprintf(stderr, "CenterWindow: Failed to read display bounds "
                        "for display #%d!\n",
            video_display);
        return;
    }

    *x = bounds.x + SDL_max((bounds.w - w) / 2, 0);
    *y = bounds.y + SDL_max((bounds.h - h) / 2, 0);
}

void I_GetWindowPosition(int *x, int *y, int w, int h)
{
    // Check that video_display corresponds to a display that really exists,
    // and if it doesn't, reset it.
    if (video_display < 0 || video_display >= SDL_GetNumVideoDisplays())
    {
        fprintf(stderr,
            "I_GetWindowPosition: We were configured to run on display #%d, "
            "but it no longer exists (max %d). Moving to display 0.\n",
            video_display, SDL_GetNumVideoDisplays() - 1);
        video_display = 0;
    }

    // in fullscreen mode, the window "position" still matters, because
    // we use it to control which display we run fullscreen on.

    if (g_i_video_globals->fullscreen)
    {
        CenterWindow(x, y, w, h);
        return;
    }

    // in windowed mode, the desired window position can be specified
    // in the configuration file.

    if (g_i_video_globals->window_position == nullptr || !strcmp(g_i_video_globals->window_position, ""))
    {
        *x = *y = SDL_WINDOWPOS_UNDEFINED;
    }
    else if (!strcmp(g_i_video_globals->window_position, "center"))
    {
        // Note: SDL has a SDL_WINDOWPOS_CENTER, but this is useless for our
        // purposes, since we also want to control which display we appear on.
        // So we have to do this ourselves.
        CenterWindow(x, y, w, h);
    }
    else if (sscanf(g_i_video_globals->window_position, "%i,%i", x, y) != 2)
    {
        // invalid format: revert to default
        fprintf(stderr, "I_GetWindowPosition: invalid window_position setting\n");
        *x = *y = SDL_WINDOWPOS_UNDEFINED;
    }
}

static void SetVideoMode()
{
    int w, h;
    int x, y;
#ifndef CRISPY_TRUECOLOR
    unsigned int rmask, gmask, bmask, amask;
#endif
    int             unused_bpp;
    int             window_flags = 0, renderer_flags = 0;
    SDL_DisplayMode mode;

    w = window_width;
    h = window_height;

    // In windowed mode, the window can be resized while the game is
    // running.
    window_flags = SDL_WINDOW_RESIZABLE;

    // Set the highdpi flag - this makes a big difference on Macs with
    // retina displays, especially when using small window sizes.
    window_flags |= SDL_WINDOW_ALLOW_HIGHDPI;

    if (g_i_video_globals->fullscreen)
    {
        if (fullscreen_width == 0 && fullscreen_height == 0)
        {
            // This window_flags means "Never change the screen resolution!
            // Instead, draw to the entire screen by scaling the texture
            // appropriately".
            window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        }
        else
        {
            w = fullscreen_width;
            h = fullscreen_height;
            window_flags |= SDL_WINDOW_FULLSCREEN;
        }
    }

    // Running without window decorations is potentially useful if you're
    // playing in three window mode and want to line up three game windows
    // next to each other on a single desktop.
    // Deliberately not documented because I'm not sure how useful this is yet.
    if (M_ParmExists("-borderless"))
    {
        window_flags |= SDL_WINDOW_BORDERLESS;
    }

    I_GetWindowPosition(&x, &y, w, h);

    // Create window and renderer contexts. We set the window title
    // later anyway and leave the window position "undefined". If
    // "window_flags" contains the fullscreen flag (see above), then
    // w and h are ignored.

    if (screen == nullptr)
    {
        screen = SDL_CreateWindow(nullptr, x, y, w, h, static_cast<Uint32>(window_flags));

        if (screen == nullptr)
        {
            I_Error("Error creating window for video startup: %s",
                SDL_GetError());
        }

        pixel_format = SDL_GetWindowPixelFormat(screen);

        SDL_SetWindowMinimumSize(screen, SCREENWIDTH, actualheight);

        I_InitWindowTitle();
        I_InitWindowIcon();
    }

    // The SDL_RENDERER_TARGETTEXTURE flag is required to render the
    // intermediate texture into the upscaled texture.
    renderer_flags = SDL_RENDERER_TARGETTEXTURE;

    if (SDL_GetCurrentDisplayMode(video_display, &mode) != 0)
    {
        I_Error("Could not get display mode for video display #%d: %s",
            video_display, SDL_GetError());
    }

    // Turn on vsync if we aren't in a -timedemo
    if (!singletics && mode.refresh_rate > 0)
    {
        if (crispy->vsync) // [crispy] uncapped vsync
        {
            renderer_flags |= SDL_RENDERER_PRESENTVSYNC;
        }
    }

    if (g_i_video_globals->force_software_renderer)
    {
        renderer_flags |= SDL_RENDERER_SOFTWARE;
        renderer_flags &= ~SDL_RENDERER_PRESENTVSYNC;
        crispy->vsync = false;
    }

    if (renderer != nullptr)
    {
        SDL_DestroyRenderer(renderer);
        // all associated textures get destroyed
        texture          = nullptr;
        texture_upscaled = nullptr;
    }

    renderer = SDL_CreateRenderer(screen, -1, static_cast<Uint32>(renderer_flags));

    // If we could not find a matching render driver,
    // try again without hardware acceleration.

    if (renderer == nullptr && !g_i_video_globals->force_software_renderer)
    {
        renderer_flags |= SDL_RENDERER_SOFTWARE;
        renderer_flags &= ~SDL_RENDERER_PRESENTVSYNC;

        renderer = SDL_CreateRenderer(screen, -1, static_cast<Uint32>(renderer_flags));

        // If this helped, save the setting for later.
        if (renderer != nullptr)
        {
            g_i_video_globals->force_software_renderer = 1;
        }
    }

    if (renderer == nullptr)
    {
        I_Error("Error creating renderer for screen window: %s",
            SDL_GetError());
    }

    // Important: Set the "logical size" of the rendering context. At the same
    // time this also defines the aspect ratio that is preserved while scaling
    // and stretching the texture into the window.

    if (g_i_video_globals->aspect_ratio_correct || g_i_video_globals->integer_scaling)
    {
        SDL_RenderSetLogicalSize(renderer,
            SCREENWIDTH,
            actualheight);
    }

    // Force integer scales for resolution-independent rendering.

#if SDL_VERSION_ATLEAST(2, 0, 5)
    SDL_RenderSetIntegerScale(renderer, static_cast<SDL_bool>(g_i_video_globals->integer_scaling));
#endif

    // Blank out the full screen area in case there is any junk in
    // the borders that won't otherwise be overwritten.

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

#ifndef CRISPY_TRUECOLOR
    // Create the 8-bit paletted and the 32-bit RGBA screenbuffer surfaces.

    if (screenbuffer != nullptr)
    {
        SDL_FreeSurface(screenbuffer);
        screenbuffer = nullptr;
    }

    if (screenbuffer == nullptr)
    {
        screenbuffer = SDL_CreateRGBSurface(0,
            SCREENWIDTH, SCREENHEIGHT, 8,
            0, 0, 0, 0);
        SDL_FillRect(screenbuffer, nullptr, 0);
    }
#endif

    // Format of argbbuffer must match the screen pixel format because we
    // import the surface data into the texture.

    if (argbbuffer != nullptr)
    {
        SDL_FreeSurface(argbbuffer);
        argbbuffer = nullptr;
    }

    if (argbbuffer == nullptr)
    {
        SDL_PixelFormatEnumToMasks(pixel_format, &unused_bpp,
            &rmask, &gmask, &bmask, &amask);
        argbbuffer = SDL_CreateRGBSurface(0,
            SCREENWIDTH, SCREENHEIGHT, 32,
            rmask, gmask, bmask, amask);
#ifdef CRISPY_TRUECOLOR
        SDL_FillRect(argbbuffer, nullptr, I_MapRGB(0xff, 0x0, 0x0));
        redpane = SDL_CreateTextureFromSurface(renderer, argbbuffer);
        SDL_SetTextureBlendMode(redpane, SDL_BLENDMODE_BLEND);

        SDL_FillRect(argbbuffer, nullptr, I_MapRGB(0xd7, 0xba, 0x45));
        yelpane = SDL_CreateTextureFromSurface(renderer, argbbuffer);
        SDL_SetTextureBlendMode(yelpane, SDL_BLENDMODE_BLEND);

        SDL_FillRect(argbbuffer, nullptr, I_MapRGB(0x0, 0xff, 0x0));
        grnpane = SDL_CreateTextureFromSurface(renderer, argbbuffer);
        SDL_SetTextureBlendMode(grnpane, SDL_BLENDMODE_BLEND);
#endif
        SDL_FillRect(argbbuffer, nullptr, 0);
    }

    if (texture != nullptr)
    {
        SDL_DestroyTexture(texture);
    }

    // Set the scaling quality for rendering the intermediate texture into
    // the upscaled texture to "nearest", which is gritty and pixelated and
    // resembles software scaling pretty well.

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

    // Create the intermediate texture that the RGBA surface gets loaded into.
    // The SDL_TEXTUREACCESS_STREAMING flag means that this texture's content
    // is going to change frequently.

    texture = SDL_CreateTexture(renderer,
        pixel_format,
        SDL_TEXTUREACCESS_STREAMING,
        SCREENWIDTH, SCREENHEIGHT);

    // Initially create the upscaled texture for rendering to screen

    CreateUpscaledTexture(true);
}

// [crispy] re-calculate SCREENWIDTH, SCREENHEIGHT, HIRESWIDTH and DELTAWIDTH
void I_GetScreenDimensions()
{
    SDL_DisplayMode mode;
    int             w = 16, h = 10;
    int             ah;

    SCREENWIDTH  = ORIGWIDTH << crispy->hires;
    SCREENHEIGHT = ORIGHEIGHT << crispy->hires;

    HIRESWIDTH = SCREENWIDTH;

    ah = (g_i_video_globals->aspect_ratio_correct == 1) ? (6 * SCREENHEIGHT / 5) : SCREENHEIGHT;

    if (SDL_GetCurrentDisplayMode(video_display, &mode) == 0)
    {
        // [crispy] sanity check: really widescreen display?
        if (mode.w * ah >= mode.h * SCREENWIDTH)
        {
            w = mode.w;
            h = mode.h;
        }
    }

    // [crispy] widescreen rendering makes no sense without aspect ratio correction
    if (crispy->widescreen && g_i_video_globals->aspect_ratio_correct)
    {
        SCREENWIDTH = w * ah / h;
        // [crispy] make sure SCREENWIDTH is an integer multiple of 4 ...
        SCREENWIDTH = (SCREENWIDTH + 3) & static_cast<int>(~3);
        // [crispy] ... but never exceeds MAXWIDTH (array size!)
        SCREENWIDTH = MIN(SCREENWIDTH, MAXWIDTH);
    }

    DELTAWIDTH = ((SCREENWIDTH - HIRESWIDTH) >> crispy->hires) / 2;
}

void I_InitGraphics()
{
    SDL_Event dummy;
#ifndef CRISPY_TRUECOLOR
    uint8_t *doompal;
#endif
    char *env;

    // Pass through the XSCREENSAVER_WINDOW environment variable to
    // SDL_WINDOWID, to embed the SDL window into the Xscreensaver
    // window.

    env = getenv("XSCREENSAVER_WINDOW");

    if (env != nullptr)
    {
        char winenv[30];
        int  winid;

        sscanf(env, "0x%x", &winid);
        M_snprintf(winenv, sizeof(winenv), "SDL_WINDOWID=%i", winid);

        putenv(winenv);
    }

    SetSDLVideoDriver();

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        I_Error("Failed to initialize video: %s", SDL_GetError());
    }

    // When in screensaver mode, run full screen and auto detect
    // screen dimensions (don't change video mode)
    if (g_i_video_globals->screensaver_mode)
    {
        g_i_video_globals->fullscreen = true;
    }

    // [crispy] run-time variable high-resolution rendering
    I_GetScreenDimensions();

#ifndef CRISPY_TRUECOLOR
    blit_rect.w = SCREENWIDTH;
    blit_rect.h = SCREENHEIGHT;
#endif

    // [crispy] (re-)initialize resolution-agnostic patch drawing
    V_Init();

    if (g_i_video_globals->aspect_ratio_correct == 1)
    {
        actualheight = 6 * SCREENHEIGHT / 5;
    }
    else
    {
        actualheight = SCREENHEIGHT;
    }

    // Create the game window; this may switch graphic modes depending
    // on configuration.
    AdjustWindowSize();
    SetVideoMode();

#ifndef CRISPY_TRUECOLOR
    // Start with a clear black screen
    // (screen will be flipped after we set the palette)

    SDL_FillRect(screenbuffer, nullptr, 0);

    // Set the palette

    doompal = cache_lump_name<uint8_t *>(DEH_String("PLAYPAL"), PU_CACHE);
    I_SetPalette(doompal);
    SDL_SetPaletteColors(screenbuffer->format->palette, palette, 0, 256);
#endif

    // SDL2-TODO UpdateFocus();
    UpdateGrab();

    // On some systems, it takes a second or so for the screen to settle
    // after changing modes.  We include the option to add a delay when
    // setting the screen mode, so that the game doesn't start immediately
    // with the player unable to see anything.

    if (g_i_video_globals->fullscreen && !g_i_video_globals->screensaver_mode)
    {
        SDL_Delay(static_cast<Uint32>(startup_delay));
    }

    // The actual 320x200 canvas that we draw to. This is the pixel buffer of
    // the 8-bit paletted screen buffer that gets blit on an intermediate
    // 32-bit RGBA screen buffer that gets loaded into a texture that gets
    // finally rendered into our window or full screen in I_FinishUpdate().

#ifndef CRISPY_TRUECOLOR
    g_i_video_globals->I_VideoBuffer = static_cast<pixel_t *>(screenbuffer->pixels);
#else
    g_i_video_globals->I_VideoBuffer = argbbuffer->pixels;
#endif
    V_RestoreBuffer();

    // Clear the screen to black.

    std::memset(g_i_video_globals->I_VideoBuffer, 0, static_cast<unsigned long>(SCREENWIDTH * SCREENHEIGHT) * sizeof(*g_i_video_globals->I_VideoBuffer));

    // clear out any events waiting at the start and center the mouse

    while (SDL_PollEvent(&dummy))
        ;

    initialized = true;

    // Call I_ShutdownGraphics on quit

    I_AtExit(I_ShutdownGraphics, true);
}

// [crispy] re-initialize only the parts of the rendering stack that are really necessary

void I_ReInitGraphics(int reinit)
{
    // [crispy] re-set rendering resolution and re-create framebuffers
    if (reinit & REINIT_FRAMEBUFFERS)
    {
        unsigned int rmask, gmask, bmask, amask;
        int          unused_bpp;

        I_GetScreenDimensions();

#ifndef CRISPY_TRUECOLOR
        blit_rect.w = SCREENWIDTH;
        blit_rect.h = SCREENHEIGHT;
#endif

        // [crispy] re-initialize resolution-agnostic patch drawing
        V_Init();

#ifndef CRISPY_TRUECOLOR
        SDL_FreeSurface(screenbuffer);
        screenbuffer = SDL_CreateRGBSurface(0,
            SCREENWIDTH, SCREENHEIGHT, 8,
            0, 0, 0, 0);
#endif

        SDL_FreeSurface(argbbuffer);
        SDL_PixelFormatEnumToMasks(pixel_format, &unused_bpp,
            &rmask, &gmask, &bmask, &amask);
        argbbuffer = SDL_CreateRGBSurface(0,
            SCREENWIDTH, SCREENHEIGHT, 32,
            rmask, gmask, bmask, amask);
#ifndef CRISPY_TRUECOLOR
        // [crispy] re-set the framebuffer pointer
        g_i_video_globals->I_VideoBuffer = static_cast<pixel_t *>(screenbuffer->pixels);
#else
        g_i_video_globals->I_VideoBuffer = argbbuffer->pixels;
#endif
        V_RestoreBuffer();

        // [crispy] it will get re-created below with the new resolution
        SDL_DestroyTexture(texture);
    }

    // [crispy] re-create renderer
    if (reinit & REINIT_RENDERER)
    {
        SDL_RendererInfo info = {};
        int              flags;

        SDL_GetRendererInfo(renderer, &info);
        flags = static_cast<int>(info.flags);

        if (crispy->vsync && !(flags & SDL_RENDERER_SOFTWARE))
        {
            flags |= SDL_RENDERER_PRESENTVSYNC;
        }
        else
        {
            flags &= ~SDL_RENDERER_PRESENTVSYNC;
        }

        SDL_DestroyRenderer(renderer);
        renderer = SDL_CreateRenderer(screen, -1, static_cast<Uint32>(flags));
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

        // [crispy] the texture gets destroyed in SDL_DestroyRenderer(), force its re-creation
        texture_upscaled = nullptr;
    }

    // [crispy] re-create textures
    if (reinit & REINIT_TEXTURES)
    {
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

        texture = SDL_CreateTexture(renderer,
            pixel_format,
            SDL_TEXTUREACCESS_STREAMING,
            SCREENWIDTH, SCREENHEIGHT);

        // [crispy] force its re-creation
        CreateUpscaledTexture(true);
    }

    // [crispy] re-set logical rendering resolution
    if (reinit & REINIT_ASPECTRATIO)
    {
        if (g_i_video_globals->aspect_ratio_correct == 1)
        {
            actualheight = 6 * SCREENHEIGHT / 5;
        }
        else
        {
            actualheight = SCREENHEIGHT;
        }

        if (g_i_video_globals->aspect_ratio_correct || g_i_video_globals->integer_scaling)
        {
            SDL_RenderSetLogicalSize(renderer,
                SCREENWIDTH,
                actualheight);
        }
        else
        {
            SDL_RenderSetLogicalSize(renderer, 0, 0);
        }

#if SDL_VERSION_ATLEAST(2, 0, 5)
        SDL_RenderSetIntegerScale(renderer, static_cast<SDL_bool>(g_i_video_globals->integer_scaling));
#endif
    }

    // [crispy] adjust the window size and re-set the palette
    need_resize = true;
}

// [crispy] take screenshot of the rendered image

void I_RenderReadPixels(uint8_t **data, int *w, int *h, int *p)
{
    SDL_Rect         rect;
    SDL_PixelFormat *format;
    int              temp;
    uint32_t         png_format;
    uint8_t         *pixels;

    // [crispy] adjust cropping rectangle if necessary
    rect.x = rect.y = 0;
    SDL_GetRendererOutputSize(renderer, &rect.w, &rect.h);
    if (g_i_video_globals->aspect_ratio_correct || g_i_video_globals->integer_scaling)
    {
        if (g_i_video_globals->integer_scaling)
        {
            int temp1, temp2, scale;
            temp1 = rect.w;
            temp2 = rect.h;
            scale = MIN(rect.w / SCREENWIDTH, rect.h / actualheight);

            rect.w = SCREENWIDTH * scale;
            rect.h = actualheight * scale;

            rect.x = (temp1 - rect.w) / 2;
            rect.y = (temp2 - rect.h) / 2;
        }
        else if (rect.w * actualheight > rect.h * SCREENWIDTH)
        {
            temp   = rect.w;
            rect.w = rect.h * SCREENWIDTH / actualheight;
            rect.x = (temp - rect.w) / 2;
        }
        else if (rect.h * SCREENWIDTH > rect.w * actualheight)
        {
            temp   = rect.h;
            rect.h = rect.w * actualheight / SCREENWIDTH;
            rect.y = (temp - rect.h) / 2;
        }
    }

    // [crispy] native PNG pixel format
#if SDL_VERSION_ATLEAST(2, 0, 5)
    png_format = SDL_PIXELFORMAT_RGB24;
#else
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    png_format = SDL_PIXELFORMAT_ABGR8888;
#else
    png_format = SDL_PIXELFORMAT_RGBA8888;
#endif
#endif
    format = SDL_AllocFormat(png_format);
    temp   = rect.w * format->BytesPerPixel; // [crispy] pitch

    // [crispy] As far as I understand the issue, SDL_RenderPresent()
    // may return early, i.e. before it has actually finished rendering the
    // current texture to screen -- from where we want to capture it.
    // However, it does never return before it has finished rendering the
    // *previous* texture.
    // Thus, we add a second call to SDL_RenderPresent() here to make sure
    // that it has at least finished rendering the previous texture, which
    // already contains the scene that we actually want to capture.
    if (crispy->post_rendering_hook)
    {
        SDL_RenderCopy(renderer, crispy->smoothscaling ? texture_upscaled : texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    // [crispy] allocate memory for screenshot image
    pixels = static_cast<uint8_t *>(malloc(static_cast<size_t>(rect.h * temp)));
    SDL_RenderReadPixels(renderer, &rect, format->format, pixels, temp);

    *data = pixels;
    *w    = rect.w;
    *h    = rect.h;
    *p    = temp;

    SDL_FreeFormat(format);
}

// Bind all variables controlling video options into the configuration
// file system.
void I_BindVideoVariables()
{
    M_BindIntVariable("use_mouse", &usemouse);
    M_BindIntVariable("fullscreen", &g_i_video_globals->fullscreen);
    M_BindIntVariable("video_display", &video_display);
    M_BindIntVariable("aspect_ratio_correct", &g_i_video_globals->aspect_ratio_correct);
    M_BindIntVariable("integer_scaling", &g_i_video_globals->integer_scaling);
    M_BindIntVariable("vga_porch_flash", &g_i_video_globals->vga_porch_flash);
    M_BindIntVariable("startup_delay", &startup_delay);
    M_BindIntVariable("fullscreen_width", &fullscreen_width);
    M_BindIntVariable("fullscreen_height", &fullscreen_height);
    M_BindIntVariable("force_software_renderer", &g_i_video_globals->force_software_renderer);
    M_BindIntVariable("max_scaling_buffer_pixels", &max_scaling_buffer_pixels);
    M_BindIntVariable("window_width", &window_width);
    M_BindIntVariable("window_height", &window_height);
    M_BindIntVariable("grabmouse", &grabmouse);
    M_BindStringVariable("video_driver", &g_i_video_globals->video_driver);
    M_BindStringVariable("window_position", &g_i_video_globals->window_position);
    M_BindIntVariable("usegamma", &g_i_video_globals->usegamma);
    M_BindIntVariable("png_screenshots", &png_screenshots);
}

#ifdef CRISPY_TRUECOLOR
const pixel_t I_BlendAdd(const pixel_t bg, const pixel_t fg)
{
    uint32_t r, g, b;

    if ((r = (fg & rmask) + (bg & rmask)) > rmask) r = rmask;
    if ((g = (fg & gmask) + (bg & gmask)) > gmask) g = gmask;
    if ((b = (fg & bmask) + (bg & bmask)) > bmask) b = bmask;

    return amask | r | g | b;
}

// [crispy] http://stereopsis.com/doubleblend.html
const pixel_t I_BlendDark(const pixel_t bg, const int d)
{
    const uint32_t ag = (bg & 0xff00ff00) >> 8;
    const uint32_t rb = bg & 0x00ff00ff;

    uint32_t sag = d * ag;
    uint32_t srb = d * rb;

    sag = sag & 0xff00ff00;
    srb = (srb >> 8) & 0x00ff00ff;

    return amask | sag | srb;
}

const pixel_t I_BlendOver(const pixel_t bg, const pixel_t fg)
{
    const uint32_t r = ((blend_alpha * (fg & rmask) + (0xff - blend_alpha) * (bg & rmask)) >> 8) & rmask;
    const uint32_t g = ((blend_alpha * (fg & gmask) + (0xff - blend_alpha) * (bg & gmask)) >> 8) & gmask;
    const uint32_t b = ((blend_alpha * (fg & bmask) + (0xff - blend_alpha) * (bg & bmask)) >> 8) & bmask;

    return amask | r | g | b;
}

const pixel_t (*blendfunc)(const pixel_t fg, const pixel_t bg) = I_BlendOver;

const pixel_t I_MapRGB(const uint8_t r, const uint8_t g, const uint8_t b)
{
    /*
	return amask |
	        (((r * rmask) >> 8) & rmask) |
	        (((g * gmask) >> 8) & gmask) |
	        (((b * bmask) >> 8) & bmask);
*/
    return SDL_MapRGB(argbbuffer->format, r, g, b);
}
#endif
