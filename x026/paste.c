/*
 * Copyright 1993, 1995, 2002, 2003, 2013 Paul Mattes.  All Rights Reserved.
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted,
 *   provided that the above copyright notice appear in all copies and that
 *   both that copyright notice and this permission notice appear in
 *   supporting documentation.
 *
 * x029 paste operations.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Core.h>
#include <X11/Shell.h>
#include <X11/Xatom.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Porthole.h>
#include <X11/xpm.h>

#include "x029.h"
#include "paste.h"

#define NP	5
static Atom paste_atom[NP];
static int n_pasting = 0;
static int pix = 0;
static Time paste_time;

/* Pasting support. */
static void
paste_callback(Widget w, XtPointer client_data, Atom *selection, Atom *type,
    XtPointer value, unsigned long *length, int *format)
{
    char *s;
    unsigned long len;

    if ((value == NULL) || (*length == 0)) {
	XtFree(value);

	/* Try the next one. */
	if (n_pasting > pix)
	    XtGetSelectionValue(w, paste_atom[pix++], XA_STRING,
		    paste_callback, NULL, paste_time);
	return;
    }

    s = (char *)value;
    len = *length;
    while (len--) {
	unsigned char c = *s++;

	if (c == '\n')
	    break;
	if (c < ' ') {
	    continue;
	}
	if (!add_char(c)) {
	    break;
	}
    }
    n_pasting = 0;

    XtFree(value);
}

void
InsertSelection_action(Widget w, XEvent *event, String *params,
	Cardinal *num_params)
{
    Cardinal i;
    Atom a;
    XButtonEvent *bevent = (XButtonEvent *)event;

    action_dbg("InsertSelection", w, event, params, num_params);

    n_pasting = 0;
    for (i = 0; i < *num_params; i++) {
	a = XInternAtom(display, params[i], True);
	if (a == None) {
	    XtWarning("no atom for selection");
	    continue;
	}
	if (n_pasting < NP)
	    paste_atom[n_pasting++] = a;
    }
    pix = 0;
    if (n_pasting > pix) {
	paste_time = bevent->time;
	XtGetSelectionValue(w, paste_atom[pix++], XA_STRING, paste_callback,
		NULL, paste_time);
    }
}
