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
 * x029 definitions from Doug Jones' punched card file convention.
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
