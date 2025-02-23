/*
 * Copyright (c) 1993-2025 Paul Mattes.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the names of Paul Mattes, Jeff Sparkes, GTRC nor the names of
 *       their contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY PAUL MATTES "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL PAUL MATTES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * x029 card images.
 */

#include <X11/Intrinsic.h>
#include <X11/xpm.h>

#include "jones.h"
#include "cardimg.h"

#include "card_collins.xpm"
#include "card_cmu.xpm"
#include "card_ibm.xpm"
#include "card_harvard.xpm"
#include "card_pink.xpm"

#include "cardmenu_collins.xpm"
#include "cardmenu_cmu.xpm"
#include "cardmenu_ibm.xpm"
#include "cardmenu_harvard.xpm"
#include "cardmenu_pink.xpm"

static const struct card_image {
    const char *name;
    const char *description;
    char **pixmap_source;
    char **pixmap_menu_source;
    unsigned char card_type[3];
} cards[] = {
    { "collins", "Collins Radio Corporation",
	card_collins_xpm, cardmenu_collins_xpm,
	{ PC_COLOR_CREAM | PC_CORNER_ROUND | PC_CUT_LEFT, 0, 0 } },
    { "cmu", "Carnegie Mellon University",
	card_cmu_xpm, cardmenu_cmu_xpm,
	{ PC_COLOR_YELLOW_STRIPE | PC_CORNER_ROUND | PC_CUT_RIGHT, 0, 0 } },
    { "ibm", "IBM cream",
	card_ibm_xpm, cardmenu_ibm_xpm,
	{ PC_COLOR_CREAM | PC_CORNER_ROUND | PC_CUT_LEFT, 0, 0 } },
    { "harvard", "Harvard University",
	card_harvard_xpm, cardmenu_harvard_xpm,
	{ PC_COLOR_CREAM | PC_CORNER_ROUND | PC_CUT_RIGHT, 0, 0 } },
    { "pink", "Plain pink",
	card_pink_xpm, cardmenu_pink_xpm,
	{ PC_COLOR_PINK | PC_CORNER_ROUND, 0, 0 } },
    { NULL, NULL }
};

#define FROM_CD(cd)	(struct card_image *)(void *)(cd)
#define TO_CD(cd)	(cardimg_t)(void *)(cd)

/* Accessors. */
const char *
cardimg_name(cardimg_t cd)
{
    struct card_image *c = FROM_CD(cd);

    return c->name;
}

const char *
cardimg_desc(cardimg_t cd)
{
    struct card_image *c = FROM_CD(cd);

    return c->description;
}

char **
cardimg_pixmap_source(cardimg_t cd)
{
    struct card_image *c = FROM_CD(cd);

    return c->pixmap_source;
}

char **
cardimg_pixmap_menu_source(cardimg_t cd)
{
    struct card_image *c = FROM_CD(cd);

    return c->pixmap_menu_source;
}

unsigned char *
cardimg_type(cardimg_t cd)
{
    struct card_image *c = FROM_CD(cd);

    return c->card_type;
}

/*
 * Match a card image by name.
 * Returns NULL for failure.
 */
cardimg_t
find_cardimg(const char *name)
{
    int i;

    for (i = 0; cards[i].name != NULL; i++) {
	if (!strcmp(name, cards[i].name)) {
	    return TO_CD(&cards[i]);
	}
    }
    return NULL;
}

/*
 * Return the default card image.
 */
cardimg_t
default_cardimg(void)
{
    return TO_CD(&cards[0]);
}

/*
 * Return the first/next card image.
 * Use NULL to start.
 */
cardimg_t
next_cardimg(cardimg_t curr)
{
    struct card_image *c;

    if (curr == NULL) {
	return TO_CD(&cards[0]);
    }
    c = FROM_CD(curr);
    if (c->name == NULL || (++c)->name == NULL) {
	return NULL;
    }
    return TO_CD(c);
}
