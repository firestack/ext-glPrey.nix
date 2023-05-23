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
#include <math.h>

/* shim */
#include "shim.h"

/* wad */
#include "mip.h"

/*
 *
 * functions
 *
 */

/*
 * mip_from_buffer
 */

mip_t *mip_from_buffer(void *buffer, size_t buffer_len)
{
	/* variables */
	int i;
	mip_t *mip;
	uint8_t *ptr;
	uint16_t offsets[64];
	uint16_t sizes[64];

	/* alloc */
	mip = malloc(sizeof(mip_t));
	if (mip == NULL) return NULL;

	/* read header */
	ptr = (uint8_t *)buffer;
	memcpy(&mip->header, ptr, sizeof(mip_header_t));
	ptr += sizeof(mip_header_t);

	/* get offsets */
	for (i = 0; i < mip->header.num_entries; i++)
	{
		memcpy(&offsets[i], ptr, sizeof(uint16_t));
		ptr += sizeof(uint16_t);
	}

	/* get sizes */
	for (i = 0; i < mip->header.num_entries; i++)
	{
		if (i == mip->header.num_entries - 1)
			sizes[i] = buffer_len - offsets[i];
		else
			sizes[i] = offsets[i + 1] - offsets[i];
	}

	/* allocate entries */
	mip->entries = malloc(sizeof(mip_entry_t) * mip->header.num_entries);

	/* get pixels */
	for (i = 0; i < mip->header.num_entries; i++)
	{
		/* width / height */
		mip->entries[i].width = mip->entries[i].height = (uint8_t)sqrtf(sizes[i]);

		/* pixels */
		mip->entries[i].pixels = malloc(sizes[i]);

		/* copy */
		ptr = (uint8_t *)buffer + offsets[i];
		memcpy(mip->entries[i].pixels, ptr, sizes[i]);
	}

	/* return ptr */
	return mip;
}

/*
 * mip_free
 */

void *mip_free(mip_t *mip)
{
	int i;

	if (mip)
	{
		if (mip->entries)
		{
			for (i = 0; i < mip->header.num_entries; i++)
			{
				if (mip->entries[i].pixels)
					free(mip->entries[i].pixels);
			}

			free(mip->entries);
		}

		free(mip);
	}

	return NULL;
}
