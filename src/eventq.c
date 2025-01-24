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
 * x029 event queue.
 */

#include <stdio.h>
#include <assert.h>
#include <sys/time.h>
#include <X11/Intrinsic.h>

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
    int count;
} event_t;

int eq_count;

static struct timeval tv_set;
static Boolean to_set = False;
static XtIntervalId set_id = 0;

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

    if (!debugging()) {
	return;
    }

    printf("queue(%s):", when);
    if (eq_first == NULL) {
	printf(" (empty)\n");
	return;
    }

    for (e = eq_first; e != NULL; e = e->next) {
	printf(" %s(%d)%s", eq_name[e->evtype], e->delay,
		e->interactive? "*": "");
	if (e->count > 1)
	    printf("x%d", e->count);
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
    Boolean complete = False;

    assert(data == (XtPointer)eq_first);
    assert(eq_count);
    assert(to_set);

    to_set = False;
    set_id = 0;

    gettimeofday(&tv, NULL);
    waited = (((tv.tv_sec - tv_set.tv_sec) * 1000000) +
	       (tv.tv_usec - tv_set.tv_usec)) / 1000;
    assert(waited >= eq_first->delay);

    dump_queue("before deq");

    /* Dequeue it. */
    e = eq_first;
    if (e->count-- < 2) {
	eq_first = e->next;
	if (eq_first == NULL)
	    eq_last = NULL;
	complete = True;
	--eq_count;
    }

    /* Run it. */
    dbg_printf("run %s(%d)\n", eq_name[e->evtype], e->delay);
    (*eq_fn[e->evtype])(e->param);

    /* Free it. */
    if (complete) {
	XtFree((XtPointer)e);
    }
    e = NULL;

    /*
     * If there are more events, schedule the next one.
     * Otherwise, see if the demo FSM needs cranking.
     */
    if (eq_count && !to_set) {
	dbg_printf("adding timeout(%d)\n", eq_first->delay);
	gettimeofday(&tv_set, NULL);
	set_id = XtAppAddTimeOut(appcontext, eq_first->delay, deq_event,
		(XtPointer)eq_first);
	to_set = True;
    } else {
	demo_fsm();
    }
}

/* Add an event to the back of the queue. */
void
enq_event(enum evtype evtype, unsigned char param, Boolean interactive,
	int delay)
{
    event_t *e;

    if (eq_last != NULL &&
	eq_last->evtype == evtype &&
	eq_last->param == param &&
	eq_last->interactive == interactive &&
	eq_last->delay == delay) {
	
	eq_last->count++;
	return;
    }

    e = (event_t *)XtMalloc(sizeof(*e));
    e->evtype = evtype;
    e->param = param;
    e->interactive = interactive;
    e->delay = delay;
    e->count = 1;

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
	set_id = XtAppAddTimeOut(appcontext, delay, deq_event, (XtPointer)e);
	to_set = True;
    }
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
	    if (e == eq_first && to_set) {
		XtRemoveTimeOut(set_id);
		set_id = 0;
		to_set = False;
	    }
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

    /*
     * If we cleared out the lead item (and canceled its timeout) but there is
     * something left, schedule it.
     *
     * This code does not handle modifying the time to wait to account for the
     * time elapsed since the previous timeout was scheduled. It could.
     */
    if (eq_count && !to_set) {
	dbg_printf("adding timeout(%d)\n", eq_first->delay);
	gettimeofday(&tv_set, NULL);
	set_id = XtAppAddTimeOut(appcontext, eq_first->delay, deq_event,
		(XtPointer)eq_first);
	to_set = True;
    }
}
