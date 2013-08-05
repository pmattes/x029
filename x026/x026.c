/*
 * Copyright 1993, 1995, 2002, 2003, 2013 Paul Mattes.  All Rights Reserved.
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted,
 *   provided that the above copyright notice appear in all copies and that
 *   both that copyright notice and this permission notice appear in
 *   supporting documentation.
 *
 * x026 -- A Keypunch Simluator
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Core.h>
#include <X11/Shell.h>
#include <X11/Xatom.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Porthole.h>
#include <X11/xpm.h>

#include "jones.h"
#include "charset.h"

#include "collins.xpm"		/* card image */
#include "carnegie2.xpm"	/* card image */
#include "ibm.xpm"		/* card image */

#include "hole.xpm"		/* hole image */

#include "off60.xpm"		/* switch, off */
#include "on60.xpm"		/* switch, on */

#include "x026.bm"		/* icon */

struct card_type {
	char *name;
	char *description;
	char **pixmap_source;
	unsigned char card_type[3];
} cards[] = {
	{ "collins", "Collins Radio Corporation", collins,
		{ PC_COLOR_CREAM | PC_CORNER_ROUND | PC_CUT_LEFT,
		  0, 0 } },
	{ "cmu", "Carnegie Mellon University", carnegie2,
		{ PC_COLOR_YELLOW_STRIPE | PC_CORNER_ROUND | PC_CUT_RIGHT,
		  0, 0 } },
	{ "ibm", "IBM", ibm,
		{ PC_COLOR_CREAM | PC_CORNER_ROUND | PC_CUT_LEFT,
		  0, 0 } },
	{ NULL, NULL }
};

char *top_label[] = { "ON", NULL, "ONE", "ON", "ON", "ON", NULL, "ON" };
char *bottom_label1[] = { "AUTO", NULL, "TWO", "AUTO", "PRINT", "LZ", NULL, "CLEAR" };
char *bottom_label2[] = { "SKIP", NULL, "PROG", "FEED", NULL, "PRINT", NULL, NULL };
char *bottom_label3[] = { "DUP", NULL, "SEL", NULL, NULL, NULL, NULL, NULL };

#define VERY_SLOW 500
#define SLOW	75
#define FAST	25

#define CELL_X_NUM	693
#define CELL_X_DENOM	80
#define CELL_WIDTH	(CELL_X_NUM / CELL_X_DENOM)
#define CELL_X(col)	(((col) * CELL_X_NUM) / CELL_X_DENOM)
#define COL_FROM_X(x)	(((x) * CELL_X_DENOM) / CELL_X_NUM)

#define CELL_Y_NUM	296
#define CELL_Y_DENOM	12
#define CELL_HEIGHT	(CELL_Y_NUM / CELL_Y_DENOM)
#define CELL_Y(row)	(((row) * CELL_Y_NUM) / CELL_Y_DENOM)
#define ROW_FROM_Y(y)	(((y) * CELL_Y_DENOM) / CELL_Y_NUM)

#define HOLE_WIDTH	5	/* unused for now */
#define HOLE_HEIGHT	11	/* unused for now */

#define SWITCH_AIR	40
#define SWITCH_HEIGHT	60
#define SWITCH_WIDTH	42
#define SWITCH_SKIP	(2*SWITCH_AIR + SWITCH_HEIGHT)

#define TOP_PAD		15
#define TEXT_PAD	8
#define HOLE_PAD	11
#define LEFT_PAD	31
#define RIGHT_PAD	15
#define BOTTOM_PAD	15
#define CARD_AIR	5

#define	BUTTON_GAP	5
#define BUTTON_BW	2
#define BUTTON_WIDTH	50
#define	BUTTON_HEIGHT	20

static char		*programname;
static Widget		toplevel;
static XtAppContext	appcontext;
static Display		*display;
static int		default_screen;
static int		root_window;
static int		card_window;
static int		depth;
static XFontStruct	*ifontinfo;
static Atom		a_delete_me;
static int		line_number = 100;
static Pixmap		hole_pixmap;
charset_t		ccharset = NULL;
struct card_type	*ccard_type = NULL;

int			batchfd = -1;

typedef enum {
	M_INTERACTIVE,	/* interactive */
	M_BATCH,	/* read from a fixed file */
	M_REMOTECTL	/* read from stdin incrementally */
} imode_t;
imode_t mode = M_INTERACTIVE;

/* Internal actions. */
static void do_nothing(int);
static void do_data(int);
static void do_multipunch(int);
static void do_left(int);
static void do_right(int);
static void do_home(int);
static void do_pan_right(int);
static void do_pan_left(int);
static void do_pan_up(int);
static void do_slam(int);
static void do_newcard(int);

static void save(int ignored);

/* Application resources. */
typedef struct {
	Pixel	foreground;
	Pixel	background;
	Pixel	cabinet;
	Pixel	cardcolor;
	Pixel	errcolor;
	char	*ifontname;
	char	*charset;
	char	*card;
	char	*batchfile;
	Boolean	autonumber;
	Boolean	typeahead;
	Boolean remotectl;
	Boolean	help;
	Boolean debug;
} AppRes, *AppResptr;

static AppRes appres;

/* Command-line options. */
static XrmOptionDescRec options[]= {
	{ "-ifont",	".ifont",	XrmoptionSepArg,	NULL },
	{ "-nonumber",	".autoNumber",  XrmoptionNoArg,		"False" },
	{ "-typeahead",	".typeahead",	XrmoptionNoArg,		"True" },
	{ "-mono",	".cabinet",	XrmoptionNoArg,		"black" },
	{ "-charset",	".charset",	XrmoptionSepArg,	NULL },
	{ "-card",	".card",	XrmoptionSepArg,	NULL },
	{ "-batch",	".batchFile",	XrmoptionSepArg,	NULL },
	{ "-remotectl",	".remoteCtl",	XrmoptionNoArg,		"True" },

	/* Options compatible with Douglas Jones's tools */
	{ "-026ftn",	".charset",	XrmoptionNoArg,		"bcd-h" },
	{ "-026comm",	".charset",	XrmoptionNoArg,		"bcd-a" },
	{ "-029",	".charset",	XrmoptionNoArg,		"029" },
	{ "-EBCDIC",	".charset",	XrmoptionNoArg,		"ebcdic" },
	{ "-debug",	".debug",	XrmoptionNoArg,		"True" },
	
	{ "-help",	".help",	XrmoptionNoArg,		"True" },
};

/* Resource list. */
#define offset(field) XtOffset(AppResptr, field)
static XtResource resources[] = {
	{ XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
	  offset(foreground), XtRString, "XtDefaultForeground" },
	{ XtNbackground, XtCBackground, XtRPixel, sizeof(Pixel),
	  offset(background), XtRString, "XtDefaultBackground" },
	{ "cabinet", "Cabinet", XtRPixel, sizeof(Pixel),
	  offset(cabinet), XtRString, "grey92" },
	{ "cardColor", "CardColor", XtRPixel, sizeof(Pixel),
	  offset(cardcolor), XtRString, "ivory" },
	{ "errColor", "ErrColor", XtRPixel, sizeof(Pixel),
	  offset(errcolor), XtRString, "firebrick" },
	{ "ifont", "IFont", XtRString, sizeof(String),
	  offset(ifontname), XtRString, 0 },
	{ "autoNumber", "AutoNumber", XtRBoolean, sizeof(Boolean),
	  offset(autonumber), XtRString, "True" },
	{ "typeahead", "Typeahead", XtRBoolean, sizeof(Boolean),
	  offset(typeahead), XtRString, "True" },
	{ "charset", "Charset", XtRString, sizeof(String),
	  offset(charset), XtRString, "bcd-h" },
	{ "card", "Card", XtRString, sizeof(String),
	  offset(card), XtRString, "collins" },
	{ "batchFile", "BatchFile", XtRString, sizeof(String),
	  offset(batchfile), XtRString, NULL },
	{ "remoteCtl", "RemoteCtl", XtRBoolean, sizeof(Boolean),
	  offset(remotectl), XtRString, "False" },
	{ "debug", "Debug", XtRBoolean, sizeof(Boolean),
	  offset(debug), XtRString, "False" },
	{ "help", "Help", XtRBoolean, sizeof(Boolean),
	  offset(help), XtRString, "False" },
};
#undef offset

/* Fallback resources. */
static String fallbacks[] = {
	"*ifont:	7x13",
	"*pos.font:	6x13bold",
	"*dialog*value*font: fixed",
	"*switch.font:  6x10",
	"*switch.background:  grey92",
	"*font:		variable",
	"*cabinet:	grey92",
	"*cardColor:	ivory",
	NULL
};

/* Xt actions. */
static void data(Widget, XEvent *, String *, Cardinal *);
static void multi_punch_data(Widget, XEvent *, String *, Cardinal *);
static void delete_window(Widget, XEvent *, String *, Cardinal *);
static void home(Widget, XEvent *, String *, Cardinal *);
static void left(Widget, XEvent *, String *, Cardinal *);
static void next(Widget, XEvent *, String *, Cardinal *);
static void redraw(Widget, XEvent *, String *, Cardinal *);
static void right(Widget, XEvent *, String *, Cardinal *);
static void tab(Widget, XEvent *, String *, Cardinal *);
static void insert_selection(Widget, XEvent *, String *, Cardinal *);
static void confirm(Widget, XEvent *, String *, Cardinal *);

/* Xt callbacks. */
static void discard(Widget, XtPointer, XtPointer);

/* Actions. */
XtActionsRec actions[] = {
	{ "Data",	data },
	{ "MultiPunchData", multi_punch_data },
	{ "DeleteWindow", delete_window },
	{ "Home",	home },
	{ "Left",	left },
	{ "Next",	next },
	{ "Redraw",	redraw },
	{ "Right",	right },
	{ "Tab",	tab },
	{ "insert-selection", insert_selection },
	{ "confirm",	confirm }
};
int actioncount = XtNumber(actions);

/* Forward references. */
static void define_widgets(void);
#define NC_ALWAYS_SCROLL	True
#define NC_SCROLL_IF_CARD	False
#define NC_INTERACTIVE		True
#define NC_BATCH		False
static void get_new_card(void);
static void save_popup(void);
static Boolean add_char(char c);
static void enq_quit(void);
static void enq_delay(void);
static void card_complete(int delay);
static void batch_fsm(void);

/* Syntax. */
void
usage(void)
{
	charset_t c = NULL;
	int i;

	fprintf(stderr, "Usage: %s [x026-options] [Xt-options]\n",
			programname);
	fprintf(stderr, "x026-options:\n\
  -ifont <font>    Interpreter (card edge) font, defaults to 7x13\n\
  -nonumber        Do not automatically number cards in cols 73..80\n\
  -charset <name>  Keypunch character set:\n");
	for (c = next_charset(NULL); c != NULL; c = next_charset(c)) {
		fprintf(stderr, "    %-9s %s\n",
			charset_name(c), charset_desc(c));
	}
	fprintf(stderr, "\
  -card <name>     Card image:\n");
	for (i = 0; cards[i].name != NULL; i++)
		fprintf(stderr, "    %-9s %s\n",
			cards[i].name, cards[i].description);
	fprintf(stderr, "\
  -026ftn          Alias for '-charset bcd-h'\n\
  -026comm         Alias for '-charset bcd-a'\n\
  -029             Alias for '-charset 029'\n\
  -EBCDIC          Alias for '-charset ebcdic'\n\
  -batch <file>    Read text file and punch it (automated display)\n\
  -batch -         Read stdin and punch it\n\
  -remotectl       Read stdin incrementally\n\
  -help            Display this text\n");
	exit(1);
}

int
main(int argc, char *argv[])
{
	XtTranslations table;
	Pixmap icon;
	int i;

	/* Figure out who we are */
	programname = strrchr(argv[0], '/');
	if (programname)
		++programname;
	else
		programname = argv[0];

	/* Initialze Xt and fetch our resources. */
	toplevel = XtVaAppInitialize(
	    &appcontext,
	    "X026",
	    options, XtNumber(options),
	    &argc, argv,
	    fallbacks,
	    XtNinput, True,
	    XtNallowShellResize, False,
	    NULL);
	if (argc > 1)
		usage();
	XtGetApplicationResources(toplevel, &appres, resources,
	    XtNumber(resources), 0, 0);
	if (appres.help)
		usage();

	/* Set up some globals. */
	display = XtDisplay(toplevel);
	default_screen = DefaultScreen(display);
	root_window = RootWindow(display, default_screen);
	depth = DefaultDepthOfScreen(XtScreen(toplevel));
	a_delete_me = XInternAtom(display, "WM_DELETE_WINDOW", False);

	/* Set up actions. */
	XtAppAddActions(appcontext, actions, actioncount);

	/* Load fonts. */
	ifontinfo = XLoadQueryFont(display, appres.ifontname);
	if (ifontinfo == NULL)
		XtError("Can't load interpreter font");

	/* Pick out the character set. */
	ccharset = find_charset(appres.charset);
	if (ccharset == NULL) {
		ccharset = default_charset();
		fprintf(stderr, "No such charset: '%s', "
				"defaulting to '%s'\n"
		                "Use '-help' to list the available "
				"character sets\n",
				appres.charset, charset_name(ccharset));
	}

	/* Define the widgets. */
	define_widgets();

	/* Set up a cute (?) icon. */
	icon = XCreateBitmapFromData(display, XtWindow(toplevel),
	    (char *)x026_bits, x026_width, x026_height);
	XtVaSetValues(toplevel, XtNiconPixmap, icon, XtNiconMask, icon, NULL);

	/* Allow us to die gracefully. */
	XSetWMProtocols(display, XtWindow(toplevel), &a_delete_me, 1);
	table = XtParseTranslationTable(
	    "<Message>WM_PROTOCOLS: DeleteWindow()");
	XtOverrideTranslations(toplevel, table);

#if defined(SOUND) /*[*/
	/* Set up clicks. */
	audio_init();
#endif /*]*/

	if (appres.batchfile != NULL) {
		if (appres.remotectl) {
			fprintf(stderr, "Batchfile and remotectl in conflict, "
				"ignoring remotectl\n");
		}
		if (strcmp(appres.batchfile, "-")) {
			batchfd = open(appres.batchfile, O_RDONLY | O_NONBLOCK);
			if (batchfd < 0) {
				perror(appres.batchfile);
				exit(1);
			}
		} else {
			batchfd = fileno(stdin);
		}
		mode = M_BATCH;
	} else if (appres.remotectl) {
		mode = M_REMOTECTL;
		batchfd = fileno(stdin);
	} else {
		mode = M_INTERACTIVE;
	}

	if (mode != M_INTERACTIVE && batchfd == fileno(stdin)) {
		if (fcntl(batchfd, F_SETFL,
			    fcntl(batchfd, F_GETFL) | O_NONBLOCK) < 0) {
			perror("fcntl");
			exit(1);
		}
	}

	if (mode == M_INTERACTIVE || mode == M_REMOTECTL) {
		get_new_card();
	}
	if (mode != M_INTERACTIVE) {
		batch_fsm();
	}

	/* Process X events forever. */
	XtAppMainLoop(appcontext);

	return 0;
}


/* Definitions for buttons along the bottom left. */

struct button {
	char *label;
	Boolean batch_disable;
	void (*callback)(int);
} button[] = {
	{ "Off", False, (void (*)())exit },
	{ "Save", True, save },
	{ NULL, False, NULL },
};

/* Callback function for buttons. */
static void
button_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
	struct button *b = (struct button *)client_data;

	(*b->callback)(0);
}

/* Definitions for the toggle switches. */

static Pixmap on, off;

struct toggle {
	Widget w;
	int on;
};
struct toggle toggles[8];

/* Timeout function for the clear switch. */
static void
unclear_event(XtPointer data, XtIntervalId *id)
{
	toggles[7].on = 0;
	XtVaSetValues(toggles[7].w, XtNbackgroundPixmap, off, NULL);
}

/* Callback function for toggle switches. */
static void
toggle_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
	struct toggle *t = (struct toggle *)client_data;

	if (t != &toggles[7]) {
		t->on = !t->on;
		XtVaSetValues(w, XtNbackgroundPixmap, t->on? on: off, NULL);
		return;
	}

	/* It's the clear switch. */
	if (t->on)
	    	return;
	t->on = !t->on;
	XtVaSetValues(w, XtNbackgroundPixmap, t->on? on: off, NULL);
	(void) XtAppAddTimeOut(appcontext, SLOW * 6, unclear_event, NULL);
}

/* Card-image data structures. */

#define N_COLS	80	/* number of columns (80, of course) */
#define N_ROWS	12	/* number of rows */
#define N_OV	10	/* number of overpunches */

typedef struct card {
	unsigned short holes[N_COLS];
	int n_ov[N_COLS];
	unsigned char coltxt[N_COLS][N_OV];
	int seq;
	struct card *prev;
	struct card *next;
} card;
static card *ccard;

static int col = 0;
static Boolean card_visible = False;
static GC gc, invgc, holegc, corner_gc;

static Widget container, porth, scrollw, cardw, posw;
static int scrollw_column;
#define SCROLLW_X()	(-(LEFT_PAD+CELL_X(scrollw_column)))

static Dimension card_width, card_height;
static Dimension hole_width, hole_height;

/* Set up the keypunch. */
static void
define_widgets(void)
{
	Dimension w, h;
	Widget ww;
	XtTranslations t;
	XGCValues xgcv;
	int i;
	Pixmap pixmap, shapemask;
	Pixmap hole_shapemask;
	Dimension sx;
	XpmAttributes attributes;
	static char translations[] = "\
		<Key>Left:	Left()\n\
		<Key>BackSpace:	Left()\n\
		<Key>Right:	Right()\n\
		<Key>space:	Right()\n\
		<Key>Home:	Home()\n\
		<Key>Return:	Next()\n\
		<Key>KP_Enter:	Home()\n\
		<Key>Down:	Next()\n\
		<Key>Tab:	Tab()\n\
		<Btn2Down>:	insert-selection(PRIMARY)\n\
		Alt<Key>:	MultiPunchData()\n\
		Meta<Key>:	MultiPunchData()\n\
		<Key>:		Data()\n";

	/* Create a container for the whole thing. */
	container = XtVaCreateManagedWidget(
	    "container", compositeWidgetClass, toplevel,
	    XtNwidth, 10,
	    XtNheight, 10,
	    XtNbackground, appres.cabinet,
	    NULL);
	XtRealizeWidget(toplevel);

	/* Figure out the pixmap. */
	for (i = 0; cards[i].name != NULL; i++) {
		if (!strcmp(appres.card, cards[i].name)) {
			ccard_type = &cards[i];
			break;
		}
	}
	if (cards[i].name == NULL) {
		fprintf(stderr, "No such card '%s', defaulting to '%s'\n"
				"Use '-help' to list the types\n",
				appres.card, cards[0].name);
		ccard_type = &cards[0];
	}
	attributes.valuemask = XpmSize;
	if (XpmCreatePixmapFromData(display, XtWindow(container),
			ccard_type->pixmap_source, &pixmap, &shapemask,
			&attributes) != XpmSuccess) {
		XtError("XpmCreatePixmapFromData failed");
	}
	card_width = attributes.width;
	card_height = attributes.height;
	attributes.valuemask = XpmSize;
	if (XpmCreatePixmapFromData(display, XtWindow(container), hole,
			&hole_pixmap, &hole_shapemask, &attributes) != XpmSuccess) {
		XtError("XpmCreatePixmapFromData failed");
	}
	hole_width = attributes.width;
	hole_height = attributes.height;
	w = card_width + 2*CARD_AIR;
	h = SWITCH_SKIP + card_height + 2*CARD_AIR + 2*BUTTON_GAP + 2*BUTTON_BW + BUTTON_HEIGHT;
	XtVaSetValues(container,
	    XtNwidth, w,
	    XtNheight, h,
	    NULL);

	/* Create the porthole within the container. */
	porth = XtVaCreateManagedWidget(
	    "porthole", portholeWidgetClass, container,
	    XtNwidth, card_width,
	    XtNheight, card_height,
	    XtNx, CARD_AIR,
	    XtNy, CARD_AIR,
	    XtNborderWidth, 0,
	    NULL);

	/* Create scrollable region within the porthole. */
	scrollw_column = 40;
	scrollw = XtVaCreateManagedWidget(
	    "scroll", compositeWidgetClass, porth,
	    XtNwidth, card_width * 3,
	    XtNheight, card_height * 4,
	    XtNx, SCROLLW_X(),
	    XtNy, -(2*card_height + TOP_PAD),
	    XtNbackground, appres.cabinet,
	    XtNborderWidth, 0,
	    NULL);

	/* Create the card itself. */
	cardw = XtVaCreateManagedWidget(
	    "card", compositeWidgetClass, scrollw,
	    XtNwidth, card_width,
	    XtNheight, card_height,
	    XtNx, card_width,
	    XtNy, card_height,
	    XtNborderWidth, 0,
	    XtNbackground, appres.cardcolor,
	    XtNbackgroundPixmap, pixmap,
	    NULL);

	/* Create the buttons in the lower left. */
	for (i = 0; button[i].label; i++) {
		ww = XtVaCreateManagedWidget(
		    button[i].label, commandWidgetClass, container,
		    XtNwidth, BUTTON_WIDTH,
		    XtNheight, BUTTON_HEIGHT,
		    XtNx, BUTTON_GAP + i*(2*BUTTON_BW + BUTTON_WIDTH + BUTTON_GAP),
		    XtNy, card_height + 2*CARD_AIR + SWITCH_SKIP + BUTTON_GAP,
		    XtNborderWidth, BUTTON_BW,
		    XtNborderColor, appres.background,
		    XtNsensitive, !(button[i].batch_disable &&
				    mode != M_INTERACTIVE),
		    NULL
		);
		XtAddCallback(ww, XtNcallback, button_callback,
		    (XtPointer)&button[i]);
	}

	/* Add the position counter in the lower right. */
	posw = XtVaCreateManagedWidget(
	    "pos", labelWidgetClass, container,
	    XtNlabel, "0",
	    XtNwidth, BUTTON_WIDTH,
	    XtNx, w - BUTTON_GAP - BUTTON_WIDTH - 2*BUTTON_BW,
	    XtNy, card_height + 2*CARD_AIR + SWITCH_SKIP + BUTTON_GAP,
	    XtNheight, BUTTON_HEIGHT,
	    XtNborderWidth, BUTTON_BW,
	    XtNborderColor, appres.background,
	    XtNresize, False,
	    NULL);

	/* Add the discard button. */
	ww = XtVaCreateManagedWidget(
	    "discard", commandWidgetClass, container,
	    XtNlabel, "Discard",
	    XtNwidth, BUTTON_WIDTH,
	    XtNx, w - 2* (BUTTON_GAP + BUTTON_WIDTH + 2*BUTTON_BW),
	    XtNy, card_height + 2*CARD_AIR + SWITCH_SKIP + BUTTON_GAP,
	    XtNheight, BUTTON_HEIGHT,
	    XtNborderWidth, BUTTON_BW,
	    XtNborderColor, appres.background,
	    XtNsensitive, mode == M_INTERACTIVE,
	    NULL);
	XtAddCallback(ww, XtNcallback, discard, NULL);

	/* Add the switches. */
	if (XpmCreatePixmapFromData(display, XtWindow(container), off60_xpm,
			&off, &shapemask, &attributes) != XpmSuccess) {
		XtError("XpmCreatePixmapFromData failed");
	}
	if (XpmCreatePixmapFromData(display, XtWindow(container), on60_xpm,
			&on, &shapemask, &attributes) != XpmSuccess) {
		XtError("XpmCreatePixmapFromData failed");
	}
	sx = (w - 8*SWITCH_WIDTH - 7*BUTTON_GAP) / 2;
	for (i = 0; i < 8; i++) {
		if (i == 1 || i == 6)
			continue;
		toggles[i].on = (i < 7);
		toggles[i].w = XtVaCreateManagedWidget(
		    "switchcmd", commandWidgetClass, container,
		    XtNwidth, SWITCH_WIDTH,
		    XtNx, sx + i*(SWITCH_WIDTH + BUTTON_GAP),
		    XtNy, h - BUTTON_GAP - SWITCH_HEIGHT - 30,
		    XtNheight, SWITCH_HEIGHT,
		    XtNborderWidth, 0,
		    XtNlabel, "",
		    XtNbackgroundPixmap, toggles[i].on? on: off,
		    XtNhighlightThickness, 0,
		    NULL);
		XtAddCallback(toggles[i].w, XtNcallback, toggle_callback,
			&toggles[i]);
		ww = XtVaCreateManagedWidget(
		    "switch", labelWidgetClass, container,
		    XtNwidth, SWITCH_WIDTH,
		    XtNx, sx + i*(SWITCH_WIDTH + BUTTON_GAP),
		    XtNy, h - BUTTON_GAP - SWITCH_HEIGHT - 40,
		    XtNborderWidth, 0,
		    XtNlabel, top_label[i],
		    NULL);
		ww = XtVaCreateManagedWidget(
		    "switch", labelWidgetClass, container,
		    XtNwidth, SWITCH_WIDTH,
		    XtNx, sx + i*(SWITCH_WIDTH + BUTTON_GAP),
		    XtNy, h - BUTTON_GAP - 35,
		    XtNborderWidth, 0,
		    XtNlabel, bottom_label1[i],
		    NULL);
		if (bottom_label2[i] != NULL)
			ww = XtVaCreateManagedWidget(
			    "switch", labelWidgetClass, container,
			    XtNwidth, SWITCH_WIDTH,
			    XtNx, sx + i*(SWITCH_WIDTH + BUTTON_GAP),
			    XtNy, h - BUTTON_GAP - 25,
			    XtNborderWidth, 0,
			    XtNlabel, bottom_label2[i],
			    NULL);
		if (bottom_label3[i] != NULL)
			ww = XtVaCreateManagedWidget(
			    "switch", labelWidgetClass, container,
			    XtNwidth, SWITCH_WIDTH,
			    XtNx, sx + i*(SWITCH_WIDTH + BUTTON_GAP),
			    XtNy, h - BUTTON_GAP - 15,
			    XtNborderWidth, 0,
			    XtNlabel, bottom_label3[i],
			    NULL);
	}

	/* Create graphics contexts for drawing. */
	xgcv.foreground = appres.foreground;
	xgcv.background = appres.cardcolor;
	xgcv.font = ifontinfo->fid;
	gc = XtGetGC(toplevel, GCForeground|GCBackground|GCFont, &xgcv);
	xgcv.foreground = appres.cabinet;
	corner_gc = XtGetGC(toplevel, GCForeground|GCBackground, &xgcv);
	xgcv.foreground = appres.cardcolor;
	xgcv.background = appres.foreground;
	xgcv.font = ifontinfo->fid;
	invgc = XtGetGC(toplevel, GCForeground|GCBackground|GCFont, &xgcv);
	xgcv.tile = hole_pixmap;
	xgcv.fill_style = FillTiled;
	holegc = XtGetGC(toplevel, GCTile|GCFillStyle, &xgcv);

	/* Fix the size of the toplevel window. */
	XtVaSetValues(toplevel,
	    XtNwidth, w,
	    XtNheight, h,
	    XtNbaseWidth, w,
	    XtNbaseHeight, h,
	    XtNminWidth, w,
	    XtNminHeight, h,
	    XtNmaxWidth, w,
	    XtNmaxHeight, h,
	    NULL);

	/* Define event translations. */
	t = XtParseTranslationTable(translations);
	XtOverrideTranslations(container, t);
	t = XtParseTranslationTable("<Expose>: Redraw()");
	XtOverrideTranslations(cardw, t);

	/* Inflate it all. */
	XtRealizeWidget(toplevel);
	card_window = XtWindow(cardw);

	/* Add the cursor. */
#define C2H (BUTTON_HEIGHT + 2*BUTTON_GAP + 2*BUTTON_BW + 10)
	ww = XtVaCreateManagedWidget(
	    "cursor2", compositeWidgetClass, container,
	    XtNwidth, CELL_WIDTH,
	    XtNheight, C2H,
	    XtNx, CARD_AIR + LEFT_PAD + CELL_X((N_COLS / 2) + 2) + 4,
	    XtNy, h-C2H-2 - SWITCH_SKIP,
	    NULL);
	XtRealizeWidget(ww);
}

/* Punch a character into a particular column of the current card. */
static Boolean
punch_char(int cn, unsigned char c)
{
	int j;

	if (charset_xlate(ccharset, c) == NS) {
		/* Map lowercase, to be polite. */
		if (islower(c) && charset_xlate(ccharset, toupper(c)) != NS)
			c = toupper(c);
		else
			return False;
	}

	/* Space?  Do nothing. */
	if (!charset_xlate(ccharset, c))
		return True;

	ccard->holes[cn] |= charset_xlate(ccharset, c);

	/* Redundant? */
	for (j = 0; j < ccard->n_ov[cn]; j++)
		if (ccard->coltxt[cn][j] == c)
			return True;

	if (ccard->n_ov[cn] < N_OV) {
		ccard->coltxt[cn][ccard->n_ov[cn]] = c;
		++ccard->n_ov[cn];
	}

	return True;
}

/* Render the image of a card column onto the X display. */
static void
draw_col(int cn)
{
	int i;
	int j;
	int x = LEFT_PAD + CELL_X(cn);

#if defined(DEBUG) /*[*/
	printf(" draw_col(col %d)\n", cn);
#endif /*]*/

	/* Draw the text at the top, possibly overstruck. */
	for (j = 0; j < ccard->n_ov[cn]; j++) {
		if (ccard->coltxt[cn][j] < ' ')
			continue;
		XDrawString(display, card_window, gc,
		    x, TOP_PAD + TEXT_PAD, (char *)&ccard->coltxt[cn][j], 1);
	}

	/* Draw the holes, top to bottom. */
	for (i = 0; i < N_ROWS; i++) {
		if (ccard->holes[cn] & (0x800>>i)) {
			XGCValues xgcv;

			xgcv.ts_x_origin = x;
			xgcv.ts_y_origin = TOP_PAD + HOLE_PAD + (CELL_Y(i));
			XChangeGC(display, holegc,
					GCTileStipXOrigin|GCTileStipYOrigin,
					&xgcv);

			XFillRectangle(display, card_window, holegc,
			    x,
			    TOP_PAD + HOLE_PAD + (CELL_Y(i)),
			    hole_width, hole_height);
		}
	}
}

/* Update the column indicator in the lower right. */
static void
set_posw(int c)
{
	static char bb[3];

	col = c;
	if (col >= N_COLS)
		strcpy(bb, "--");
	else
		sprintf(bb, "%02d", col+1);
	XtVaSetValues(posw, XtNlabel, bb, NULL);
}

/* Go to the next card. */
static void
do_newcard(int replace)
{
	int i;

	if (mode != M_INTERACTIVE)
		replace = True;

	if (!ccard || !replace) {
		card *c;

		c = (card *)XtMalloc(sizeof(card));
		c->prev = ccard;
		c->next = NULL;
		if (ccard)
			ccard->next = c;
		ccard = c;
		c->seq = line_number;
		line_number += 10;
	} else if (mode != M_INTERACTIVE) {
		ccard->seq = line_number;
		line_number += 10;
	}
	(void) memset(ccard->coltxt, ' ', sizeof(ccard->coltxt));
	(void) memset(ccard->holes, 0, sizeof(ccard->holes));
	(void) memset(ccard->n_ov, 0, sizeof(ccard->n_ov));
	set_posw(0);
	if (appres.autonumber) {
		char ln_buf[9];

		(void) sprintf(ln_buf, "%08d", ccard->seq);
		for (i = 0; i < 8; i++)
			punch_char(72+i, ln_buf[i]);
	}
}

/* Redraw the entire card image. */
static void
redraw(Widget wid, XEvent  *event, String  *params, Cardinal *num_params)
{
	int i;
	Dimension x, y, w, h;

	if (event && event->type == Expose) {
		x = event->xexpose.x;
		y = event->xexpose.y;
		w = event->xexpose.width;
		h = event->xexpose.height;
#if defined(DEBUG) /*[*/
		printf("redraw x=%d y=%d w=%d h=%d\n",
				x, y, w, h);
#endif /*]*/
	} else {
		x = y = 0;
		w = card_width;
		h = card_height;
#if defined(DEBUG) /*[*/
		printf("redraw without Expose event\n");
#endif /*]*/
	}

	/* Slice off the padding. */
	if (x < LEFT_PAD) {			/* Left. */
		if (w <= LEFT_PAD - x) {
#if defined(DEBUG) /*[*/
			printf("ignoring left\n");
#endif /*]*/
			return;
		}
		w -= LEFT_PAD - x;
		x = 0;
	} else
		x -= LEFT_PAD;
	if (y < TOP_PAD) {			/* Top. */
		if (h <= TOP_PAD - y) {
#if defined(DEBUG) /*[*/
			printf("ignoring top\n");
#endif /*]*/
			return;
		}
		h -= TOP_PAD - y;
		y = 0;
	} else
		y -= TOP_PAD;
	if (x >= (CELL_X(N_COLS))) {		/* Right. */
#if defined(DEBUG) /*[*/
		printf("ignoring right\n");
#endif /*]*/
		return;
	}
	if (x + w > (CELL_X(N_COLS)))
		w = (CELL_X(N_COLS)) - x;
	if (y >= (CELL_Y(N_ROWS))) {		/* Bottom. */
#if defined(DEBUG) /*[*/
		printf("ignoring left\n");
#endif /*]*/
		return;
	}
	if (y + h > (CELL_Y(N_ROWS)))
		h = (CELL_Y(N_ROWS)) - y;
	
	for (i = COL_FROM_X(x);
	     i < COL_FROM_X(x + w + CELL_WIDTH) && i < N_COLS;
	     i++) {
		draw_col(i);
	}
}

/* Exit. */
static void
delete_window(Widget wid, XEvent *event, String *params, Cardinal *num_params)
{
	exit(0);
}

/* Find the first card in the deck. */
static card *
first_card(void)
{
	card *c;

	for (c = ccard; c && c->prev; c = c->prev) {
	}
	return c;
}

static void
save(int ignored)
{
	save_popup();
}

/*
 * Internals of functions that are enqueued with a delay.
 */

static void
do_nothing(int ignored)
{
}

static void
do_data(int c)
{
	if (col < N_COLS && punch_char(col, c)) {
		draw_col(col);
#if defined(SOUND) /*[*/
		loud_click();
#endif /*]*/
		do_right(0);
	}
}

static void
do_multipunch(int c)
{
	if (col < N_COLS && punch_char(col, c)) {
		draw_col(col);
#if defined(SOUND) /*[*/
		loud_click();
#endif /*]*/
	}
}

static void
do_left(int c)
{
	if (col) {
		do_pan_left(0);
		set_posw(col - 1);
	}
}

static void
do_right(int do_click)
{
	if (col < N_COLS) {
		do_pan_right(do_click);
		set_posw(col + 1);
	}
}

static void
do_pan_left(int ignored)
{
	scrollw_column--;
	XtVaSetValues(scrollw, XtNx, SCROLLW_X(), NULL);
#if defined(SOUND) /*[*/
	soft_click();
#endif /*]*/
}

static void
do_pan_right(int do_click)
{
	scrollw_column++;
	XtVaSetValues(scrollw, XtNx, SCROLLW_X(), NULL);
#if defined(SOUND) /*[*/
	if (do_click)
		soft_click();
#endif /*]*/
}

static void
do_pan_up(int ignored)
{
	Dimension y;

	XtVaGetValues(scrollw, XtNy, &y, NULL);
	y += CELL_HEIGHT;
	XtVaSetValues(scrollw, XtNy, y, NULL);
}

static void
do_home(int ignored)
{
	do_pan_left(0);
	set_posw(col - 1);
}

static void
do_slam(int ignored)
{
	scrollw_column = 40;
	XtVaSetValues(scrollw,
	    XtNx, SCROLLW_X(),
	    XtNy, -(2*card_height) + TOP_PAD,
	    NULL);
}

static void
do_quit(int ignored)
{
    	exit(0);
}

static void
do_invisible(int ignored)
{
	card_visible = False;
}

static void
do_visible(int ignored)
{
	card_visible = True;
}

/*
 * Event queueing: Inserting artificial delays in processing certain events.
 */
enum evtype { DUMMY, DATA, MULTI, LEFT, RIGHT, HOME,
	      PAN_RIGHT, PAN_LEFT, PAN_UP, SLAM, NEWCARD, QUIT,
	      INVISIBLE, VISIBLE };
void (*eq_fn[])(int) = {
	do_nothing,
	do_data,
	do_multipunch,
	do_left,
	do_right,
	do_home,
	do_pan_right,
	do_pan_left,
	do_pan_up,
	do_slam,
	do_newcard,
	do_quit,
	do_invisible,
	do_visible
};
char *eq_name[] = {
	"DUMMY", "DATA", "MULTI", "LEFT", "RIGHT", "HOME",
	"PAN_RIGHT", "PAN_LEFT", "PAN_UP", "SLAM", "NEWCARD", "QUIT",
	"INVISIBLE", "VISIBLE"
};
typedef struct event {
	struct event *next;
	enum evtype evtype;
	unsigned char param;	/* optional */
	int delay;
} event_t;
event_t *eq_first, *eq_last;
int eq_count;

static void
dump_queue(const char *when)
{
	event_t *e;

	if (!appres.debug)
		return;

	printf("queue(%s):", when);

	for (e = eq_first; e != NULL; e = e->next) {
		printf(" %s(%d)%s",  eq_name[e->evtype], e->delay,
			e == eq_last? "*": "");
	}
	printf("\n");
}

/* Run the event at the front of the queue. */
static void
deq_event(XtPointer data, XtIntervalId *id)
{
	event_t *e;

	if (!eq_count)
		return;

	/* Dequeue it. */
	e = eq_first;
	eq_first = e->next;
	if (eq_first == NULL)
		eq_last = NULL;
	dump_queue("deq");

	/* Run it. */
	if (appres.debug)
		printf("run %s(%d)\n", eq_name[e->evtype], e->delay);
	(*eq_fn[e->evtype])(e->param);

	/* Free it. */
	XtFree((XtPointer)e);
	e = NULL;

	/*
	 * If there are more events, schedule the next one.
	 * Otherwise, see if the batch FSM needs cranking.
	 */
	if (--eq_count) {
		if (appres.debug)
			printf("TimeOut(%d)\n", eq_first->delay);
		(void) XtAppAddTimeOut(appcontext, eq_first->delay, deq_event,
			NULL);
	} else if (mode != M_INTERACTIVE)
		batch_fsm();
}

/* Add an event to the back of the queue. */
static Boolean
enq_event(enum evtype evtype, unsigned char param, Boolean restricted, int delay)
{
	event_t *e;

	if (restricted && eq_count) {
		XBell(display, 0);
		return False;
	}
	if (evtype == DUMMY && delay == 0)
		return True;
	e = (event_t *)XtMalloc(sizeof(*e));
	e->evtype = evtype;
	e->param = param;
	e->delay = delay;

	e->next = NULL;
	if (eq_first == NULL)
		eq_first = e;
	if (eq_last != NULL)
		eq_last->next = e;
	eq_last = e;
	dump_queue("enq");

	if (!eq_count++) {
		if (appres.debug)
			printf("TimeOut(%d)\n", delay);
		(void) XtAppAddTimeOut(appcontext, delay, deq_event, NULL);
	}
	return True;
}

static Boolean
add_char(char c)
{
	return enq_event(DATA, c, False, SLOW);
}

static void
enq_quit(void)
{
	(void) enq_event(QUIT, 0, False, SLOW * 2);
}

static void
enq_delay(void)
{
	(void) enq_event(DUMMY, 0, False, VERY_SLOW);
}

/*
 * Externals of delayed functions, called by the toolkit.
 */

static void
data(Widget wid, XEvent *event, String *params, Cardinal *num_params)
{
	XKeyEvent *kevent = (XKeyEvent *)event;
	char buf[10];
	KeySym ks;
	int ll;

	ll = XLookupString(kevent, buf, 10, &ks, (XComposeStatus *)NULL);
	if (ll == 1) {
		enq_event(DATA, buf[0] & 0xff, !appres.typeahead, SLOW);
	}
}

static void
multi_punch_data(Widget wid, XEvent *event, String *params, Cardinal *num_params)
{
	XKeyEvent *kevent = (XKeyEvent *)event;
	char buf[10];
	KeySym ks;
	int ll;

	ll = XLookupString(kevent, buf, 10, &ks, (XComposeStatus *)NULL);
	if (ll == 1) {
		enq_event(MULTI, buf[0], !appres.typeahead, SLOW);
	}
}

static void
left(Widget wid, XEvent *event, String *params, Cardinal *num_params)
{
	enq_event(LEFT, 0, !appres.typeahead, SLOW);
}

static void
right(Widget wid, XEvent *event, String *params, Cardinal *num_params)
{
	enq_event(RIGHT, 1, !appres.typeahead, SLOW);
}

static void
home(Widget wid, XEvent *event, String *params, Cardinal *num_params)
{
	int i;

	if (!enq_event(DUMMY, 0, True, SLOW))
		return;

	for (i = 0; i < col; i++)
		enq_event(HOME, 0, False, FAST);
}

/*
 * Scroll the current card away, and get a new one.
 */
static void
next(Widget wid, XEvent *event, String *params, Cardinal *num_params)
{
	if (ccard->next) {
		ccard = ccard->next;
		set_posw(0);
	} else {
		card_complete(FAST);
		get_new_card();
	}
}

static void
tab(Widget wid, XEvent *event, String *params, Cardinal *num_params)
{
	int i;

	if (!enq_event(DUMMY, 0, True, SLOW))
		return;

	for (i = col; i < 6; i++)
		enq_event(RIGHT, 1, False, SLOW);
}

/* Throw away this card. */
static void
discard(Widget w, XtPointer client_data, XtPointer call_data)
{
	int i;

	if (!enq_event(DUMMY, 0, True, SLOW))
		return;

	/* Scroll the previous card away. */
	if (ccard) {
		for (i = 0; i <= col; i++)
			enq_event(LEFT, 0, False, FAST);
		for (i = 0; i < N_COLS/2; i++)
			enq_event(PAN_LEFT, 0, False, FAST);
	}

	enq_event(NEWCARD, True, False, FAST);

	/* Scroll the new card down. */
	enq_event(SLAM, 0, False, SLOW);
	for (i = 0; i <= N_ROWS; i++)
		enq_event(PAN_UP, 0, False, SLOW);

}

/* Scroll the current card off to the left. */
static void
card_complete(int delay)
{
	int i;

	for (i = col; i < N_COLS; i++)
		enq_event(RIGHT, 0, False, delay);
	for (i = 0; i < N_COLS/2 + 10; i++)
		enq_event(PAN_RIGHT, 0, False, delay);
	enq_event(INVISIBLE, 0, False, 0);
}

/* Generate a fresh card. */
static void
get_new_card(void)
{
	int i;

	enq_event(NEWCARD, False, False, FAST);

	/* Scroll the new card down. */
	enq_event(SLAM, 0, False, SLOW);
	for (i = 0; i <= N_ROWS; i++)
		enq_event(PAN_UP, 0, False, SLOW);
	enq_event(VISIBLE, 0, False, 0);
}

#define NP	5
Atom paste_atom[NP];
int n_pasting = 0;
int pix = 0;
Time paste_time;

/* Pasting support. */
static void
paste_callback(Widget w, XtPointer client_data, Atom *selection, Atom *type,
    XtPointer value, unsigned long *length, int *format)
{
	char *s;
	unsigned long len;

	if ((value == NULL) || (*length == 0)) {
		XtFree(value);

		/* Try the next one. */
		if (n_pasting > pix)
			XtGetSelectionValue(w, paste_atom[pix++], XA_STRING,
			    paste_callback, NULL, paste_time);
		return;
	}

	s = (char *)value;
	len = *length;
	while (len--) {
		unsigned char c = *s++;

		if (c == '\n')
			break;
		if (c < ' ') {
			continue;
		}
		if (!enq_event(DATA, c, False, SLOW))
			break;
	}
	n_pasting = 0;

	XtFree(value);
}

static void
insert_selection(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
	int	i;
	Atom	a;
	XButtonEvent *bevent = (XButtonEvent *)event;

	n_pasting = 0;
	for (i = 0; i < *num_params; i++) {
		a = XInternAtom(display, params[i], True);
		if (a == None) {
			XtWarning("no atom for selection");
			continue;
		}
		if (n_pasting < NP)
			paste_atom[n_pasting++] = a;
	}
	pix = 0;
	if (n_pasting > pix) {
		paste_time = bevent->time;
		XtGetSelectionValue(w, paste_atom[pix++], XA_STRING,
		    paste_callback, NULL, paste_time);
	}
}

/* 'Save' pop-up. */

static Boolean save_popup_created = False;
static Widget save_shell, save_dialog;

static void
center_it(Widget w, XtPointer client_data, XtPointer call_data)
{
	Dimension width, height;
	Position toplevel_x, toplevel_y;
	Dimension toplevel_width, toplevel_height;

	/* Get the popup's dimensions */
	XtVaGetValues(w,
	    XtNwidth, &width,
	    XtNheight, &height,
	NULL);

	/* Get the toplevel position and dimensions. */
	XtVaGetValues(toplevel,
	    XtNwidth, &toplevel_width,
	    XtNheight, &toplevel_height,
	    XtNx, &toplevel_x,
	    XtNy, &toplevel_y,
	    NULL);

	/* Compute our position relatve to those. */
	XtVaSetValues(w,
	    XtNx, toplevel_x + (toplevel_width - width) / 2,
	    XtNy, toplevel_y + (toplevel_height - height) / 2,
	    NULL);
}

static void
save_file_ascii(void)
{
	char *sfn;
	FILE *f;
	card *c;
	int i, j, h;

	XtVaGetValues(save_dialog, XtNvalue, &sfn, NULL);
	f = fopen(sfn, "w");
	if (f == NULL) {
	    XBell(display, 100);
	    XtVaSetValues(save_dialog,
		XtNlabel, strerror(errno),
		NULL);
	    XtVaSetValues(XtNameToWidget(save_dialog, XtNlabel),
		XtNforeground, appres.errcolor,
		NULL);
	    return;
	}
	XtPopdown(save_shell);

	for (c = first_card(); c; c = c->next) {
		if (c == ccard)
			continue;
		for (i = 0; i < N_COLS; i++) {
			if (!c->n_ov[i]) {
				fputc(' ', f);
				continue;
			}
			/* Try to find a single character. */
			for (h = 0; h < 256; h++) {
				if (h == '\n' || h == '\b')
					continue;
				if (charset_xlate(ccharset, h) != NS &&
				    charset_xlate(ccharset, h) == c->holes[i]) {
					fputc(h, f);
					break;
				}
			}
			if (h < 256)
				continue;
			/* Write it with backspaces. */
			for (j = 0; j < c->n_ov[i]; j++) {
				if (j)
					fputc('\b', f);
				fputc(c->coltxt[i][j], f);
			}
		}
		fputc('\n', f);
	}
	fclose(f);
}

static void
save_file_image(void)
{
	char *sfn;
	FILE *f;
	card *c;

	XtVaGetValues(save_dialog, XtNvalue, &sfn, NULL);
	f = fopen(sfn, "w");
	if (f == NULL) {
	    XBell(display, 100);
	    XtVaSetValues(save_dialog,
		XtNlabel, strerror(errno),
		NULL);
	    XtVaSetValues(XtNameToWidget(save_dialog, XtNlabel),
		XtNforeground, appres.errcolor,
		NULL);
	    return;
	}
	XtPopdown(save_shell);

	/* Write the header. */
	fprintf(f, "H80");

	/* Write the cards. */
	for (c = first_card(); c; c = c->next) {
		int i;
		unsigned char b3[3];

		if (c == ccard)
			continue;

		fprintf(f, "%c%c%c",
		    0x80 | ccard_type->card_type[0],
		    0x80 | ccard_type->card_type[1] |
			charset_punch_type(ccharset),
		    0x80 | ccard_type->card_type[2]);
		for (i = 0; i < N_COLS; i++) {
			if (i % 2) {
				b3[1] |= (c->holes[i] >> 8) & 0xf;
				b3[2] = c->holes[i] & 0xff;
				if (fwrite(b3, 1, 3, f) < 3)
					break;
			} else {
				b3[0] = c->holes[i] >> 4;
				b3[1] = (c->holes[i] & 0xf) << 4;
			}
		}
	}
	fclose(f);
}

static void
save_ascii_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
	save_file_ascii();
}

static void
save_image_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
	save_file_image();
}

static void
cancel_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
	XtPopdown(save_shell);
}

static void
confirm(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
	save_file_ascii();
}

static void
save_popup(void)
{
	Widget w;

	if (!save_popup_created) {
		Dimension width, height;

		/* Create the shell. */
		save_shell = XtVaCreatePopupShell("save",
		    transientShellWidgetClass, toplevel,
		    NULL);
		XtAddCallback(save_shell, XtNpopupCallback, center_it, NULL);

		/* Create the dialog in the popup. */
		save_dialog = XtVaCreateManagedWidget(
		    "dialog", dialogWidgetClass,
		    save_shell,
		    XtNvalue, "",
		    XtNbackground, appres.cabinet,
		    NULL);
		w = XtNameToWidget(save_dialog, XtNvalue);
		XtVaSetValues(w,
		    XtNwidth, 200,
		    NULL);
		XtOverrideTranslations(w,
		    XtParseTranslationTable("<Key>Return: confirm()"));
		XtVaSetValues(XtNameToWidget(save_dialog, XtNlabel),
		    XtNwidth, 200,
		    XtNbackground, appres.cabinet,
		    NULL);
		if (appres.cabinet == XBlackPixel(display, default_screen))
			XtVaSetValues(XtNameToWidget(save_dialog, XtNlabel),
			    XtNforeground, XWhitePixel(display, default_screen),
			    NULL);

		/* Add 'Save' and 'Cancel' buttons to the dialog. */
		w = XtVaCreateManagedWidget("Save Ascii", commandWidgetClass,
		    save_dialog,
		    NULL);
		XtAddCallback(w, XtNcallback, save_ascii_callback, NULL);
		w = XtVaCreateManagedWidget("Save Images", commandWidgetClass,
		    save_dialog,
		    NULL);
		XtAddCallback(w, XtNcallback, save_image_callback, NULL);
		w = XtVaCreateManagedWidget("Cancel", commandWidgetClass,
		    save_dialog,
		    NULL);
		XtAddCallback(w, XtNcallback, cancel_callback, NULL);

		/* Set the width and height. */
		XtRealizeWidget(save_shell);
		XtVaGetValues(save_shell,
		    XtNwidth, &width,
		    XtNheight, &height,
		NULL);
		XtVaSetValues(save_shell,
		    XtNheight, height,
		    XtNwidth, width,
		    XtNbaseHeight, height,
		    XtNbaseWidth, width,
		    XtNminHeight, height,
		    XtNminWidth, width,
		    XtNmaxHeight, height,
		    XtNmaxWidth, width,
		    NULL);

		save_popup_created = True;
	}
	XtVaSetValues(save_dialog,
	    XtNlabel, "Save File Name",
	    NULL);
	if (appres.cabinet == XBlackPixel(display, default_screen))
		XtVaSetValues(XtNameToWidget(save_dialog, XtNlabel),
		    XtNforeground, XWhitePixel(display, default_screen),
		    NULL);
	else
		XtVaSetValues(XtNameToWidget(save_dialog, XtNlabel),
		    XtNforeground, appres.foreground,
		    NULL);
	XtPopup(save_shell, XtGrabExclusive);
	/*XSetWMProtocols(display, XtWindow(save_shell), &delete_me, 1);*/
}

/* Batch processing. */
typedef enum {
	BS_READ,	/* need to read from the file */
	BS_CHAR,	/* need to process a character from the file */
	BS_SPACE,	/* need to space over the rest of the card */
	BS_EOF		/* done */
} batch_state_t;
static const char *bs_name[] = { "READ", "CHAR", "SPACE", "EOF" };
static batch_state_t bs = BS_READ;
XtInputId read_id = 0;

/* Input is now readable. */
static void
read_more(XtPointer closure, int *fd, XtInputId *id)
{
	XtRemoveInput(read_id);
	read_id = 0;
	batch_fsm();
}

/*
 * Crank the batch FSM.
 */
static void
batch_fsm(void)
{
	static char buf[1024];
	ssize_t n2r, nr;
	static ssize_t rbsize = 0;
	static char *s = buf + sizeof(buf);
	static Boolean any = False;
	char c;

	do {
		if (appres.debug)
			printf("batch_fsm: %s\n", bs_name[bs]);

		switch (bs) {

		case BS_READ:
			/* Keep munching on the same buffer. */
			if (s < buf + rbsize) {
				if (appres.debug)
				    	printf(" continuing, %zd more\n",
						buf + rbsize - s);
			} else {

				/* Read the next card. */
				nr = read(batchfd, buf, sizeof(buf));
				if (appres.debug)
					printf(" got %zd chars\n", nr);
				if (nr == 0) {
					if (read_id != 0) {
						XtRemoveInput(read_id);
						read_id = 0;
					}
					close(batchfd);
					batchfd = -1;
					enq_quit();

					/* Next, exit. */
					bs = BS_EOF;
					break;
				}
				if (nr < 0) {
					if (errno == EWOULDBLOCK) {
						if (read_id == 0)
							read_id = XtAppAddInput(
								appcontext,
								batchfd,
								(XtPointer)
								 XtInputReadMask,
								read_more, NULL);
						return;
					} else {
						perror("read(stdin)");
						exit(1);
					}
				}
				rbsize = nr;
				s = buf;
			}

			/* Next, start munching on it. */
			any = True;
			bs = BS_CHAR;
			break;

		case BS_CHAR:
			if (!card_visible) {
				get_new_card();
				if (mode == M_BATCH) {
					enq_delay();
				}
				break;
			}
			c = *s++;
			if (appres.debug)
				printf(" c = 0x%02x, col = %d\n",
					c & 0xff, col);
			if (c == '\n') {
				/*
				 * End of input line.
				 *
				 * Delay if there is anything to see; start
				 * spacing to the end of the card.
				 */
				if (col)
					enq_delay();
				bs = BS_SPACE;
				break;
			}
			add_char(c);
			if (s >= buf + rbsize) {
				/*
				 * Ran out of buffer without a newline.
				 *
				 * Go read some more.
				 */
				bs = BS_READ;
				continue;
			}
			if (col >= (appres.autonumber? (N_COLS - 1 - 8):
						       (N_COLS - 1))) {
				/*
				 * Overflowed the card.
				 *
				 * Eat the character.
				 */
				continue;
			}
			break;

		case BS_SPACE:
			if (appres.debug)
				printf(" col = %d\n", col);

			if (col >= N_COLS - 1) {
				/* End of card.  Scroll it away. */
				card_complete(SLOW);

				/*
				 * In remote control mode, create a new card.
				 * In batch mode, wait for data before
				 * doing it.
				 */
				if (mode == M_REMOTECTL) {
					get_new_card();
				}

				bs = BS_READ;
				break;
			}

			/* Add a space. */
			add_char(' ');
			break;

		case BS_EOF:
			/* Done. */
			exit(0);
			break;
		}
	} while (eq_first == NULL);

}
