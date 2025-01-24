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
