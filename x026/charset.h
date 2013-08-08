/*
 * Copyright 1993, 1995, 2002, 2003, 2013 Paul Mattes.  All Rights Reserved.
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted,
 *   provided that the above copyright notice appear in all copies and that
 *   both that copyright notice and this permission notice appear in
 *   supporting documentation.
 *
 * x029 character set support
 */

#if !defined(CHARSET_H) /*[*/
#define CHARSET_H 1

/* Symbolic code for 'no such translation.' */
#define NS	0xffffffffU

/*
 * Character sets.
 * A character set is a 256-element mapping from ISO 8859-1 to punched
 * card code.  The card codes are defined per Douglas Jones's convention:
 *   numeric 9	00001
 *   numeric 8  00002
 *   numeric 7  00004
 *   numeric 6  00010
 *   numeric 5  00020
 *   numeric 4  00040
 *   numeric 3  00100
 *   numeric 2  00200
 *   numeric 1  00400
 *   zone 10    01000
 *   zone 11    02000
 *   zone 12    04000
 */

/* Opaque type. */
typedef struct _fake_charset *charset_t;

/* Accessors. */
extern const char *charset_name(charset_t cs);
extern const char *charset_desc(charset_t cs);
extern punch_type_t charset_punch_type(charset_t cs);
extern unsigned charset_xlate(charset_t cs, unsigned char ix);

/*
 * Match a character set by name.
 * Returns NULL for failure.
 */
extern charset_t find_charset(const char *name);

/*
 * Return the default character set.
 */
extern charset_t default_charset(void);

/*
 * Return the first/next character set.
 * Use NULL to start.
 */
extern charset_t next_charset(charset_t curr);

#endif /*]*/
