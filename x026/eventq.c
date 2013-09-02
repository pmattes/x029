/*
 * Copyright 1993, 1995, 2002, 2003, 2013 Paul Mattes.  All Rights Reserved.
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted,
 *   provided that the above copyright notice appear in all copies and that
 *   both that copyright notice and this permission notice appear in
 *   supporting documentation.
 *
 * x029 event queue.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/time.h>
#include <inttypes.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Core.h>
#include <X11/Shell.h>
#include <X11/Xatom.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Porthole.h>
#include <X11/xpm.h>

#include "jones.h"		/* file format definitions */
#include "charset.h"		/* character set module */
#include "cardimg.h"		/* card image module */
#include "x029.h"		/* global definitions for x029 */
#include "eventq.h"		/* definitions for this module */

typedef struct event {
    struct event *next;
    enum evtype evtype;
    unsigned char param;
    Boolean interactive;
    int delay;
} event_t;

int eq_count;

static struct timeval tv_set;
static Boolean to_set = False;

static void (*eq_fn[NUM_EVENTS])(int) = {
    queued_DUMMY,
    queued_DATA,
    queued_MULTIPUNCH,
    queued_KEY_LEFT,
    queued_KYBD_RIGHT,
    queued_HOME,
    queued_PAN_RIGHT_BOTH,
    queued_PAN_RIGHT_PRINT,
    queued_PAN_RIGHT_READ,
    queued_PAN_LEFT_PRINT,
    queued_PAN_LEFT_BOTH,
    queued_PAN_UP,
    queued_SLAM,
    queued_NEWCARD,
    queued_FLUX,
    queued_REGISTERED,
    queued_REL_RIGHT,
    queued_POWER_ON,
    queued_PRESS_FEED,
    queued_PRESS_REL,
    queued_EMPTY,
    queued_STACK,
    queued_OFF,
    queued_CLEAR_SEQ
};

static const char *eq_name[NUM_EVENTS] = {
    "DUMMY",
    "DATA",
    "MULTIPUNCH",
    "KEY_LEFT",
    "KYBD_RIGHT",
    "HOME",
    "PAN_RIGHT_BOTH",
    "PAN_RIGHT_PRINT",
    "PAN_RIGHT_READ",
    "PAN_LEFT_PRINT",
    "PAN_LEFT_BOTH",
    "PAN_UP",
    "SLAM",
    "NEWCARD",
    "FLUX",
    "REGISTERED",
    "REL_RIGHT",
    "POWER_ON",
    "PRESS_FEED",
    "PRESS_REL",
    "EMPTY",
    "STACK",
    "OFF",
    "CLEAR_SEQ"
};

static event_t *eq_first, *eq_last;

static void
dump_queue(const char *when)
{
    event_t *e = NULL;
    event_t *prev = NULL;
    int cnt = 0;

    if (!debugging()) {
	return;
    }

    printf("queue(%s):", when);
    if (eq_first == NULL) {
	printf(" (empty)\n");
	return;
    }

    for (e = eq_first; e != NULL; e = e->next) {
	if (prev == NULL) {
	    prev = e;
	    cnt = 1;
	    continue;
	}
	if (prev->evtype == e->evtype && prev->interactive == e->interactive &&
		prev->delay == e->delay) {
	    cnt++;
	    continue;
	}
	printf(" %s(%d)%s", eq_name[prev->evtype], prev->delay,
		prev->interactive? "*": "");
	if (cnt > 1)
	    printf("x%d", cnt);
	prev = e;
	cnt = 1;
    }
    if (cnt) {
	printf(" %s(%d)%s", eq_name[prev->evtype], prev->delay,
		prev->interactive? "*": "");
	if (cnt > 1)
	    printf("x%d", cnt);
    }
    printf("\n");
}

/* Run the event at the front of the queue. */
static void
deq_event(XtPointer data, XtIntervalId *id)
{
    event_t *e;
    struct timeval tv;
    long waited;

    assert(data == (XtPointer)eq_first);
    assert(eq_count);
    assert(to_set);

    to_set = False;

    gettimeofday(&tv, NULL);
    waited = (((tv.tv_sec - tv_set.tv_sec) * 1000000) +
	       (tv.tv_usec - tv_set.tv_usec)) / 1000;
    assert(waited >= eq_first->delay);

    dump_queue("before deq");

    /* Dequeue it. */
    e = eq_first;
    eq_first = e->next;
    if (eq_first == NULL)
	eq_last = NULL;
    --eq_count;

    /* Run it. */
    dbg_printf("run %s(%d)\n", eq_name[e->evtype], e->delay);
    (*eq_fn[e->evtype])(e->param);

    /* Free it. */
    XtFree((XtPointer)e);
    e = NULL;

    /*
     * If there are more events, schedule the next one.
     * Otherwise, see if the demo FSM needs cranking.
     */
    if (eq_count && !to_set) {
	dbg_printf("adding timeout(%d)\n", eq_first->delay);
	gettimeofday(&tv_set, NULL);
	(void) XtAppAddTimeOut(appcontext, eq_first->delay, deq_event,
		(XtPointer)eq_first);
	to_set = True;
    } else {
	demo_fsm();
    }
}

/* Add an event to the back of the queue. */
Boolean
enq_event(enum evtype evtype, unsigned char param, Boolean interactive,
	int delay)
{
    event_t *e;

    if (evtype == DUMMY && delay == 0)
	return True;
    e = (event_t *)XtMalloc(sizeof(*e));
    e->evtype = evtype;
    e->param = param;
    e->interactive = interactive;
    e->delay = delay;

    e->next = NULL;
    if (eq_last != NULL)
	eq_last->next = e;
    else
	eq_first = e;
    eq_last = e;
    dump_queue("after enq");

    if (!eq_count++) {
	assert(to_set == False);
	dbg_printf("adding timeout(%d)\n", delay);
	gettimeofday(&tv_set, NULL);
	(void) XtAppAddTimeOut(appcontext, delay, deq_event, (XtPointer)e);
	to_set = True;
    }
    return True;
}

/* Flush typeahead events from the queue. */
void
flush_typeahead(void)
{
    event_t *e;
    event_t *prev = NULL;
    event_t *next;

    for (e = eq_first; e != NULL; e = next) {
	next = e->next;
	if (e->interactive) {
	    dbg_printf("Flush %s(%d)*\n", eq_name[e->evtype], e->delay);
	    if (prev != NULL) {
		prev->next = e->next;
	    } else {
		eq_first = e->next;
	    }
	    if (e->next == NULL) {
		eq_last = prev;
	    }
	    XtFree((XtPointer)e);
	    eq_count--;
	    continue;
	}
	prev = e;
    }
    dump_queue("after flush");
}
