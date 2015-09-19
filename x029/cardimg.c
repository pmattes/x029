/*
 * Copyright 1993, 1995, 2002, 2003, 2013 Paul Mattes.  All Rights Reserved.
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted,
 *   provided that the above copyright notice appear in all copies and that
 *   both that copyright notice and this permission notice appear in
 *   supporting documentation.
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
