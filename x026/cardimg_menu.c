/*
 * Copyright 1993, 1995, 2002, 2003, 2013 Paul Mattes.  All Rights Reserved.
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted,
 *   provided that the above copyright notice appear in all copies and that
 *   both that copyright notice and this permission notice appear in
 *   supporting documentation.
 *
 * x029 card image menu.
 */

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Command.h>
#include <X11/xpm.h>

#include "jones.h"		/* file format definitions */
#include "charset.h"		/* character set module */
#include "cardimg.h"		/* card image module */
#include "x029.h"		/* global definitions for x029 */
#include "cardimg_menu.h"	/* global definitions for this module */

typedef struct {
    cardimg_t c;
    Pixmap p;
} cmp_t;
static cmp_t *cmp = NULL;
static int num_entries = 0;
static Widget cardimg_button;
static int current_ix = 0;

/* Mouse-over action: display the name. */
void
Hover_action(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    XtVaSetValues(w, XtNlabel, cardimg_name(cmp[current_ix].c), NULL);
}

/* Mouse-leave action: display just the image. */
void
UnHover_action(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    XtVaSetValues(w, XtNlabel, "", NULL);
}

static void
cardimg_button_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    /* Change the picture on the button. */
    current_ix = (current_ix + 1) % num_entries;
    XtVaSetValues(cardimg_button,
	    XtNbackgroundPixmap, cmp[current_ix].p,
	    XtNlabel, cardimg_name(cmp[current_ix].c),
	    NULL);

    /* Change the next image in the hopper. */
    set_next_card_image(cmp[current_ix].c);
}

void
cardimg_menu_init(cardimg_t current_cardimg, Widget container, Dimension x,
	Dimension y)
{
    cardimg_t c;
    Pixmap shapemask;
    XpmAttributes attributes;
    XtTranslations table;

    /* Set up the pixmaps for the menu. */
    attributes.valuemask = XpmSize;
    for (c = next_cardimg(NULL); c != NULL; c = next_cardimg(c)) {
	cmp = (cmp_t *)XtRealloc((XtPointer)cmp,
		(num_entries + 1) * sizeof(cmp_t));
	cmp[num_entries].c = c;
	if (c == current_cardimg) {
	    current_ix = num_entries;
	}
	if (XpmCreatePixmapFromData(display, XtWindow(container),
		    cardimg_pixmap_menu_source(c), &cmp[num_entries].p,
		    &shapemask, &attributes) != XpmSuccess) {
	    XtError("XpmCreatePixmapFromData failed");
	}
	num_entries++;
    }

    /* Set up the menu. */
    cardimg_button = XtVaCreateManagedWidget(
	    "cardimgButton", commandWidgetClass, container,
	    XtNx, x,
	    XtNy, y,
	    XtNwidth, CARDIMG_MENU_WIDTH,
	    XtNheight, CARDIMG_MENU_HEIGHT,
	    XtNlabel, "",
	    XtNbackgroundPixmap, cmp[current_ix].p,
	    XtNborderWidth, 0,
	    XtNresize, False,
	    NULL);
    XtAddCallback(cardimg_button, XtNcallback, cardimg_button_callback, NULL);

    table = XtParseTranslationTable("<EnterWindow>: Hover()\n\
<LeaveWindow>: UnHover()");
    XtOverrideTranslations(cardimg_button, table);
}
