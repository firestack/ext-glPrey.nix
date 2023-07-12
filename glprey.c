/*
MIT License

Copyright (c) 2023 erysdren (it/she/they)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*
 *
 * headers
 *
 */

/* std */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

/* sdl2 */
#include <SDL.h>
#include <SDL_opengl.h>

/* gl */
#include <GL/gl.h>
#include <GL/glu.h>

/* glprey */
#include "wad.h"
#include "mip.h"
#include "bsp.h"

/*
 *
 * macros
 *
 */

/* gfx */
#define WIDTH 640
#define HEIGHT 480
#define FOV 90

/* pi */
#ifndef M_PI
#define M_PI 3.14159265
#endif
#ifndef M_PI_2
#define M_PI_2 M_PI / 2
#endif

#define SPEED (400.0f)

#define SCALE (1.0f/128.0f)
#define SCALE2 (1.0f/1024.0f)

#define KEY(x) (keys[x])

#define RAD2DEG(x) ((x) * 180.0f/M_PI)
#define DEG2RAD(x) ((x) * M_PI/180.0f)

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN3(a, b, c) MIN(a, MIN(b, c))
#define MAX3(a, b, c) MAX(a, MAX(b, c))
#define CLAMP(a, min, max) MIN(MAX(a, min), max)

/*
 *
 * types
 *
 */

typedef struct
{
	GLuint id;
	uint8_t *pixels;
	int width;
	int height;
	char name[8];
} gl_texture_t;

/*
 *
 * globals
 *
 */

/* sdl2 */
SDL_Window *window;
SDL_GLContext context;
const Uint8 *keys;

/* gl */
GLint gl_bsp;
gl_texture_t gl_textures[128];
size_t *lightmap_offsets = NULL;
int num_gl_textures = 0;
vec3_t m_pos;
vec3_t m_rot;
vec3_t m_look;
vec3_t m_strafe;
vec3_t m_vel;
vec3_t m_speedkey;
bool wireframe = false;

/* prey */
bsp_t *bsp = NULL;
wad_t *wad = NULL;
uint8_t *palette = NULL;
int len_palette = 0;

/*
 *
 * functions
 *
 */

/*
 * error
 */

void error(const char *s, ...)
{
	/* variables */
	va_list ap;
	static char scratch[256];

	/* do vargs */
	va_start(ap, s);
	vsnprintf(scratch, 256, s, ap);
	va_end(ap);

	/* show message box error */
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", scratch, NULL);

	/* print to stderr */
	fprintf(stderr, "error: %s\n", scratch);

	/* exit */
	exit(1);
}

/*
 * dot
 */

GLfloat dot(vec3_t v1, vec3_t v2, vec3_t *o)
{
	GLfloat result = 0.0f;

	if (o)
	{
		o->x = v1.x * v2.x;
		o->y = v1.y * v2.y;
		o->z = v1.z * v2.z;
		result += o->x;
		result += o->y;
		result += o->z;
	}
	else
	{
		result += v1.x * v2.x;
		result += v1.y * v2.y;
		result += v1.z * v2.z;
	}

	return result;
}

/*
 * normalize
 */

GLfloat normalize(vec3_t *v)
{
	GLfloat w = sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
	v->x /= w;
	v->y /= w;
	v->z /= w;
	return w;
}

/*
 * find_texture
 */

int find_texture(char *s)
{
	int i;

	for (i = 0; i < num_gl_textures; i++)
	{
		if (strcmp(gl_textures[i].name, s) == 0)
			return gl_textures[i].id;
	}

	error("couldn't find texture %s\n", s);

	return -1;
}

/*
 * init
 */

void init(bsp_t *bsp)
{
	int i, v;
	mip_t *mip;
	int num_pixels;

	/* enable features */
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	/* generate textures */
	for (i = 0; i < wad->header.num_lumps; i++)
	{
		/* only miptex */
		if (wad->lumps[i].type != 11)
			continue;

		/* get mip */
		mip = mip_from_buffer(wad->lumps[i].data, wad->lumps[i].len_data);
		if (!mip)
		{
			printf("couldn't read mip %s\n", wad->lumps[i].name);
			continue;
		}

		/* copy values for searching */
		memcpy(gl_textures[num_gl_textures].name, wad->lumps[i].name, 8);
		gl_textures[num_gl_textures].width = mip->header.width;
		gl_textures[num_gl_textures].height = mip->header.height;
		num_pixels = gl_textures[num_gl_textures].width * gl_textures[num_gl_textures].height;

		/* create 24 bit version */
		gl_textures[num_gl_textures].pixels = malloc(num_pixels * 3);
		for (v = 0; v < num_pixels; v++)
		{
			uint8_t *entry = &((uint8_t *)palette)[mip->entries[0].pixels[v] * 3];
			gl_textures[num_gl_textures].pixels[v * 3] = *(entry);
			gl_textures[num_gl_textures].pixels[(v * 3) + 1] = *(entry + 1);
			gl_textures[num_gl_textures].pixels[(v * 3) + 2] = *(entry + 2);
		}

		glGenTextures(1, &gl_textures[num_gl_textures].id);
		glBindTexture(GL_TEXTURE_2D, gl_textures[num_gl_textures].id);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, gl_textures[num_gl_textures].width, gl_textures[num_gl_textures].height, 0, GL_RGB, GL_UNSIGNED_BYTE, gl_textures[num_gl_textures].pixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		/* number of gl textures */
		num_gl_textures++;

		/* free miptex */
		mip_free(mip);
	}

	/* open list */
	gl_bsp = glGenLists(1);
	glNewList(gl_bsp, GL_COMPILE);

	glShadeModel(GL_FLAT);

	/* do commands */
	for (i = 0; i < bsp->num_polygons; i++)
	{
		vec3_t tu, tv, to;

		/* find texture */
		v = find_texture(bsp->polygons[i].tname);

		glBindTexture(GL_TEXTURE_2D, v);
		glEnable(GL_TEXTURE_2D);
		glBegin(GL_TRIANGLE_FAN);

		/* copy values */
		tu = bsp->polygons[i].tu;
		tv = bsp->polygons[i].tv;
		to = bsp->polygons[i].to;

		for (v = 0; v < bsp->polygons[i].num_verts; v++)
		{
			float s, t;
			vec3_t p, tp, nu, nv;

			p.x = bsp->xcomponents[bsp->vertices[bsp->polygons[i].verts[v]].x];
			p.y = bsp->ycomponents[bsp->vertices[bsp->polygons[i].verts[v]].y];
			p.z = bsp->zcomponents[bsp->vertices[bsp->polygons[i].verts[v]].z];

			/* calc offset */
			tp.x = (p.x - to.x) * SCALE2;
			tp.y = (p.y - to.y) * SCALE2;
			tp.z = (p.z - to.z) * SCALE2;

			/* normalize */
			nu = tu;
			nv = tv;
			normalize(&nu);
			normalize(&nv);

			/* get s,t */
			s = dot(tp, nu, NULL);
			t = dot(tp, nv, NULL);

			/* do commands */
			glTexCoord2f(s / 8, t / 8);
			glVertex3f(p.x, p.y, p.z);
		}

		glEnd();
		glDisable(GL_TEXTURE_2D);
	}

	/* close list */
	glEndList();

	m_pos.x = bsp->camera.viewpoint.x;
	m_pos.y = bsp->camera.viewpoint.y;
	m_pos.z = bsp->camera.viewpoint.z;
}

/*
 * check_extension
 */

int check_extension(const char *string, const char *ext)
{
	int s, e;

	e = strlen(ext) + 1;
	s = strlen(string) + 1;

	while (--e >= 0 && --s >= 0)
	{
		if (ext[e] != string[s])
			return 0;
	}

	return 1;
}

/*
 * camera
 */

void camera()
{
	int w, h;
	GLfloat x, y;

	SDL_GL_GetDrawableSize(window, &w, &h);

	x = (GLfloat)w / (GLfloat)h;
	y = (GLfloat)h / (GLfloat)w;

	/* speed */
	if (KEY(SDL_SCANCODE_LSHIFT))
	{
		m_speedkey.x = 2;
		m_speedkey.y = 2;
		m_speedkey.z = 2;
	}
	else
	{
		m_speedkey.x = 1;
		m_speedkey.y = 1;
		m_speedkey.z = 1;
	}

	/* forwards */
	if (KEY(SDL_SCANCODE_W))
	{
		m_pos.x += m_look.x * SPEED * m_speedkey.x;
		m_pos.y += m_look.y * SPEED * m_speedkey.y;
		m_pos.z += m_look.z * SPEED * m_speedkey.z;
	}

	/* backwards */
	if (KEY(SDL_SCANCODE_S))
	{
		m_pos.x -= m_look.x * SPEED * m_speedkey.x;
		m_pos.y -= m_look.y * SPEED * m_speedkey.y;
		m_pos.z -= m_look.z * SPEED * m_speedkey.z;
	}

	/* left */
	if (KEY(SDL_SCANCODE_A))
	{
		m_pos.x += m_strafe.x * SPEED * m_speedkey.x;
		m_pos.z += m_strafe.z * SPEED * m_speedkey.z;
	}

	/* right */
	if (KEY(SDL_SCANCODE_D))
	{
		m_pos.x -= m_strafe.x * SPEED * m_speedkey.x;
		m_pos.z -= m_strafe.z * SPEED * m_speedkey.z;
	}

	/* arrow keys */
	if (KEY(SDL_SCANCODE_UP)) m_rot.x += 0.1f;
	if (KEY(SDL_SCANCODE_DOWN)) m_rot.x -= 0.1f;
	if (KEY(SDL_SCANCODE_LEFT)) m_rot.y -= 0.1f;
	if (KEY(SDL_SCANCODE_RIGHT)) m_rot.y += 0.1f;

	/* lock camera */
	if (m_rot.x < DEG2RAD(-75.0f)) m_rot.x = DEG2RAD(-75.0f);
	if (m_rot.x > DEG2RAD(75.0f)) m_rot.x = DEG2RAD(75.0f);

	/* set viewport */
	glViewport(0, 0, w, h);

	/* set perspective */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(FOV * y, x, 1, FLT_MAX);

	/* set camera view */
	m_look.x = cosf(m_rot.y) * cosf(m_rot.x);
	m_look.y = sinf(m_rot.x);
	m_look.z = sinf(m_rot.y) * cosf(m_rot.x);
	m_strafe.x = cosf(m_rot.y - M_PI_2);
	m_strafe.z = sinf(m_rot.y - M_PI_2);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(m_pos.x, m_pos.y, m_pos.z, m_pos.x + m_look.x,
		m_pos.y + m_look.y, m_pos.z + m_look.z, 0.0f, 1.0f, 0.0);
}

/*
 * frame
 */

bool frame(void)
{
	SDL_Event event;
	bool ret = true;

	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT) ret = false;
	}

	keys = SDL_GetKeyboardState(NULL);

	SDL_GL_SwapWindow(window);

	glClearColor(0, 0, 0, 1);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	return ret;
}

/*
 * main
 */

int main(int argc, char *argv[])
{
	int i;
	float time = 0.0f;

	/* check if user specified files */
	for (i = 1; i < argc; i++)
	{
		/* bsp */
		if (strcmp(argv[i], "--bsp") == 0 && i + 1 < argc)
		{
			bsp = bsp_read(argv[i + 1]);
			if (!bsp) error("couldn't read bsp %s", argv[i + 1]);
		}

		/* wad */
		if (strcmp(argv[i], "--wad") == 0 && i + 1 < argc)
		{
			wad = wad_read(argv[i + 1]);
			if (!wad) error("couldn't read wad %s", argv[i + 1]);
		}
	}

	/* read them again if user didn't select */
	if (bsp == NULL) bsp = bsp_read("DEMO4.BSP");
	if (bsp == NULL) error("couldn't read wad DEMO4.BSP");
	if (wad == NULL) wad = wad_read("MACT.WAD");
	if (wad == NULL) error("couldn't read wad MACT.WAD");

	/* palette */
	palette = wad_find(wad, "PAL", &len_palette);
	if (palette == NULL)
		error("wad does not contain PAL lump");

	/* sdl */
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	window = SDL_CreateWindow("glprey", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	context = SDL_GL_CreateContext(window);

	/* init model */
	init(bsp);

	/* main loop */
	while (frame())
	{
		/* inputs */
		if (KEY(SDL_SCANCODE_ESCAPE))
			break;

		/* do camera */
		camera();

		/* other inputs */
		if (time > 0)
		{
			time -= 1.0f;
		}
		if (KEY(SDL_SCANCODE_TAB) && time < 1)
		{
			wireframe = wireframe ? false : true;
			time += 10.0f;
		}

		/* render map, optionally with wireframe */
		glPushMatrix();
		if (wireframe == true)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glCallList(gl_bsp);
		glPopMatrix();
	}

	/* free textures */
	for (i = 0; i < num_gl_textures; i++)
	{
		if (gl_textures[i].pixels)
			free(gl_textures[i].pixels);
	}

	/* quit */
	glDeleteLists(gl_bsp, 1);
	bsp_free(bsp);
	wad_free(wad);
	SDL_DestroyWindow(window);
	SDL_GL_DeleteContext(context);
	SDL_Quit();

	/* exit gracefully */
	return 0;
}
