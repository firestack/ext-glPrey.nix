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
#include <float.h>

/* platform */
#include "platform.h"

/* tinygl */
#include "TinyGL/inc/gl.h"
#include "TinyGL/inc/zbuffer.h"

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

/*
 *
 * globals
 *
 */

/* tinygl */
ZBuffer *zb;
GLint gl_bsp;
GLfloat gl_pos[3];
GLfloat gl_rot[3];

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
 * gluLookAt
 */

void gluLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez, GLfloat centerx, GLfloat centery, GLfloat centerz, GLfloat upx, GLfloat upy, GLfloat upz)
{
	GLfloat m[16];
	GLfloat x[3], y[3], z[3];
	GLfloat mag;

	/* Make rotation matrix */

	/* Z vector */
	z[0] = eyex - centerx;
	z[1] = eyey - centery;
	z[2] = eyez - centerz;
	mag = sqrt( z[0]*z[0] + z[1]*z[1] + z[2]*z[2] );
	if (mag) {  /* mpichler, 19950515 */
		z[0] /= mag;
		z[1] /= mag;
		z[2] /= mag;
	}

	/* Y vector */
	y[0] = upx;
	y[1] = upy;
	y[2] = upz;

	/* X vector = Y cross Z */
	x[0] =  y[1]*z[2] - y[2]*z[1];
	x[1] = -y[0]*z[2] + y[2]*z[0];
	x[2] =  y[0]*z[1] - y[1]*z[0];

	/* Recompute Y = Z cross X */
	y[0] =  z[1]*x[2] - z[2]*x[1];
	y[1] = -z[0]*x[2] + z[2]*x[0];
	y[2] =  z[0]*x[1] - z[1]*x[0];

	/* mpichler, 19950515 */
	/* cross product gives area of parallelogram, which is < 1.0 for
		* non-perpendicular unit-length vectors; so normalize x, y here
		*/

	mag = sqrt( x[0]*x[0] + x[1]*x[1] + x[2]*x[2] );
	if (mag) {
		x[0] /= mag;
		x[1] /= mag;
		x[2] /= mag;
	}

	mag = sqrt( y[0]*y[0] + y[1]*y[1] + y[2]*y[2] );
	if (mag) {
		y[0] /= mag;
		y[1] /= mag;
		y[2] /= mag;
	}

	#define M(row,col)  m[col*4+row]
	M(0,0) = x[0];  M(0,1) = x[1];  M(0,2) = x[2];  M(0,3) = 0.0;
	M(1,0) = y[0];  M(1,1) = y[1];  M(1,2) = y[2];  M(1,3) = 0.0;
	M(2,0) = z[0];  M(2,1) = z[1];  M(2,2) = z[2];  M(2,3) = 0.0;
	M(3,0) = 0.0;   M(3,1) = 0.0;   M(3,2) = 0.0;   M(3,3) = 1.0;
	#undef M

	glMultMatrixf(m);

	/* Translate Eye to Origin */
	glTranslatef(-eyex, -eyey, -eyez);
}

/*
 * init
 */

void init(bsp_t *bsp)
{
	int i, v;
	GLfloat color[4] = {1.0, 1.0, 1.0, 1.0};

	/* enable features */
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);

	/* open list */
	gl_bsp = glGenLists(1);
	glNewList(gl_bsp, GL_COMPILE);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);

	glShadeModel(GL_FLAT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	/* do commands */
	for (i = 0; i < bsp->num_polygons; i++)
	{
		glBegin(GL_TRIANGLE_FAN);

		for (v = 0; v < bsp->polygons[i].num_verts; v++)
		{
			float x, y, z;

			x = bsp->xcomponents[bsp->vertices[bsp->polygons[i].verts[v]].v[0]];
			y = bsp->ycomponents[bsp->vertices[bsp->polygons[i].verts[v]].v[1]];
			z = bsp->zcomponents[bsp->vertices[bsp->polygons[i].verts[v]].v[2]];

			glVertex3f(x, y, z);
		}

		glEnd();
	}

	/* close list */
	glEndList();

	gl_pos[0] = bsp->camera.viewpoint.v[0];
	gl_pos[1] = bsp->camera.viewpoint.v[1];
	gl_pos[2] = bsp->camera.viewpoint.v[2];
}

/*
 * main
 */

int main(int argc, char **argv)
{
	/* read prey bsp */
	bsp = bsp_read("DEMO4.BSP");

	/* read prey wad */
	wad = wad_read("MACT.WAD");

	/* palette */
	palette = wad_find(wad, "PAL", &len_palette);
	if (palette == NULL)
		platform_error("wad does not contain PAL lump");

	/* tinygl */
	zb = ZB_open(WIDTH, HEIGHT, ZB_MODE_5R6G5B, 0, NULL, NULL, NULL);
	if (zb == NULL) platform_error("couldn't allocate zbuffer");
	glInit(zb);

	/* init model */
	init(bsp);

	/* init */
	platform_init(WIDTH, HEIGHT, BPP, "prey95bsp");

	/* main loop */
	while (platform_frame())
	{
		GLfloat h = (GLfloat)HEIGHT / (GLfloat)WIDTH;

		/* inputs */
		if (platform_key(KEY_ESCAPE)) break;
		if (platform_key(KEY_W)) gl_pos[0] += 32;
		if (platform_key(KEY_A)) gl_pos[1] -= 32;
		if (platform_key(KEY_S)) gl_pos[0] -= 32;
		if (platform_key(KEY_D)) gl_pos[1] += 32;
		if (platform_key(KEY_UP)) gl_rot[0] += 8;
		if (platform_key(KEY_DOWN)) gl_rot[0] -= 8;
		if (platform_key(KEY_LEFT)) gl_rot[1] += 8;
		if (platform_key(KEY_RIGHT)) gl_rot[1] -= 8;

		/* gl */
		glViewport(0, 0, (GLint)WIDTH, (GLint)HEIGHT);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glFrustum(-1.0, 1.0, -h, h, 8, FLT_MAX);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(gl_pos[0], gl_pos[1], gl_pos[2], 0, 0, 0, 0, 1, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glPushMatrix();
		glCallList(gl_bsp);
		glPopMatrix();

		/* blit */
		platform_blit(WIDTH, HEIGHT, zb->linesize, zb->pbuf);
	}

	/* quit */
	platform_quit();
	bsp_free(bsp);
	wad_free(wad);
	glClose();

	/* exit gracefully */
	return 0;
}
