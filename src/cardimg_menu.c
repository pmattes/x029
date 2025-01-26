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
 * x029 card image menu.
 */

#include <stdbool.h>

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
