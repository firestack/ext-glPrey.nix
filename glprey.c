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
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_opengl.h>

/* gl */
#include <GL/gl.h>
#include <GL/glu.h>

/* backend */
#include "backend.h"

/* glprey */
#include "wad.h"
#include "mip.h"
#include "bsp.h"

/*
 *
 * macros
 *
 */

/* level scale */
#define SCALE (1.0f/1024.0f)

/*
 *
 * globals
 *
 */

/* gl */
GLint gl_bsp;
gl_texture_t gl_textures[128];
size_t *lightmap_offsets = NULL;
int num_gl_textures = 0;
bool wireframe = false;

/* prey */
bsp_t *bsp = NULL;
wad_t *wad = NULL;
uint8_t *palette = NULL;
uint8_t *colormap = NULL;
int len_palette = 0;

/*
 *
 * functions
 *
 */

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
 * init_bsp
 */

void init_bsp(bsp_t *bsp)
{
	int i, v;
	mip_t *mip;
	int num_pixels;

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
		vec2_t mins, maxs;
		/*
		vec2_t extents;
		*/

		/* init */
		mins.x = INFINITY;
		mins.y = INFINITY;
		maxs.x = -INFINITY;
		maxs.y = -INFINITY;

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
			tp.x = (p.x - to.x);
			tp.y = (p.y - to.y);
			tp.z = (p.z - to.z);

			/* normalize */
			nu = tu;
			nv = tv;
			normalize(&nu);
			normalize(&nv);

			/* get s,t */
			s = dot(tp, nu) / 8;
			t = dot(tp, nv) / 8;

			/* yeah */
			s *= SCALE;
			t *= SCALE;
			p.x *= SCALE;
			p.y *= SCALE;
			p.z *= SCALE;

			/* store s, t */
			if (s > maxs.x) maxs.x = s;
			if (t > maxs.y) maxs.y = t;
			if (s < mins.x) mins.x = s;
			if (t < mins.y) mins.y = t;

			/*
			printf("x: %0.4f y: %0.4f z: %0.4f\n", p.x, p.y, p.z);
			printf("s: %0.4f t: %0.4f\n\n", s, t);
			*/

			/* do commands */
			glTexCoord2f(s, t);
			glVertex3f(p.x, p.y, p.z);
		}

		/* get extents */
		/*
		extents.x = (maxs.x - mins.x);
		extents.y = (maxs.y - mins.y);

		printf("mins: %0.4f %0.4f\n", mins.x, mins.y);
		printf("maxs: %0.4f %0.4f\n", maxs.x, maxs.y);
		printf("extents: %0.4f %0.4f\n", extents.x, extents.y);
		printf("bytes: %0.4f\n\n", extents.x * extents.y);
		*/

		/* end gl */
		glEnd();
		glDisable(GL_TEXTURE_2D);
	}

	/* close list */
	glEndList();

	/* set camera pos */
	camera_set_pos(
		bsp->camera.viewpoint.x * SCALE,
		bsp->camera.viewpoint.y * SCALE,
		bsp->camera.viewpoint.z * SCALE
	);
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

	/* init sdl and gl  */
	init(640, 480, "glPrey");

	/* init bsp */
	init_bsp(bsp);

	/* main loop */
	while (frame())
	{
		/* inputs */
		if (key(SDL_SCANCODE_ESCAPE))
			break;

		/* do camera */
		camera(1.0f, 90.0f);

		/* other inputs */
		if (time > 0)
		{
			time -= 1.0f;
		}
		if (key(SDL_SCANCODE_TAB) && time < 1)
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
	quit();

	/* exit gracefully */
	return 0;
}
