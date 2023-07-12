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

/* vec3 (float) */
typedef struct
{
	float x, y, z;
} vec3_t;

/* vec3 (integer) */
typedef struct
{
	int x, y, z;
} vec3i_t;

/* bsp camera */
typedef struct
{
	vec3_t viewpoint;
	vec3_t viewnormal;
	int viewangle;
	int texturelength;
} camera_t;

#define TOKEN_STR_LEN 128

/* lexer token */
typedef struct
{
	char str[TOKEN_STR_LEN];
	int len;
} token_t;

/* bsp x/y/z component */
typedef float component_t;

/* bsp polygon */
typedef struct
{
	int verts[32];
	int num_verts;
	char tname[32];
	vec3_t tu;
	vec3_t tv;
	vec3_t to;
	int node;
} polygon_t;

/* bsp node */
typedef struct
{
	float a, b, c, d;
	int inid;
	int outid;
	int front;
	int back;
} node_t;

/* bsp structure */
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

/* function prototypes */
bsp_t *bsp_read(const char *filename);
void bsp_free(bsp_t *bsp);
void bsp_save(bsp_t *bsp, const char *filename);
