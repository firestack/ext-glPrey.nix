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

/* shim */
#include "shim.h"

/* tinygl */
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/ostinygl.h>

/* utilities */
#include "rgb.h"

/* prey95bsp */
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
#define BPP 16
#define FOV 90

/* pi */
#ifndef M_PI
#define M_PI 3.14159265
#endif
#ifndef M_PI_2
#define M_PI_2 M_PI / 2
#endif

#define SPEED (0.5f)

#define SCALE (1.0f/128.0f)
#define SCALE2 (1.0f/1024.0f)

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

/* tinygl */
ostgl_context_t *gl_context;
GLint gl_bsp;
gl_texture_t gl_textures[128];
int num_gl_textures = 0;
vec3_t m_pos;
vec3_t m_rot;
vec3_t m_look;
vec3_t m_strafe;
vec3_t m_vel;
vec3_t m_speedkey;
bool wireframe = false;

/* prey */
bsp_t *bsp;
wad_t *wad;
uint8_t *palette;
int len_palette;

/*
 *
 * functions
 *
 */

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
	glEnable(GL_LIGHTING);
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
		node_t *node;
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

		/* get node */
		node = &bsp->nodes[bsp->polygons[i].node];
		printf("node %d polygon %d\n", bsp->polygons[i].node, i);
		printf("abc: %0.4f %0.4f %0.4f\n", node->a, node->b, node->c);
		printf("d: %0.4f\n", node->d);
		printf("tu: %0.4f %0.4f %0.4f\n", tu.x, tu.y, tu.z);
		printf("tv: %0.4f %0.4f %0.4f\n", tv.x, tv.y, tv.z);
		printf("to: %0.4f %0.4f %0.4f\n\n", to.x * SCALE2, to.y * SCALE2, to.z * SCALE2);

		for (v = 0; v < bsp->polygons[i].num_verts; v++)
		{
			float s, t;
			vec3_t p, tp;

			p.x = bsp->xcomponents[bsp->vertices[bsp->polygons[i].verts[v]].x];
			p.y = bsp->ycomponents[bsp->vertices[bsp->polygons[i].verts[v]].y];
			p.z = bsp->zcomponents[bsp->vertices[bsp->polygons[i].verts[v]].z];

			/* calc offset */
			tp.x = (p.x - to.x);
			tp.y = (p.y - to.y);
			tp.z = (p.z - to.z);

			/* get s,t */
			s = dot(tp, tu, NULL);
			t = dot(tp, tv, NULL);

			printf("s: %0.4f t: %0.4f\n", s * SCALE2, t * SCALE2);
			printf("x: %0.4f y: %0.4f z: %0.4f\n", p.x * SCALE2, p.y * SCALE2, p.z * SCALE2);
			printf("tx: %0.4f ty: %0.4f tz: %0.4f\n\n", tp.x * SCALE2, tp.y * SCALE2, tp.z * SCALE2);

			glTexCoord2f(s * SCALE2, t * SCALE2);
			glVertex3f(p.x * SCALE2, p.y * SCALE2, p.z * SCALE2);
		}

		glEnd();
		glDisable(GL_TEXTURE_2D);
	}

	/* close list */
	glEndList();

	m_pos.x = bsp->camera.viewpoint.x * SCALE2;
	m_pos.y = bsp->camera.viewpoint.y * SCALE2;
	m_pos.z = bsp->camera.viewpoint.z * SCALE2;
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
	GLfloat w = (GLfloat)gl_context->width / (GLfloat)gl_context->height;
	GLfloat h = (GLfloat)gl_context->height / (GLfloat)gl_context->width;

	/* speed */
	if (shim_key_read(SHIM_KEY_LSHIFT))
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
	if (shim_key_read(SHIM_KEY_W))
	{
		m_pos.x += m_look.x * SPEED * m_speedkey.x;
		m_pos.y += m_look.y * SPEED * m_speedkey.y;
		m_pos.z += m_look.z * SPEED * m_speedkey.z;
	}

	/* backwards */
	if (shim_key_read(SHIM_KEY_S))
	{
		m_pos.x -= m_look.x * SPEED * m_speedkey.x;
		m_pos.y -= m_look.y * SPEED * m_speedkey.y;
		m_pos.z -= m_look.z * SPEED * m_speedkey.z;
	}

	/* left */
	if (shim_key_read(SHIM_KEY_A))
	{
		m_pos.x += m_strafe.x * SPEED * m_speedkey.x;
		m_pos.z += m_strafe.z * SPEED * m_speedkey.z;
	}

	/* right */
	if (shim_key_read(SHIM_KEY_D))
	{
		m_pos.x -= m_strafe.x * SPEED * m_speedkey.x;
		m_pos.z -= m_strafe.z * SPEED * m_speedkey.z;
	}

	/* arrow keys */
	if (shim_key_read(SHIM_KEY_UP)) m_rot.x += 0.1f;
	if (shim_key_read(SHIM_KEY_DOWN)) m_rot.x -= 0.1f;
	if (shim_key_read(SHIM_KEY_LEFT)) m_rot.y -= 0.1f;
	if (shim_key_read(SHIM_KEY_RIGHT)) m_rot.y += 0.1f;

	/* set viewport */
	glViewport(0, 0, (GLint)gl_context->width, (GLint)gl_context->height);

	/* set perspective */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(FOV * h, w, 1, FLT_MAX);

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
 * main
 */

int main(int argc, char **argv)
{
	int i;

	/* suppress warnings */
	SHIM_UNUSED(argc);
	SHIM_UNUSED(argv);

	/* read prey bsp */
	bsp = bsp_read("DEMO4.BSP");

	/* read prey wad */
	wad = wad_read("MACT.WAD");

	/* palette */
	palette = wad_find(wad, "PAL", &len_palette);
	if (palette == NULL)
		shim_error("wad does not contain PAL lump");

	/* tinygl */
	gl_context = ostgl_create_context(WIDTH, HEIGHT, BPP);
	ostgl_make_current(gl_context);

	/* init model */
	init(bsp);

	/* init */
	shim_init(gl_context->width, gl_context->height, gl_context->depth, "prey95bsp");

	/* main loop */
	while (shim_frame())
	{
		/* inputs */
		if (shim_key_read(SHIM_KEY_ESCAPE))
			shim_should_quit(SHIM_TRUE);

		/* do camera */
		camera();

		/* clear screen */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* render map, optionally with wireframe */
		glPushMatrix();
		glCallList(gl_bsp);
		if (wireframe)
		{
			GLfloat white[4] = {1.0, 1.0, 1.0, 1.0};
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glMaterialfv(GL_FRONT, GL_EMISSION, white);
			glPolygonOffset(0, -1);
			glCallList(gl_bsp);
		}
		glPopMatrix();

		/* blit */
		shim_blit(gl_context->width, gl_context->height, gl_context->depth, gl_context->pixels);
	}

	/* free textures */
	for (i = 0; i < num_gl_textures; i++)
	{
		if (gl_textures[i].pixels)
			free(gl_textures[i].pixels);
	}

	/* quit */
	glDeleteLists(gl_bsp, 1);
	ostgl_delete_context(gl_context);
	shim_quit();
	bsp_free(bsp);
	wad_free(wad);

	/* exit gracefully */
	return 0;
}
