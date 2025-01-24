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
