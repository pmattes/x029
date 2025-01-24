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
 * x029 character set menu.
 */

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Command.h>
#include <X11/xpm.h>

#include "jones.h"		/* file format definitions */
#include "charset.h"		/* character set module */
#include "cardimg.h"		/* card image module */
#include "x029.h"		/* global definitions for x029 */
#include "charset_menu.h"	/* global definitions for this module */

#include "charset.xpm"		/* button background */

charset_t *cs = NULL;
static int cs_num_entries = 0;
static Widget charset_button;
static int cs_current_ix = 0;

/* Mouse-over action: display the name. */
void
Hover2_action(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    XtVaSetValues(w, XtNlabel, charset_name(get_charset()), NULL);
}

static void
charset_button_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    int next_ix = (cs_current_ix + 1) % cs_num_entries;

    if (set_charset(cs[next_ix]) < 0) {
	XBell(display, 0);
	return;
    }

    /* Change the label on the button. */
    cs_current_ix = next_ix;
    XtVaSetValues(charset_button,
	    XtNlabel, charset_name(cs[cs_current_ix]),
	    NULL);

    /* Change the next image in the hopper. */
    set_charset(cs[cs_current_ix]);
}

void
charset_menu_init(charset_t current_charset, Widget container, Dimension x,
	Dimension y)
{
    charset_t c;
    XtTranslations table;
    XpmAttributes attributes;
    Pixmap pixmap, shapemask;

    /* Set up the list of names. */
    for (c = next_charset(NULL); c != NULL; c = next_charset(c)) {
	cs = (charset_t *)XtRealloc((XtPointer)cs,
		(cs_num_entries + 1) * sizeof(charset_t));
	cs[cs_num_entries] = c;
	if (c == current_charset) {
	    cs_current_ix = cs_num_entries;
	}
	cs_num_entries++;
    }

    /* Set up the bitmap. */
    attributes.valuemask = XpmSize;
    if (XpmCreatePixmapFromData(display, XtWindow(container),
		charset_xpm, &pixmap, &shapemask, &attributes) != XpmSuccess) {
	XtError("XpmCreatePixmapFromData failed");
    }

    /* Set up the menu. */
    charset_button = XtVaCreateManagedWidget(
	    "charsetButton", commandWidgetClass, container,
	    XtNx, x,
	    XtNy, y,
	    XtNwidth, CARDIMG_MENU_WIDTH,
	    XtNheight, CARDIMG_MENU_HEIGHT,
	    XtNlabel, "",
	    XtNborderWidth, 0,
	    XtNresize, False,
	    XtNbackgroundPixmap, pixmap,
	    NULL);
    XtAddCallback(charset_button, XtNcallback, charset_button_callback, NULL);

    table = XtParseTranslationTable("<EnterWindow>: Hover2()\n\
<LeaveWindow>: UnHover()");
    XtOverrideTranslations(charset_button, table);
}
