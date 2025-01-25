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
void enq_event(enum evtype evtype, unsigned char param, Boolean interactive, int delay);

/* Flush typeahead events from the queue. */
void flush_typeahead(void);

extern int eq_count;

#endif /*]*/
