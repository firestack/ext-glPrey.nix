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

/* prey95bsp */
#include "bsp.h"

/*
 *
 * functions
 *
 */

/*
 * main
 */

int main(void)
{
	/* variables */
	bsp_t *bsp;
	int i, v;
	FILE *ply;

	/* read bsp */
	bsp = bsp_read("DEMO4.BSP");
	if (!bsp) return 1;

	/* write ply */
	ply = fopen("DEMO4.BSP.ply", "wb");
	if (!ply) return 1;

	/* write header */
	fprintf(ply, "ply\nformat ascii 1.0\n");
	fprintf(ply, "element vertex %d\n", bsp->num_vertices);
	fprintf(ply, "property float x\nproperty float y\nproperty float z\n");
	fprintf(ply, "element face %d\n", bsp->num_polygons);
	fprintf(ply, "property list uchar uint vertex_indices\n");
	fprintf(ply, "end_header\n");

	/* write vertices */
	for (i = 0; i < bsp->num_vertices; i++)
	{
		fprintf(ply, "%0.6f %0.6f %0.6f\n",
			bsp->xcomponents[bsp->vertices[i].v[0]],
			bsp->ycomponents[bsp->vertices[i].v[1]],
			bsp->zcomponents[bsp->vertices[i].v[2]]
		);
	}

	/* write polygons */
	for (i = 0; i < bsp->num_polygons; i++)
	{
		fprintf(ply, "%d", bsp->polygons[i].num_verts);

		for (v = 0; v < bsp->polygons[i].num_verts; v++)
		{
			fprintf(ply, " %d", bsp->polygons[i].verts[v]);
		}

		fprintf(ply, "\n");
	}

	/* free memory */
	fclose(ply);
	bsp_free(bsp);

	/* return success */
	return 0;
}
