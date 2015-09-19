/*
 * Copyright 1993, 1995, 2002, 2003, 2013 Paul Mattes.  All Rights Reserved.
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted,
 *   provided that the above copyright notice appear in all copies and that
 *   both that copyright notice and this permission notice appear in
 *   supporting documentation.
 *
 * x029 card images
 */

#if !defined(CARDIMG_H) /*[*/
#define CARDIMG_H 1

/* Opaque type. */
typedef struct _fake_cardimg *cardimg_t;

/* Accessors. */
extern const char *cardimg_name(cardimg_t cd);
extern const char *cardimg_desc(cardimg_t cd);
extern char **cardimg_pixmap_source(cardimg_t cd);
extern char **cardimg_pixmap_menu_source(cardimg_t cd);
extern unsigned char *cardimg_type(cardimg_t cd);

/*
 * Match a card image by name.
 * Returns NULL for failure.
 */
extern cardimg_t find_cardimg(const char *name);

/*
 * Return the default card image.
 */
extern cardimg_t default_cardimg(void);

/*
 * Return the first/next card image.
 * Use NULL to start.
 */
extern cardimg_t next_cardimg(cardimg_t curr);

#endif /*]*/
