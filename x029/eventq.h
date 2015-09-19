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

#if !defined(EVENTQ_H) /*[*/
#define EVENTQ_H 1

enum evtype {
    DUMMY,
    DATA,
    MULTIPUNCH,
    KEY_LEFT,
    KYBD_RIGHT,
    HOME,
    PAN_RIGHT_BOTH,
    PAN_RIGHT_PRINT,
    PAN_RIGHT_READ,
    PAN_LEFT_PRINT,
    PAN_LEFT_BOTH,
    PAN_UP,
    SLAM,
    NEWCARD,
    FLUX,
    REGISTERED,
    REL_RIGHT,
    POWER_ON,
    PRESS_FEED,
    PRESS_REL,
    EMPTY,
    STACK,
    OFF,
    CLEAR_SEQ,
    NUM_EVENTS
};

typedef void eq_fn_t(int);

extern eq_fn_t queued_DUMMY;
extern eq_fn_t queued_DATA;
extern eq_fn_t queued_MULTIPUNCH;
extern eq_fn_t queued_KEY_LEFT;
extern eq_fn_t queued_KYBD_RIGHT;
extern eq_fn_t queued_HOME;
extern eq_fn_t queued_PAN_RIGHT_BOTH;
extern eq_fn_t queued_PAN_RIGHT_PRINT;
extern eq_fn_t queued_PAN_RIGHT_READ;
extern eq_fn_t queued_PAN_LEFT_PRINT;
extern eq_fn_t queued_PAN_LEFT_BOTH;
extern eq_fn_t queued_PAN_UP;
extern eq_fn_t queued_SLAM;
extern eq_fn_t queued_NEWCARD;
extern eq_fn_t queued_FLUX;
extern eq_fn_t queued_REGISTERED;
extern eq_fn_t queued_REL_RIGHT;
extern eq_fn_t queued_POWER_ON;
extern eq_fn_t queued_PRESS_FEED;
extern eq_fn_t queued_PRESS_REL;
extern eq_fn_t queued_EMPTY;
extern eq_fn_t queued_STACK;
extern eq_fn_t queued_OFF;
extern eq_fn_t queued_CLEAR_SEQ;

/* Add an event to the back of the queue. */
extern void enq_event(enum evtype evtype, unsigned char param,
	Boolean interactive, int delay);

/* Flush typeahead events from the queue. */
extern void flush_typeahead(void);

extern int eq_count;

#endif /*]*/
