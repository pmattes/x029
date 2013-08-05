/*
 * Copyright 1993, 1995, 2002, 2003, 2013 Paul Mattes.  All Rights Reserved.
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted,
 *   provided that the above copyright notice appear in all copies and that
 *   both that copyright notice and this permission notice appear in
 *   supporting documentation.
 *
 * x026 definitions from Doug Jones' punched card file convention.
 */

#if !defined(JONES_H) /*[*/
#define JONES_H 1

typedef enum {
    PTYPE_NONPRINTING = 0,
    PTYPE_026_COMMERCIAL = 0x08,
    PTYPE_026_FORTRAN = 0x10,
    PTYPE_029 = 0x20
} punch_type_t;

#define PC_COLOR_CREAM		(0 << 3)
#define PC_COLOR_WHITE		(1 << 3)
#define PC_COLOR_YELLOW		(2 << 3)
#define PC_COLOR_PINK		(3 << 3)
#define PC_COLOR_BLUE		(4 << 3)
#define PC_COLOR_GREEN		(5 << 3)
#define PC_COLOR_ORANGE		(6 << 3)
#define PC_COLOR_BROWN		(7 << 3)
#define PC_COLOR_YELLOW_STRIPE	(10 << 3)
#define PC_COLOR_PINK_STRIPE	(11 << 3)
#define PC_COLOR_BLUE_STRIPE	(12 << 3)
#define PC_COLOR_GREEN_STRIPE	(13 << 3)
#define PC_COLOR_ORANGE_STRIPE	(14 << 3)
#define PC_COLOR_BROWN_STRIPE	(15 << 3)

#define PC_CORNER_ROUND		(0 << 2)
#define PC_CORNER_SQUARE	(1 << 2)

#define PC_CUT_NEITHER		0
#define PC_CUT_RIGHT		1
#define PC_CUT_LEFT		2
#define PC_CUT_BOTH		3

#endif /*]*/
