/* ****************************************************************************
 *
 * ANTI-CAPITALIST SOFTWARE LICENSE (v 1.4)
 *
 * Copyright © 2023 erysdren (it/they/she)
 *
 * This is anti-capitalist software, released for free use by individuals
 * and organizations that do not operate by capitalist principles.
 *
 * Permission is hereby granted, free of charge, to any person or
 * organization (the "User") obtaining a copy of this software and
 * associated documentation files (the "Software"), to use, copy, modify,
 * merge, distribute, and/or sell copies of the Software, subject to the
 * following conditions:
 *
 *   1. The above copyright notice and this permission notice shall be
 *   included in all copies or modified versions of the Software.
 *
 *   2. The User is one of the following:
 *     a. An individual person, laboring for themselves
 *     b. A non-profit organization
 *     c. An educational institution
 *     d. An organization that seeks shared profit for all of its members,
 *     and allows non-members to set the cost of their labor
 *
 *   3. If the User is an organization with owners, then all owners are
 *   workers and all workers are owners with equal equity and/or equal vote.
 *
 *   4. If the User is an organization, then the User is not law enforcement
 *   or military, or working for or under either.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT EXPRESS OR IMPLIED WARRANTY OF
 * ANY KIND, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * ************************************************************************* */

/*
 * std
 */

#include <stdint.h>

/*
 * keyboard scancodes
 */

#if PLATFORM_SDL2

#include <SDL2/SDL.h>

enum
{
	KEY_ESCAPE = SDL_SCANCODE_ESCAPE,
	KEY_1 = SDL_SCANCODE_1,
	KEY_2 = SDL_SCANCODE_2,
	KEY_3 = SDL_SCANCODE_3,
	KEY_4 = SDL_SCANCODE_4,
	KEY_5 = SDL_SCANCODE_5,
	KEY_6 = SDL_SCANCODE_6,
	KEY_7 = SDL_SCANCODE_7,
	KEY_8 = SDL_SCANCODE_8,
	KEY_9 = SDL_SCANCODE_9,
	KEY_0 = SDL_SCANCODE_0,
	KEY_MINUS = SDL_SCANCODE_MINUS,
	KEY_EQUALS = SDL_SCANCODE_EQUALS,
	KEY_BACKSPACE = SDL_SCANCODE_BACKSPACE,
	KEY_TAB = SDL_SCANCODE_TAB,
	KEY_Q = SDL_SCANCODE_Q,
	KEY_W = SDL_SCANCODE_W,
	KEY_E = SDL_SCANCODE_E,
	KEY_R = SDL_SCANCODE_R,
	KEY_T = SDL_SCANCODE_T,
	KEY_Y = SDL_SCANCODE_Y,
	KEY_U = SDL_SCANCODE_U,
	KEY_I = SDL_SCANCODE_I,
	KEY_O = SDL_SCANCODE_O,
	KEY_P = SDL_SCANCODE_P,
	KEY_LEFTBRACKET = SDL_SCANCODE_LEFTBRACKET,
	KEY_RIGHTBRACKET = SDL_SCANCODE_RIGHTBRACKET,
	KEY_ENTER = SDL_SCANCODE_RETURN,
	KEY_CONTROL = SDL_SCANCODE_LCTRL,
	KEY_A = SDL_SCANCODE_A,
	KEY_S = SDL_SCANCODE_S,
	KEY_D = SDL_SCANCODE_D,
	KEY_F = SDL_SCANCODE_F,
	KEY_G = SDL_SCANCODE_G,
	KEY_H = SDL_SCANCODE_H,
	KEY_J = SDL_SCANCODE_J,
	KEY_K = SDL_SCANCODE_K,
	KEY_L = SDL_SCANCODE_L,
	KEY_SEMICOLON = SDL_SCANCODE_SEMICOLON,
	KEY_QUOTE = SDL_SCANCODE_APOSTROPHE,
	KEY_TILDE = SDL_SCANCODE_GRAVE,
	KEY_LSHIFT = SDL_SCANCODE_LSHIFT,
	KEY_BACKSLASH = SDL_SCANCODE_BACKSLASH,
	KEY_Z = SDL_SCANCODE_Z,
	KEY_X = SDL_SCANCODE_X,
	KEY_C = SDL_SCANCODE_C,
	KEY_V = SDL_SCANCODE_V,
	KEY_B = SDL_SCANCODE_B,
	KEY_N = SDL_SCANCODE_N,
	KEY_M = SDL_SCANCODE_M,
	KEY_COMMA = SDL_SCANCODE_COMMA,
	KEY_PERIOD = SDL_SCANCODE_PERIOD,
	KEY_SLASH = SDL_SCANCODE_SLASH,
	KEY_RSHIFT = SDL_SCANCODE_RSHIFT,
	KEY_MULTIPLY = SDL_SCANCODE_KP_MULTIPLY,
	KEY_ALT = SDL_SCANCODE_LALT,
	KEY_SPACE = SDL_SCANCODE_SPACE,
	KEY_CAPSLOCK = SDL_SCANCODE_CAPSLOCK,
	KEY_F1 = SDL_SCANCODE_F1,
	KEY_F2 = SDL_SCANCODE_F2,
	KEY_F3 = SDL_SCANCODE_F3,
	KEY_F4 = SDL_SCANCODE_F4,
	KEY_F5 = SDL_SCANCODE_F5,
	KEY_F6 = SDL_SCANCODE_F6,
	KEY_F7 = SDL_SCANCODE_F7,
	KEY_F8 = SDL_SCANCODE_F8,
	KEY_F9 = SDL_SCANCODE_F9,
	KEY_F10 = SDL_SCANCODE_F10,
	KEY_NUMLOCK = SDL_SCANCODE_NUMLOCKCLEAR,
	KEY_SCROLLLOCK = SDL_SCANCODE_SCROLLLOCK,
	KEY_HOME = SDL_SCANCODE_HOME,
	KEY_UP = SDL_SCANCODE_UP,
	KEY_PAGEUP = SDL_SCANCODE_PAGEUP,
	KEY_LEFT = SDL_SCANCODE_LEFT,
	KEY_RIGHT = SDL_SCANCODE_RIGHT,
	KEY_PLUS = SDL_SCANCODE_KP_PLUS,
	KEY_END = SDL_SCANCODE_END,
	KEY_DOWN = SDL_SCANCODE_DOWN,
	KEY_PAGEDOWN = SDL_SCANCODE_PAGEDOWN,
	KEY_INSERT = SDL_SCANCODE_INSERT,
	KEY_DELETE = SDL_SCANCODE_DELETE,
	KEY_F11 = SDL_SCANCODE_F11,
	KEY_F12 = SDL_SCANCODE_F12
};

#elif PLATFORM_SDL

#include <SDL.h>

enum
{
	KEY_ESCAPE = SDLK_ESCAPE,
	KEY_1 = SDLK_1,
	KEY_2 = SDLK_2,
	KEY_3 = SDLK_3,
	KEY_4 = SDLK_4,
	KEY_5 = SDLK_5,
	KEY_6 = SDLK_6,
	KEY_7 = SDLK_7,
	KEY_8 = SDLK_8,
	KEY_9 = SDLK_9,
	KEY_0 = SDLK_0,
	KEY_MINUS = SDLK_MINUS,
	KEY_EQUALS = SDLK_EQUALS,
	KEY_BACKSPACE = SDLK_BACKSPACE,
	KEY_TAB = SDLK_TAB,
	KEY_Q = SDLK_q,
	KEY_W = SDLK_w,
	KEY_E = SDLK_e,
	KEY_R = SDLK_r,
	KEY_T = SDLK_t,
	KEY_Y = SDLK_y,
	KEY_U = SDLK_u,
	KEY_I = SDLK_i,
	KEY_O = SDLK_o,
	KEY_P = SDLK_p,
	KEY_LEFTBRACKET = SDLK_LEFTBRACKET,
	KEY_RIGHTBRACKET = SDLK_RIGHTBRACKET,
	KEY_ENTER = SDLK_RETURN,
	KEY_CONTROL = SDLK_LCTRL,
	KEY_A = SDLK_a,
	KEY_S = SDLK_s,
	KEY_D = SDLK_d,
	KEY_F = SDLK_f,
	KEY_G = SDLK_g,
	KEY_H = SDLK_h,
	KEY_J = SDLK_j,
	KEY_K = SDLK_k,
	KEY_L = SDLK_l,
	KEY_SEMICOLON = SDLK_SEMICOLON,
	KEY_QUOTE = SDLK_QUOTE,
	KEY_TILDE = SDLK_BACKQUOTE,
	KEY_LSHIFT = SDLK_LSHIFT,
	KEY_BACKSLASH = SDLK_BACKSLASH,
	KEY_Z = SDLK_z,
	KEY_X = SDLK_x,
	KEY_C = SDLK_c,
	KEY_V = SDLK_v,
	KEY_B = SDLK_b,
	KEY_N = SDLK_n,
	KEY_M = SDLK_m,
	KEY_COMMA = SDLK_COMMA,
	KEY_PERIOD = SDLK_PERIOD,
	KEY_SLASH = SDLK_SLASH,
	KEY_RSHIFT = SDLK_RSHIFT,
	KEY_MULTIPLY = SDLK_KP_MULTIPLY,
	KEY_ALT = SDLK_LALT,
	KEY_SPACE = SDLK_SPACE,
	KEY_CAPSLOCK = SDLK_CAPSLOCK,
	KEY_F1 = SDLK_F1,
	KEY_F2 = SDLK_F2,
	KEY_F3 = SDLK_F3,
	KEY_F4 = SDLK_F4,
	KEY_F5 = SDLK_F5,
	KEY_F6 = SDLK_F6,
	KEY_F7 = SDLK_F7,
	KEY_F8 = SDLK_F8,
	KEY_F9 = SDLK_F9,
	KEY_F10 = SDLK_F10,
	KEY_NUMLOCK = SDLK_NUMLOCK,
	KEY_SCROLLLOCK = SDLK_SCROLLOCK,
	KEY_HOME = SDLK_HOME,
	KEY_UP = SDLK_UP,
	KEY_PAGEUP = SDLK_PAGEUP,
	KEY_LEFT = SDLK_LEFT,
	KEY_RIGHT = SDLK_RIGHT,
	KEY_PLUS = SDLK_KP_PLUS,
	KEY_END = SDLK_END,
	KEY_DOWN = SDLK_DOWN,
	KEY_PAGEDOWN = SDLK_PAGEDOWN,
	KEY_INSERT = SDLK_INSERT,
	KEY_DELETE = SDLK_DELETE,
	KEY_F11 = SDLK_F11,
	KEY_F12 = SDLK_F12
};

#else

enum
{
	KEY_ESCAPE = 0x01,
	KEY_1 = 0x02,
	KEY_2 = 0x03,
	KEY_3 = 0x04,
	KEY_4 = 0x05,
	KEY_5 = 0x06,
	KEY_6 = 0x07,
	KEY_7 = 0x08,
	KEY_8 = 0x09,
	KEY_9 = 0x0A,
	KEY_0 = 0x0B,
	KEY_MINUS = 0x0C,
	KEY_EQUALS = 0x0D,
	KEY_BACKSPACE = 0x0E,
	KEY_TAB = 0x0F,
	KEY_Q = 0x10,
	KEY_W = 0x11,
	KEY_E = 0x12,
	KEY_R = 0x13,
	KEY_T = 0x14,
	KEY_Y = 0x15,
	KEY_U = 0x16,
	KEY_I = 0x17,
	KEY_O = 0x18,
	KEY_P = 0x19,
	KEY_LEFTBRACKET = 0x1A,
	KEY_RIGHTBRACKET = 0x1B,
	KEY_ENTER = 0x1C,
	KEY_CONTROL = 0x1D,
	KEY_A = 0x1E,
	KEY_S = 0x1F,
	KEY_D = 0x20,
	KEY_F = 0x21,
	KEY_G = 0x22,
	KEY_H = 0x23,
	KEY_J = 0x24,
	KEY_K = 0x25,
	KEY_L = 0x26,
	KEY_SEMICOLON = 0x27,
	KEY_QUOTE = 0x28,
	KEY_TILDE = 0x29,
	KEY_LSHIFT = 0x2A,
	KEY_BACKSLASH = 0x2B,
	KEY_Z = 0x2C,
	KEY_X = 0x2D,
	KEY_C = 0x2E,
	KEY_V = 0x2F,
	KEY_B = 0x30,
	KEY_N = 0x31,
	KEY_M = 0x32,
	KEY_COMMA = 0x33,
	KEY_PERIOD = 0x34,
	KEY_SLASH = 0x35,
	KEY_RSHIFT = 0x36,
	KEY_MULTIPLY = 0x37,
	KEY_ALT = 0x38,
	KEY_SPACE = 0x39,
	KEY_CAPSLOCK = 0x3A,
	KEY_F1 = 0x3B,
	KEY_F2 = 0x3C,
	KEY_F3 = 0x3D,
	KEY_F4 = 0x3E,
	KEY_F5 = 0x3F,
	KEY_F6 = 0x40,
	KEY_F7 = 0x41,
	KEY_F8 = 0x42,
	KEY_F9 = 0x43,
	KEY_F10 = 0x44,
	KEY_NUMLOCK = 0x45,
	KEY_SCROLLLOCK = 0x46,
	KEY_HOME = 0x47,
	KEY_UP = 0x48,
	KEY_PAGEUP = 0x49,
	KEY_LEFT = 0x4B,
	KEY_RIGHT = 0x4D,
	KEY_PLUS = 0x4E,
	KEY_END = 0x4F,
	KEY_DOWN = 0x50,
	KEY_PAGEDOWN = 0x51,
	KEY_INSERT = 0x52,
	KEY_DELETE = 0x53,
	KEY_F11 = 0x57,
	KEY_F12 = 0x58
};

#endif

/*
 * functions
 */

int platform_init(int w, int h, int bpp, const char *title);
void platform_quit();
int platform_frame();
void platform_blit(int w, int h, int stride, void *pixels);
void platform_screen_clear(uint32_t c);
int platform_key(int sc);
void platform_draw_pixel(uint16_t x, uint16_t y, uint32_t c);
int platform_mouse(int *x, int *y, int *dx, int *dy);
void platform_error(const char *s, ...);
void platform_mouse_capture();
void platform_mouse_release();
