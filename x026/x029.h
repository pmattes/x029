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

typedef struct card {
    unsigned short holes[N_COLS];
    int n_ov[N_COLS];
    unsigned char coltxt[N_COLS][N_OV];
    int seq;
    struct card *prev;
    struct card *next;
} card;
extern card *ccard;

extern Widget toplevel;
extern Display *display;
extern int default_screen;
extern charset_t ccharset;
extern cardimg_t ccardimg;

extern Boolean add_char(char c);
extern card *first_card(void);
extern Pixel get_errcolor(void);
extern Pixel get_cabinet(void);
extern Pixel get_foreground(void);

#endif /*]*/
