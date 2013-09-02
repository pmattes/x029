/*
 * Copyright 1993, 1995, 2002, 2003, 2013 Paul Mattes.  All Rights Reserved.
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted,
 *   provided that the above copyright notice appear in all copies and that
 *   both that copyright notice and this permission notice appear in
 *   supporting documentation.
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
