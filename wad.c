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
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* shim */
#include "shim.h"

/* wad */
#include "wad.h"

/*
 *
 * functions
 *
 */

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
		shim_error("failed to open %s", filename);

	/* alloc */
	wad = calloc(1, sizeof(wad_t));
	if (wad == NULL)
		shim_error("failed malloc");

	/* read header */
	fread(&wad->header, sizeof(wad_header_t), 1, file);

	/* check magic */
	if (memcmp(&wad->header.magic, "IWAD", 4) != 0)
		shim_error("invalid wad file");

	/* allocate lumps */
	wad->lumps = calloc(wad->header.num_lumps, sizeof(wad_lump_t));
	if (wad->lumps == NULL)
		shim_error("failed malloc");

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
			shim_error("failed malloc");

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

/*
 * wad_find
 */

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
