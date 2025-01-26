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
 * x029 paste operations.
 */

#include <stdbool.h>

#include <X11/Intrinsic.h>
#include <X11/Xatom.h>

#include "jones.h"
#include "cardimg.h"
#include "charset.h"
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
	add_paste_char(*s++);
    }
    n_pasting = 0;
    poke_fsm();

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
