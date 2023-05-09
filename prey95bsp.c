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

/*
 *
 * types
 *
 */

typedef struct
{
	float v[3];
} vec3_t;

typedef struct
{
	int v[3];
} vec3i_t;

typedef struct
{
	vec3_t viewpoint;
	vec3_t viewnormal;
	int viewangle;
	int texturelength;
} camera_t;

typedef struct
{
	char str[128];
	int len;
} token_t;

typedef float component_t;

typedef struct
{
	int verts[32];
	int num_verts;
	char tname[32];
	vec3_t tu;
	vec3_t tv;
	vec3_t to;
} polygon_t;

typedef struct
{
	float a, b, c, d;
	int inid;
	int outid;
	int front;
	int back;
} node_t;

typedef struct
{
	camera_t camera;

	component_t *xcomponents;
	int num_xcomponents;

	component_t *ycomponents;
	int num_ycomponents;

	component_t *zcomponents;
	int num_zcomponents;

	vec3i_t *vertices;
	int num_vertices;

	polygon_t *polygons;
	int num_polygons;

	node_t *nodes;
	int num_nodes;
} bsp_t;

typedef struct
{
	char magic[4];
	int32_t num_lumps;
	int32_t ofs_lumps;
} wad_header_t;

typedef struct
{
	int32_t ofs_data;
	int32_t len_data;
	int32_t type;
	char name[8];
	void *data;
} wad_lump_t;

typedef struct
{
	wad_header_t header;
	wad_lump_t *lumps;
} wad_t;

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
 * token_read
 */

int token_read(FILE *stream, token_t *token)
{
	/* variables */
	int i;
	char c;

	/* clear string */
	for (i = 0; i < sizeof(token->str); i++) token->str[i] = '\0';

	/* parse loop */
	i = 0;
	while (!feof(stream) && i < sizeof(token->str))
	{
		/* get char */
		c = fgetc(stream);

		/* return success if we've reached a space */
		/* otherwise add to string */
		if (isspace(c))
		{
			/* if we've just started, skip */
			if (i == 0) continue;

			/* return success */
			token->len = i + 1;
			return 1;
		}
		else
		{
			token->str[i++] = c;
		}
	}

	/* return failure */
	return 0;
}

/*
 * token_string
 */

int token_string(token_t *token, const char *string)
{
	if (strcmp(token->str, string) != 0)
		return 0;

	return 1;
}

/*
 * token_expect
 */

int token_expect(FILE *stream, token_t *token, const char *string)
{
	token_read(stream, token);
	return token_string(token, string);
}

/*
 * print_vec3
 */

void print_vec3(FILE *stream, vec3_t *vec)
{
	fprintf(stream, "%0.0f %0.0f %0.0f", vec->v[0], vec->v[1], vec->v[2]);
}

/*
 * print_node
 */

void print_node(FILE *stream, node_t *node, int n)
{
	fprintf(stream, "node: %d\n", n);
	fprintf(stream, "\tA: %0.0f\n", node->a);
	fprintf(stream, "\tB: %0.0f\n", node->b);
	fprintf(stream, "\tC: %0.0f\n", node->c);
	fprintf(stream, "\tD: %0.0f\n", node->d);
	fprintf(stream, "\tinid: %d\n", node->inid);
	fprintf(stream, "\toutid: %d\n", node->outid);
	fprintf(stream, "\n");
	fflush(stream);
}

/*
 * print_polygon
 */

void print_polygon(FILE *stream, polygon_t *polygon, int n)
{
	int i;

	/* idx */
	fprintf(stream, "polygon: %d\n", n);

	/* verts */
	fprintf(stream, "\tnum_verts: %d\n", polygon->num_verts);
	fprintf(stream, "\tverts: ");
	for (i = 0; i < polygon->num_verts; i++)
		fprintf(stream, "%d ", polygon->verts[i]);
	fprintf(stream, "\n");

	/* tname */
	fprintf(stream, "\ttname: %s\n", polygon->tname);

	/* tu */
	fprintf(stream, "\ttu: ");
	print_vec3(stream, &polygon->tu);
	fprintf(stream, "\n");

	/* tv */
	fprintf(stream, "\ttv: ");
	print_vec3(stream, &polygon->tv);
	fprintf(stream, "\n");

	/* to */
	fprintf(stream, "\tto: ");
	print_vec3(stream, &polygon->to);
	fprintf(stream, "\n");

	/* flush */
	fprintf(stream, "\n");
	fflush(stream);
}

/*
 * read_vec3i
 */

void read_vec3i(FILE *stream, vec3i_t *vec)
{
	/* variables */
	token_t token;
	int i;

	for (i = 0; i < 3; i++)
	{
		token_read(stream, &token);
		vec->v[i] = atoi(token.str);
	}
}

/*
 * read_vec3
 */

void read_vec3(FILE *stream, vec3_t *vec)
{
	/* variables */
	token_t token;
	int i;

	for (i = 0; i < 3; i++)
	{
		token_read(stream, &token);
		vec->v[i] = atof(token.str);
	}
}

/*
 * read_float
 */

void read_float(FILE *stream, float *f)
{
	token_t token;
	token_read(stream, &token);
	*f = atof(token.str);
}

/*
 * read_int
 */

void read_int(FILE *stream, int *i)
{
	token_t token;
	token_read(stream, &token);
	*i = atoi(token.str);
}

/*
 * read_string
 */

void read_string(FILE *stream, char *string, int maxlen)
{
	token_t token;
	token_read(stream, &token);
	strncpy(string, token.str, maxlen);
}

/*
 * read_polygon
 */

void read_polygon(bsp_t *bsp, FILE *stream, polygon_t *polygon)
{
	/* blah */
	int i;
	token_t token;

	/* verts */
	i = 0;
	if (token_expect(stream, &token, "verts"))
	{
		/* tname is next */
		while (!token_expect(stream, &token, "tname"))
		{
			/* otherwise its a vert */
			polygon->verts[i++] = atoi(token.str);
		}

		polygon->num_verts = i;
	}
	else
	{
		platform_error("no verts in polygon");
	}

	/* tname */
	read_string(stream, polygon->tname, sizeof(polygon->tname));

	/* tu */
	if (token_expect(stream, &token, "tu"))
		read_vec3(stream, &polygon->tu);

	/* tv */
	if (token_expect(stream, &token, "tv"))
		read_vec3(stream, &polygon->tv);

	/* to */
	if (token_expect(stream, &token, "to"))
		read_vec3(stream, &polygon->to);
}

/*
 * read_node
 */

void read_node(bsp_t *bsp, FILE *stream, node_t *node)
{
	token_t token;

	while (token_read(stream, &token))
	{
		/* A */
		if (token_string(&token, "A"))
			read_float(stream, &node->a);

		/* B */
		if (token_string(&token, "B"))
			read_float(stream, &node->b);

		/* C */
		if (token_string(&token, "C"))
			read_float(stream, &node->c);

		/* D */
		if (token_string(&token, "D"))
			read_float(stream, &node->d);

		/* inid */
		if (token_string(&token, "inid"))
			read_int(stream, &node->inid);

		/* outid */
		if (token_string(&token, "outid"))
			read_int(stream, &node->outid);

		/* front */
		if (token_string(&token, "front"))
			read_int(stream, &node->front);

		/* back */
		if (token_string(&token, "back"))
			read_int(stream, &node->back);

		/* polygon */
		if (token_string(&token, "polygon"))
		{
			int p;
			read_int(stream, &p);
			read_polygon(bsp, stream, &bsp->polygons[p]);
		}

		/* next node */
		if (token_string(&token, "node"))
		{
			int n;
			read_int(stream, &n);
			read_node(bsp, stream, &bsp->nodes[n]);
		}
	}
}

/*
 * bsp_read
 */

bsp_t *bsp_read(const char *filename)
{
	/* variables */
	FILE *file;
	token_t token;
	bsp_t *bsp;

	/* open file */
	file = fopen(filename, "rb");
	if (file == NULL)
		platform_error("failed to open %s", filename);

	/* alloc */
	bsp = calloc(1, sizeof(bsp_t));
	if (bsp == NULL)
		platform_error("failed malloc");

	/* token loop */
	while (token_read(file, &token))
	{
		/*
		 * read camera
		 */

		if (token_string(&token, "viewpoint"))
			read_vec3(file, &bsp->camera.viewpoint);

		if (token_string(&token, "viewnormal"))
			read_vec3(file, &bsp->camera.viewnormal);

		if (token_string(&token, "viewangle"))
			read_int(file, &bsp->camera.viewangle);

		if (token_string(&token, "texturelength"))
			read_int(file, &bsp->camera.texturelength);

		/* read xcomponents */
		if (token_string(&token, "xcomponents"))
		{
			int x;
			read_int(file, &bsp->num_xcomponents);
			bsp->xcomponents = calloc(bsp->num_xcomponents, sizeof(component_t));
			for (x = 0; x < bsp->num_xcomponents; x++) read_float(file, &bsp->xcomponents[x]);
			printf("%d xcomponents read\n", bsp->num_xcomponents);
		}

		/* read ycomponents */
		if (token_string(&token, "ycomponents"))
		{
			int y;
			read_int(file, &bsp->num_ycomponents);
			bsp->ycomponents = calloc(bsp->num_ycomponents, sizeof(component_t));
			for (y = 0; y < bsp->num_ycomponents; y++) read_float(file, &bsp->ycomponents[y]);
			printf("%d ycomponents read\n", bsp->num_ycomponents);
		}

		/* read zcomponents */
		if (token_string(&token, "zcomponents"))
		{
			int z;
			read_int(file, &bsp->num_zcomponents);
			bsp->zcomponents = calloc(bsp->num_zcomponents, sizeof(component_t));
			for (z = 0; z < bsp->num_zcomponents; z++) read_float(file, &bsp->zcomponents[z]);
			printf("%d zcomponents read\n", bsp->num_zcomponents);
		}

		/* read vertices */
		if (token_string(&token, "numverts"))
		{
			int v;

			read_int(file, &bsp->num_vertices);

			bsp->vertices = calloc(bsp->num_vertices, sizeof(vec3i_t));

			for (v = 0; v < bsp->num_vertices; v++)
			{
				read_vec3i(file, &bsp->vertices[v]);
			}

			printf("%d vertices read\n", bsp->num_vertices);
		}

		/* allocate nodes */
		if (token_string(&token, "numnodes"))
		{
			read_int(file, &bsp->num_nodes);
			bsp->nodes = calloc(bsp->num_nodes, sizeof(node_t));
			printf("%d nodes read\n", bsp->num_nodes);
		}

		/* allocate polygons */
		if (token_string(&token, "numpolys"))
		{
			read_int(file, &bsp->num_polygons);
			bsp->polygons = calloc(bsp->num_polygons, sizeof(polygon_t));
			printf("%d poylgons read\n", bsp->num_polygons);
		}

		/* read polygon */
		if (token_string(&token, "polygon"))
		{
			int p;
			read_int(file, &p);
			read_polygon(bsp, file, &bsp->polygons[p]);
		}

		/* read node */
		if (token_string(&token, "node"))
		{
			int n;
			read_int(file, &n);
			read_node(bsp, file, &bsp->nodes[n]);
		}
	}

	/* close file */
	fclose(file);

	/* return ptr */
	return bsp;
}

/*
 * bsp_free
 */

void bsp_free(bsp_t *bsp)
{
	if (bsp)
	{
		if (bsp->xcomponents) free(bsp->xcomponents);
		if (bsp->ycomponents) free(bsp->ycomponents);
		if (bsp->zcomponents) free(bsp->zcomponents);
		if (bsp->vertices) free(bsp->vertices);
		if (bsp->polygons) free(bsp->polygons);
		if (bsp->nodes) free(bsp->nodes);

		free(bsp);
	}
}

/*
 * wad_read
 */

wad_t *wad_read(const char *filename)
{
	/* variables */
	FILE *file;
	int i;
	wad_t *wad;

	/* open file */
	file = fopen(filename, "rb");
	if (file == NULL)
		platform_error("failed to open %s", filename);

	/* alloc */
	wad = calloc(1, sizeof(wad_t));
	if (wad == NULL)
		platform_error("failed malloc");

	/* read header */
	fread(&wad->header, sizeof(wad_header_t), 1, file);

	/* check magic */
	if (memcmp(&wad->header.magic, "IWAD", 4) != 0)
		platform_error("invalid wad file");

	/* allocate lumps */
	wad->lumps = calloc(wad->header.num_lumps, sizeof(wad_lump_t));
	if (wad->lumps == NULL)
		platform_error("failed malloc");

	/* read lumps */
	fseek(file, wad->header.ofs_lumps, SEEK_SET);
	for (i = 0; i < wad->header.num_lumps; i++)
	{
		fread(&wad->lumps[i].ofs_data, sizeof(int32_t), 1, file);
		fread(&wad->lumps[i].len_data, sizeof(int32_t), 1, file);
		fread(&wad->lumps[i].type, sizeof(int32_t), 1, file);
		fread(&wad->lumps[i].name, sizeof(char), 8, file);
	}

	/* read lump data */
	for (i = 0; i < wad->header.num_lumps; i++)
	{
		/* alloc */
		wad->lumps[i].data = malloc(wad->lumps[i].len_data);
		if (wad->lumps[i].data == NULL)
			platform_error("failed malloc");

		/* read */
		fseek(file, wad->lumps[i].ofs_data, SEEK_SET);
		fread(wad->lumps[i].data, wad->lumps[i].len_data, 1, file);
	}

	/* close file */
	fclose(file);

	/* return ptr */
	return wad;
}

/*
 * wad_free
 */

void wad_free(wad_t *wad)
{
	int i;

	if (wad)
	{
		if (wad->lumps)
		{
			for (i = 0; i < wad->header.num_lumps; i++)
			{
				if (wad->lumps[i].data)
					free(wad->lumps[i].data);
			}

			free(wad->lumps);
		}

		free(wad);
	}
}

void *wad_find(wad_t *wad, const char *search, int *size)
{
	/* variables */
	int i;

	/* search */
	for (i = 0; i < wad->header.num_lumps; i++)
	{
		if (strcmp(search, wad->lumps[i].name) == 0)
		{
			if (size) *size = wad->lumps[i].len_data;
			return wad->lumps[i].data;
		}
	}

	/* failure */
	if (size) *size = 0;
	return NULL;
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
