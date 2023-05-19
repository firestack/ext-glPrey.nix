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

#define SPEED 400

/*
 *
 * globals
 *
 */

/* tinygl */
ostgl_context *gl_context;
GLint gl_bsp;
vec3_t m_pos;
vec3_t m_rot;
vec3_t m_look;
vec3_t m_strafe;
vec3_t m_vel;
vec3_t m_speedkey;

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
 * init
 */

void init(bsp_t *bsp)
{
	int i, v;

	/* enable features */
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);

	/* open list */
	gl_bsp = glGenLists(1);
	glNewList(gl_bsp, GL_COMPILE);

	glShadeModel(GL_FLAT);

	/* do commands */
	for (i = 0; i < bsp->num_polygons; i++)
	{
		glBegin(GL_TRIANGLE_FAN);

		for (v = 0; v < bsp->polygons[i].num_verts; v++)
		{
			glVertex3f(
				bsp->xcomponents[bsp->vertices[bsp->polygons[i].verts[v]].v[0]],
				bsp->ycomponents[bsp->vertices[bsp->polygons[i].verts[v]].v[1]],
				bsp->zcomponents[bsp->vertices[bsp->polygons[i].verts[v]].v[2]]
			);
		}

		glEnd();
	}

	/* close list */
	glEndList();

	m_pos.v[0] = bsp->camera.viewpoint.v[0];
	m_pos.v[1] = bsp->camera.viewpoint.v[1];
	m_pos.v[2] = bsp->camera.viewpoint.v[2];
}

/*
 * main
 */

int main(int argc, char **argv)
{
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
		GLfloat w = (GLfloat)gl_context->width / (GLfloat)gl_context->height;
		GLfloat h = (GLfloat)gl_context->height / (GLfloat)gl_context->width;
		GLfloat white[4] = {1.0, 1.0, 1.0, 1.0};
		GLfloat black[4] = {0.0, 0.0, 0.0, 1.0};

		/* inputs */
		if (shim_key_read(SHIM_KEY_ESCAPE))
			shim_should_quit(SHIM_TRUE);

		/* speed */
		if (shim_key_read(SHIM_KEY_LSHIFT))
		{
			m_speedkey.v[0] = 2;
			m_speedkey.v[1] = 2;
			m_speedkey.v[2] = 2;
		}
		else
		{
			m_speedkey.v[0] = 1;
			m_speedkey.v[1] = 1;
			m_speedkey.v[2] = 1;
		}

		/* forwards */
		if (shim_key_read(SHIM_KEY_W))
		{
			m_pos.v[0] += m_look.v[0] * SPEED * m_speedkey.v[0];
			m_pos.v[1] += m_look.v[1] * SPEED * m_speedkey.v[1];
			m_pos.v[2] += m_look.v[2] * SPEED * m_speedkey.v[2];
		}

		/* backwards */
		if (shim_key_read(SHIM_KEY_S))
		{
			m_pos.v[0] -= m_look.v[0] * SPEED * m_speedkey.v[0];
			m_pos.v[1] -= m_look.v[1] * SPEED * m_speedkey.v[1];
			m_pos.v[2] -= m_look.v[2] * SPEED * m_speedkey.v[2];
		}

		/* left */
		if (shim_key_read(SHIM_KEY_A))
		{
			m_pos.v[0] += m_strafe.v[0] * SPEED * m_speedkey.v[0];
			m_pos.v[2] += m_strafe.v[2] * SPEED * m_speedkey.v[2];
		}

		/* right */
		if (shim_key_read(SHIM_KEY_D))
		{
			m_pos.v[0] -= m_strafe.v[0] * SPEED * m_speedkey.v[0];
			m_pos.v[2] -= m_strafe.v[2] * SPEED * m_speedkey.v[2];
		}

		/* arrow keys */
		if (shim_key_read(SHIM_KEY_UP)) m_rot.v[0] += 0.1f;
		if (shim_key_read(SHIM_KEY_DOWN)) m_rot.v[0] -= 0.1f;
		if (shim_key_read(SHIM_KEY_LEFT)) m_rot.v[1] -= 0.1f;
		if (shim_key_read(SHIM_KEY_RIGHT)) m_rot.v[1] += 0.1f;

		/* set viewport */
		glViewport(0, 0, (GLint)gl_context->width, (GLint)gl_context->height);

		/* set perspective */
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(FOV * h, w, 8, FLT_MAX);

		/* set camera view */
		m_look.v[0] = cosf(m_rot.v[1]) * cosf(m_rot.v[0]);
		m_look.v[1] = sinf(m_rot.v[0]);
		m_look.v[2] = sinf(m_rot.v[1]) * cosf(m_rot.v[0]);
		m_strafe.v[0] = cosf(m_rot.v[1] - M_PI_2);
		m_strafe.v[2] = sinf(m_rot.v[1] - M_PI_2);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(m_pos.v[0], m_pos.v[1], m_pos.v[2], m_pos.v[0] + m_look.v[0], m_pos.v[1] + m_look.v[1], m_pos.v[2] + m_look.v[2], 0.0f, 1.0f, 0.0);

		/* clear screen */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* render map with wireframe */
		glPushMatrix();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glMaterialfv(GL_FRONT, GL_EMISSION, black);
		glCallList(gl_bsp);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glMaterialfv(GL_FRONT, GL_EMISSION, white);
		glPolygonOffset(0, -1);
		glCallList(gl_bsp);
		glPopMatrix();

		/* blit */
		shim_blit(gl_context->width, gl_context->height, gl_context->depth, gl_context->pixels);
	}

	/* quit */
	ostgl_delete_context(gl_context);
	shim_quit();
	bsp_free(bsp);
	wad_free(wad);

	/* exit gracefully */
	return 0;
}
