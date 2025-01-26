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
 * x029 save operations
 */

#include <stdio.h>
#include <errno.h>
#include <stdbool.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Command.h>

#include "jones.h"
#include "charset.h"
#include "cardimg.h"
#include "x029.h"

#include "jones.h"
#include "save.h"

static Boolean save_popup_created = False;
static Widget save_shell, save_dialog;

static void
center_it(Widget w, XtPointer client_data, XtPointer call_data)
{
    Dimension width, height;
    Position toplevel_x, toplevel_y;
    Dimension toplevel_width, toplevel_height;

    /* Get the popup's dimensions */
    XtVaGetValues(w,
	XtNwidth, &width,
	XtNheight, &height,
	NULL);

    /* Get the toplevel position and dimensions. */
    XtVaGetValues(toplevel,
	XtNwidth, &toplevel_width,
	XtNheight, &toplevel_height,
	XtNx, &toplevel_x,
	XtNy, &toplevel_y,
	NULL);

    /* Compute our position relatve to those. */
    XtVaSetValues(w,
	XtNx, toplevel_x + (toplevel_width - width) / 2,
	XtNy, toplevel_y + (toplevel_height - height) / 2,
	NULL);
}

static void
save_file_ascii(void)
{
    char *sfn;
    FILE *f;
    card_t *c;
    int i, j, h;

    XtVaGetValues(save_dialog, XtNvalue, &sfn, NULL);
    f = fopen(sfn, "w");
    if (f == NULL) {
	XBell(display, 100);
	XtVaSetValues(save_dialog,
	    XtNlabel, strerror(errno),
	    NULL);
	XtVaSetValues(XtNameToWidget(save_dialog, XtNlabel),
	    XtNforeground, get_errcolor(),
	    NULL);
	return;
    }
    XtPopdown(save_shell);

    for (c = first_card(); c; c = next_card(c)) {
	for (i = 0; i < N_COLS; i++) {
	    if (!c->n_ov[i]) {
		fputc(' ', f);
		continue;
	    }
	    /* Try to find a single character. */
	    for (h = 0; h < 256; h++) {
		if (h == '\n' || h == '\b')
			continue;
		if (charset_xlate(c->charset, h) != NS &&
		    charset_xlate(c->charset, h) == c->holes[i]) {
		    fputc(h, f);
		    break;
		}
	    }
	    if (h < 256)
		continue;
	    /* Write it with backspaces. */
	    for (j = 0; j < c->n_ov[i]; j++) {
		if (j)
		    fputc('\b', f);
		fputc(c->coltxt[i][j], f);
	    }
	}
	fputc('\n', f);
    }
    fclose(f);
    clear_stacker();
}

static void
save_file_image(void)
{
    char *sfn;
    FILE *f;
    card_t *c;

    XtVaGetValues(save_dialog, XtNvalue, &sfn, NULL);
    f = fopen(sfn, "w");
    if (f == NULL) {
	XBell(display, 100);
	XtVaSetValues(save_dialog,
	    XtNlabel, strerror(errno),
	    NULL);
	XtVaSetValues(XtNameToWidget(save_dialog, XtNlabel),
	    XtNforeground, get_errcolor(),
	    NULL);
	return;
    }
    XtPopdown(save_shell);

    /* Write the header. */
    fprintf(f, "H80");

    /* Write the cards. */
    for (c = first_card(); c; c = next_card(c)) {
	int i;
	unsigned char b3[3];

	fprintf(f, "%c%c%c",
	    0x80 | cardimg_type(c->cardimg)[0],
	    0x80 | cardimg_type(c->cardimg)[1] |
		charset_punch_type(c->charset),
	    0x80 | cardimg_type(c->cardimg)[2]);
	for (i = 0; i < N_COLS; i++) {
	    if (i % 2) {
		b3[1] |= (c->holes[i] >> 8) & 0xf;
		b3[2] = c->holes[i] & 0xff;
		if (fwrite(b3, 1, 3, f) < 3)
		    break;
	    } else {
		b3[0] = c->holes[i] >> 4;
		b3[1] = (c->holes[i] & 0xf) << 4;
	    }
	}
    }
    fclose(f);
    clear_stacker();
}

static void
save_ascii_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    save_file_ascii();
}

static void
save_image_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    save_file_image();
}

static void
cancel_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    XtPopdown(save_shell);
}

void
Confirm_action(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    action_dbg("Confirm", w, event, params, num_params);

    save_file_ascii();
}

void
save_popup(void)
{
    Widget w;

    if (!save_popup_created) {
	Dimension width, height;

	/* Create the shell. */
	save_shell = XtVaCreatePopupShell("save",
	    transientShellWidgetClass, toplevel,
	    XtNtitle, "x029 Save",
	    NULL);
	XtAddCallback(save_shell, XtNpopupCallback, center_it, NULL);
	XtOverrideTranslations(save_shell,
	    XtParseTranslationTable("<Message>WM_PROTOCOLS: DeleteWindow()"));

	/* Create the dialog in the popup. */
	save_dialog = XtVaCreateManagedWidget(
	    "dialog", dialogWidgetClass, save_shell,
	    XtNlabel, "Save File Name",
	    XtNvalue, "",
	    XtNbackground, get_cabinet(),
	    NULL);
	w = XtNameToWidget(save_dialog, XtNvalue);
	XtVaSetValues(w,
	    XtNwidth, 200,
	    NULL);
	XtOverrideTranslations(w,
	    XtParseTranslationTable("<Key>Return: Confirm()"));
	XtVaSetValues(XtNameToWidget(save_dialog, XtNlabel),
	    XtNwidth, 200,
	    XtNbackground, get_cabinet(),
	    NULL);
	if (get_cabinet() == XBlackPixel(display, default_screen))
	    XtVaSetValues(XtNameToWidget(save_dialog, XtNlabel),
		XtNforeground, XWhitePixel(display, default_screen),
		NULL);

	/* Add 'Save' and 'Cancel' buttons to the dialog. */
	w = XtVaCreateManagedWidget("Save Ascii", commandWidgetClass,
	    save_dialog,
	    NULL);
	XtAddCallback(w, XtNcallback, save_ascii_callback, NULL);
	w = XtVaCreateManagedWidget("Save Images", commandWidgetClass,
	    save_dialog,
	    NULL);
	XtAddCallback(w, XtNcallback, save_image_callback, NULL);
	w = XtVaCreateManagedWidget("Cancel", commandWidgetClass,
	    save_dialog,
	    NULL);
	XtAddCallback(w, XtNcallback, cancel_callback, NULL);

	/* Set the width and height. */
	XtRealizeWidget(save_shell);
	XtVaGetValues(save_shell,
	    XtNwidth, &width,
	    XtNheight, &height,
	    NULL);
	XtVaSetValues(save_shell,
	    XtNheight, height,
	    XtNwidth, width,
	    XtNbaseHeight, height,
	    XtNbaseWidth, width,
	    XtNminHeight, height,
	    XtNminWidth, width,
	    XtNmaxHeight, height,
	    XtNmaxWidth, width,
	    NULL);

	save_popup_created = True;
    }
    if (get_cabinet() == XBlackPixel(display, default_screen))
	XtVaSetValues(XtNameToWidget(save_dialog, XtNlabel), XtNforeground,
		XWhitePixel(display, default_screen), NULL);
    else
	XtVaSetValues(XtNameToWidget(save_dialog, XtNlabel), XtNforeground,
		get_foreground(), NULL);
    XtPopup(save_shell, XtGrabExclusive);
    XSetWMProtocols(display, XtWindow(save_shell), &a_delete_me, 1);
}
