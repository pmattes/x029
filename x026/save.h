/*
 * Copyright 1993, 1995, 2002, 2003, 2013 Paul Mattes.  All Rights Reserved.
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted,
 *   provided that the above copyright notice appear in all copies and that
 *   both that copyright notice and this permission notice appear in
 *   supporting documentation.
 *
 * x029 save operations
 */

#if !defined(SAVE_H) /*[*/
# define SAVE_H 1

extern void save_popup(void);
extern void confirm_action(Widget w, XEvent *event, String *params,
	Cardinal *num_params);

#endif /*]*/
