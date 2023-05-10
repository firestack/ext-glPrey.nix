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

/* platform */
#define PLATFORM_SDL2 1
#include "platform.h"

/* small3dlib */
#define S3L_RESOLUTION_X 640
#define S3L_RESOLUTION_Y 480
#define S3L_PIXEL_FUNCTION S3L_Pixel
#define S3L_Z_BUFFER 1
#define S3L_USE_WIDER_TYPES 1
#define S3L_UNIT(i) (S3L_Unit)((i) * S3L_F)
#define S3L_NEAR_CROSS_STRATEGY 3
#define S3L_PERSPECTIVE_CORRECTION 1
#include "small3dlib.h"

/* utilities */
#define BPP 32
#include "rgb.h"

/* palette macro */
#if BPP == 8
#define PALETTE(i, p) (i)
#elif BPP == 32
#define PALETTE(i, p) ARGB(PALETTE_RGB24(i, p)->r, PALETTE_RGB24(i, p)->g, PALETTE_RGB24(i, p)->b, 255)
#endif

/* prey95bsp */
#include "wad.h"
#include "bsp.h"

/*
 *
 * globals
 *
 */

/* prey */
bsp_t *bsp;
wad_t *wad;
uint8_t *palette;
int len_palette;

/* s3l components */
struct
{
	S3L_Index *triangles;
	int num_triangles;

	S3L_Unit *vertices;
	int num_vertices;

	S3L_Model3D model;
	S3L_Scene scene;
} s3l;

/*
 *
 * functions
 *
 */

/*
 * S3L_Pixel
 */

static inline void S3L_Pixel(S3L_PixelInfo *pixel)
{
	/* wireframe or barycentric */
	if (pixel->barycentric[0] == 0 || pixel->barycentric[1] == 0 || pixel->barycentric[2] == 0)
		platform_draw_pixel(pixel->x, pixel->y, ARGB(255, 255, 255, 255));
	else
		platform_draw_pixel(pixel->x, pixel->y, ARGB(pixel->barycentric[0] >> 1, pixel->barycentric[1] >> 1, pixel->barycentric[2] >> 1, 255));
}

/*
 * s3l_init
 */

void s3l_init(bsp_t *bsp)
{
	/* variables */
	int i, v, t;

	/* create vertices */
	s3l.num_vertices = bsp->num_vertices;
	s3l.vertices = calloc(s3l.num_vertices, sizeof(S3L_Unit) * 3);

	/* read vertices */
	for (i = 0; i < bsp->num_vertices; i++)
	{
		s3l.vertices[i * 3] = bsp->xcomponents[bsp->vertices[i].v[0]];
		s3l.vertices[(i * 3) + 1] = bsp->ycomponents[bsp->vertices[i].v[1]];
		s3l.vertices[(i * 3) + 2] = bsp->zcomponents[bsp->vertices[i].v[2]];
	}

	/* determine number of tris */
	s3l.num_triangles = 0;
	for (i = 0; i < bsp->num_polygons; i++)
	{
		s3l.num_triangles += (bsp->polygons[i].num_verts - 2);
	}

	/* create triangles */
	s3l.triangles = calloc(s3l.num_triangles, sizeof(S3L_Index) * 3);

	/* triangulate polygons */
	t = 0;
	for (i = 0; i < bsp->num_polygons; i++)
	{
		for (v = 0; v < bsp->polygons[i].num_verts - 2; v++)
		{
			s3l.triangles[t * 3] = bsp->polygons[i].verts[0];
			s3l.triangles[(t * 3) + 1] = bsp->polygons[i].verts[v + 1];
			s3l.triangles[(t * 3) + 2] = bsp->polygons[i].verts[v + 2];
			t++;
		}
	}

	/* init model */
	S3L_model3DInit(s3l.vertices, s3l.num_vertices, s3l.triangles, s3l.num_triangles, &s3l.model);
	s3l.model.config.backfaceCulling = 1;

	/* init scene */
	S3L_sceneInit(&s3l.model, 1, &s3l.scene);

	/* camera */
	s3l.scene.camera.transform.translation.x = bsp->camera.viewpoint.v[0];
	s3l.scene.camera.transform.translation.y = bsp->camera.viewpoint.v[1];
	s3l.scene.camera.transform.translation.z = bsp->camera.viewpoint.v[2];
}

/*
 * s3l_free
 */

void s3l_free()
{
	if (s3l.vertices) free(s3l.vertices);
	if (s3l.triangles) free(s3l.triangles);
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

	/* create s3l scene */
	s3l_init(bsp);

	/* init */
	platform_init(S3L_RESOLUTION_X, S3L_RESOLUTION_Y, BPP, "bsp");

	/* main loop */
	while (platform_frame())
	{
		/* variables */
		S3L_Vec4 v_forward, v_up, v_right;
		int dx, dy;

		/* get movedir */
		S3L_rotationToDirections(s3l.scene.camera.transform.rotation, S3L_F, &v_forward, &v_right, &v_up);

		/* process inputs */
		if (platform_key(KEY_ESCAPE)) break;

		/* process inputs */
		if (platform_key(KEY_W)) S3L_vec3Add(&s3l.scene.camera.transform.translation, v_forward);
		if (platform_key(KEY_A)) S3L_vec3Sub(&s3l.scene.camera.transform.translation, v_right);
		if (platform_key(KEY_S)) S3L_vec3Sub(&s3l.scene.camera.transform.translation, v_forward);
		if (platform_key(KEY_D)) S3L_vec3Add(&s3l.scene.camera.transform.translation, v_right);
		if (platform_key(KEY_UP)) s3l.scene.camera.transform.rotation.x += 4;
		if (platform_key(KEY_DOWN)) s3l.scene.camera.transform.rotation.x -= 4;
		if (platform_key(KEY_LEFT)) s3l.scene.camera.transform.rotation.y += 4;
		if (platform_key(KEY_RIGHT)) s3l.scene.camera.transform.rotation.y -= 4;

		/* process mouse */
		if (platform_mouse(NULL, NULL, &dx, &dy))
		{
			platform_mouse_capture();
			s3l.scene.camera.transform.rotation.y -= dx;
			s3l.scene.camera.transform.rotation.x -= dy;
		}
		else
		{
			platform_mouse_release();
		}

		/* clear screen */
		platform_screen_clear(PALETTE(0, palette));

		/* draw */
		S3L_newFrame();
		S3L_drawScene(s3l.scene);
	}

	/* quit */
	platform_quit();
	bsp_free(bsp);
	wad_free(wad);
	s3l_free();

	/* exit gracefully */
	return 0;
}
