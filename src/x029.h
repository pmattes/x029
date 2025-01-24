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
 * x029 global symbols.
 */

#if !defined(X029_H) /*[*/
# define X029_H 1

# define N_COLS	80	/* number of columns (80, of course) */
# define N_ROWS	12	/* number of rows */
# define N_OV	10	/* number of overpunches */

#define CARDIMG_MENU_WIDTH	100
#define CARDIMG_MENU_HEIGHT	44

typedef struct card {
    struct card *next;
    cardimg_t cardimg;
    charset_t charset;
    unsigned short holes[N_COLS];
    int n_ov[N_COLS];
    unsigned char coltxt[N_COLS][N_OV];
    int seq;
} card_t;

extern Widget toplevel;
extern Display *display;
extern int default_screen;
extern Atom a_delete_me;
extern XtAppContext appcontext;

extern Boolean add_char(char c);
extern card_t *first_card(void);
extern card_t *next_card(card_t *c);
extern void clear_stacker(void);
extern void set_next_card_image(cardimg_t c);
extern int set_charset(charset_t c);

extern Pixel get_errcolor(void);
extern Pixel get_cabinet(void);
extern Pixel get_foreground(void);
extern charset_t get_charset(void);
extern Boolean debugging(void);
extern void demo_fsm(void);

extern void dbg_printf(const char *fmt, ...);
extern void dbg_cprintf(const char *fmt, ...);
extern void action_dbg(const char *name, Widget wid, XEvent *event,
	String *params, Cardinal *num_params);

#endif /*]*/
