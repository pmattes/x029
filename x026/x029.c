/*
 * Copyright 1993, 1995, 2002, 2003, 2013 Paul Mattes.  All Rights Reserved.
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted,
 *   provided that the above copyright notice appear in all copies and that
 *   both that copyright notice and this permission notice appear in
 *   supporting documentation.
 *
 * x029 -- A Keypunch Simluator
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdarg.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Core.h>
#include <X11/Shell.h>
#include <X11/Xatom.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Porthole.h>
#include <X11/xpm.h>

#include "jones.h"		/* file format definitions */
#include "charset.h"		/* character set module */
#include "cardimg.h"		/* card image module */
#include "x029.h"		/* global definitions for x029 */
#include "paste.h"		/* paste module */
#include "save.h"		/* save module */

#include "hole.xpm"		/* hole image */
#include "flipper_off.xpm"	/* power switch, off */
#include "flipper_on.xpm"	/* power switch, on */
#include "off60.xpm"		/* toggle switch, off */
#include "on60.xpm"		/* toggle switch, on */
#include "feed.xpm"		/* FEED key */
#include "feed_pressed.xpm"	/* FEED key */
#include "rel.xpm"		/* REL key */
#include "rel_pressed.xpm"	/* REL key */
#include "save.xpm"		/* SAVE button */
#include "save_pressed.xpm"	/* SAVE button */
#include "drop.xpm"		/* DROP button */
#include "drop_pressed.xpm"	/* DROP button */
#include "ci2.xpm"		/* column indicator */
#include "arrow.xpm"		/* arrow */
#include "x029.bm"		/* icon */

enum {
    T_AUTO_SKIP_DUP,
    T_UNUSED_1,
    T_PROG_SEL,
    T_AUTO_FEED,
    T_PRINT,
    T_LZ_PRINT,
    T_UNUSED_2,
    T_CLEAR
} toggle_ix;

char *top_label[] = { "ON", NULL, "ONE", "ON", "ON", "ON", NULL, "ON" };
char *bottom_label1[] = { "AUTO", NULL, "TWO", "AUTO", "PRINT", "LZ", NULL, "CLEAR" };
char *bottom_label2[] = { "SKIP", NULL, "PROG", "FEED", NULL, "PRINT", NULL, NULL };
char *bottom_label3[] = { "DUP", NULL, "SEL", NULL, NULL, NULL, NULL, NULL };

#define VERY_SLOW 500
#define SLOW	75
#define FAST	25
#define VERY_FAST 15

#define SLAM_COL	15
#define SLAM_TARGET_COL	40

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
#define BUTTON_WIDTH	45
#define	BUTTON_HEIGHT	20

#define POSW_INNER_HEIGHT	29
#define POSW_TFRAME	8
#define POSW_FRAME	4
#define POSW_HEIGHT	(POSW_TFRAME + POSW_INNER_HEIGHT + POSW_FRAME)
#define POSW_INNER_WIDTH	(KEY_WIDTH * 3)
#define POSW_WIDTH	(POSW_FRAME + POSW_INNER_WIDTH + POSW_FRAME)
#define ARROW_WIDTH	19

#define STACKER_WIDTH	43

#define KEY_WIDTH	40
#define KEY_HEIGHT	40

#define POWER_GAP	10
#define POWER_WIDTH	30
#define POWER_HEIGHT	40

Widget			toplevel;
Display			*display;
int			default_screen;
charset_t		ccharset = NULL;
cardimg_t		ccardimg = NULL;

static char		*programname;
static XtAppContext	appcontext;
static int		root_window;
static int		card_window;
static int		depth;
static XFontStruct	*ifontinfo;
Atom			a_delete_me;
static int		line_number = 100;
static int		card_count = 0;
static Pixmap		hole_pixmap;

static Pixmap		flipper_off, flipper_on;
static Widget		power_widget;
static Widget		stacker;

int			demofd = -1;

/* Mode: interactive versus demo. */
typedef enum {
    M_INTERACTIVE,		/* interactive */
    M_BATCH,			/* read from a fixed file */
    M_REMOTECTL			/* read from stdin incrementally */
} imode_t;
imode_t mode = M_INTERACTIVE;

/* 029 simulated key structure. */
typedef struct _key {
    const char *name;		/* descriptive name for debug */
    Widget widget;		/* Xt widget */
    Pixmap normal_pixmap;	/* normal (unpressed) pixmap */
    Pixmap pressed_pixmap;	/* pressed pixmap */
    XtIntervalId timeout_id;	/* un-press timeout ID, or 0 if none pending */
    void (*backend)(struct _key *); /* backend */
} kpkey_t;
kpkey_t rel_key;		/* REL key */
kpkey_t feed_key;		/* FEED key */
kpkey_t save_key;		/* SAVE key */
kpkey_t drop_key;		/* DROP key */

typedef void (*key_backend_t)(kpkey_t *);
static void key_init(kpkey_t *key, const char *name, Widget container, int x,
	int y, char *normal_pixmap_src[], char *pressed_pixmap_src[],
	key_backend_t backend);

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
    char	*demofile;
    Boolean	autonumber;
    Boolean	typeahead;
    Boolean 	remotectl;
    Boolean	help;
    Boolean 	debug;
} AppRes, *AppResptr;

static AppRes appres;

/* Command-line options. */
static XrmOptionDescRec options[]= {
    { "-ifont",	".ifont",	XrmoptionSepArg,	NULL },
    { "-nonumber",	".autoNumber",  XrmoptionNoArg,		"False" },
    { "-number",	".autoNumber",  XrmoptionNoArg,		"True" },
    { "-typeahead",	".typeahead",	XrmoptionNoArg,		"True" },
    { "-mono",	".cabinet",	XrmoptionNoArg,		"black" },
    { "-charset",	".charset",	XrmoptionSepArg,	NULL },
    { "-card",	".card",	XrmoptionSepArg,	NULL },
    { "-demo",	".demoFile",	XrmoptionSepArg,	NULL },
    { "-remotectl",	".remoteCtl",	XrmoptionNoArg,		"True" },
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
      offset(autonumber), XtRString, "False" },
    { "typeahead", "Typeahead", XtRBoolean, sizeof(Boolean),
      offset(typeahead), XtRString, "True" },
    { "charset", "Charset", XtRString, sizeof(String),
      offset(charset), XtRString, NULL },
    { "card", "Card", XtRString, sizeof(String),
      offset(card), XtRString, NULL },
    { "demoFile", "DemoFile", XtRString, sizeof(String),
      offset(demofile), XtRString, NULL },
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
    "*ifont:		7x13",
    "*stackerDepression.background:	grey57",
    "*depression.background:	grey57",
    "*stacker.font:	6x13bold",
    "*stacker.foreground:	black",
    "*stacker.background:	grey92",
    "*dialog*value*font: fixed",
    "*powerDepression.background:	grey57",
    "*switch.font:  	6x10",
    "*switch.background:  grey92",
    "*font:		variable",
    "*cabinet:		grey92",
    "*cardColor:	ivory",
    NULL
};

/* Xt actions (xxx_action). */
static void Data_action(Widget, XEvent *, String *, Cardinal *);
static void MultiPunchData_action(Widget, XEvent *, String *, Cardinal *);
static void DeleteWindow_action(Widget, XEvent *, String *, Cardinal *);
static void Home_action(Widget, XEvent *, String *, Cardinal *);
static void Left_action(Widget, XEvent *, String *, Cardinal *);
static void Release_action(Widget, XEvent *, String *, Cardinal *);
static void Redraw_action(Widget, XEvent *, String *, Cardinal *);
static void Right_action(Widget, XEvent *, String *, Cardinal *);
static void Tab_action(Widget, XEvent *, String *, Cardinal *);

/* Xt callbacks. */
static void key_press(Widget, XtPointer, XtPointer);

/* Actions. */
static XtActionsRec actions[] = {
    { "Data",		Data_action },
    { "MultiPunchData", MultiPunchData_action },
    { "DeleteWindow",	DeleteWindow_action },
    { "Home",		Home_action },
    { "Left",		Left_action },
    { "Release",	Release_action },
    { "Redraw",		Redraw_action },
    { "Right",		Right_action },
    { "Tab",		Tab_action },
    { "InsertSelection", InsertSelection_action },
    { "Confirm",	Confirm_action }
};
static int actioncount = XtNumber(actions);

static Boolean power_on = False;
static enum {
    C_EMPTY,		/* No card in punch station, no operation pending */
    C_FLUX,		/* Operation in progress (in or out) */
    C_REGISTERED	/* Card registered in punch station */
} card_state = C_EMPTY;
#define CARD_REGISTERED	(card_state == C_REGISTERED)

/* Key press back-ends (xxx_backend). */
static void save_key_backend(kpkey_t *key);
static void drop_key_backend(kpkey_t *key);
static void rel_key_backend(kpkey_t *);
static void feed_key_backend(kpkey_t *);

/* Queued actions (queued_xxx). */
static void queued_nothing(int);
static void queued_data(int);
static void queued_multipunch(int);
static void queued_left(int);
static void queued_kybd_right(int);
static void queued_rel_right(int);
static void queued_home(int);
static void queued_pan_right(int);
static void queued_pan_left(int);
static void queued_pan_up(int);
static void queued_slam(int);
static void queued_newcard(int);
static void queued_power_on(int);
static void queued_press_feed(int);
static void queued_press_rel(int);
static void queued_empty(int);
static void queued_stack(int);

/* Other forward references. */
static void define_widgets(void);
static void startup_power_feed(void);
static void startup_power(void);
static void do_feed(Boolean keep_sequence);
static void enq_delay(void);
static void do_release(int delay);
static void demo_fsm(void);
static void show_key_down(kpkey_t *key);
static void display_card_count(void);
static void flush_typeahead(void);

static void dbg_printf(const char *format, ...);

/* Syntax. */
void
usage(void)
{
    charset_t cs = NULL;
    cardimg_t ci = NULL;

    fprintf(stderr, "Usage: %s [x029-options] [Xt-options]\n", programname);
    fprintf(stderr, "x029-options:\n\
  -ifont <font>    Interpreter (card edge) font, defaults to 7x13\n\
  -number          Automatically number cards in cols 73..80\n\
  -charset <name>  Keypunch character set:\n");
    for (cs = next_charset(NULL); cs != NULL; cs = next_charset(cs)) {
	fprintf(stderr, "    %-9s %s%s\n",
		charset_name(cs), charset_desc(cs),
		(cs == default_charset())? " (default)": "");
    }
    fprintf(stderr, "\
  -card <name>     Card image:\n");
    for (ci = next_cardimg(NULL); ci != NULL; ci = next_cardimg(ci)) {
	fprintf(stderr, "    %-9s %s%s\n", cardimg_name(ci), cardimg_desc(ci),
		(ci == default_cardimg())? " (default)": "");
    }
    fprintf(stderr, "\
  -026ftn          Alias for '-charset bcd-h'\n\
  -026comm         Alias for '-charset bcd-a'\n\
  -029             Alias for '-charset 029'\n\
  -EBCDIC          Alias for '-charset ebcdic'\n\
  -demo <file>     Read text file and punch it (automated display)\n\
  -demo -          Read stdin and punch it\n\
  -remotectl       Read stdin incrementally\n\
  -help            Display this text\n");
    exit(1);
}

int
main(int argc, char *argv[])
{
    XtTranslations table;
    Pixmap icon;

    /* Figure out who we are */
    programname = strrchr(argv[0], '/');
    if (programname)
	++programname;
    else
	programname = argv[0];

    /* Initialze Xt and fetch our resources. */
    toplevel = XtVaAppInitialize(
	&appcontext,
	"X029",
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
    if (appres.charset != NULL) {
	ccharset = find_charset(appres.charset);
	if (ccharset == NULL) {
	    ccharset = default_charset();
	    fprintf(stderr, "No such charset: '%s', defaulting to '%s'\n"
			    "Use '-help' to list the available character "
			    "sets\n",
			    appres.charset, charset_name(ccharset));
	    }
    } else {
	ccharset = default_charset();
    }

    if (appres.demofile != NULL) {
	if (appres.remotectl) {
	    fprintf(stderr, "Demofile and remotectl in conflict, "
			    "ignoring remotectl\n");
	}
	if (strcmp(appres.demofile, "-")) {
	    demofd = open(appres.demofile, O_RDONLY | O_NONBLOCK);
	    if (demofd < 0) {
		perror(appres.demofile);
		exit(1);
	    }
	} else {
	    demofd = fileno(stdin);
    }
	mode = M_BATCH;
    } else if (appres.remotectl) {
	mode = M_REMOTECTL;
	demofd = fileno(stdin);
    } else {
	mode = M_INTERACTIVE;
    }

    /* Define the widgets. */
    define_widgets();

    /* Set up a cute (?) icon. */
    icon = XCreateBitmapFromData(display, XtWindow(toplevel),
	(char *)x029_bits, x029_width, x029_height);
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

    if (mode != M_INTERACTIVE && demofd == fileno(stdin)) {
	if (fcntl(demofd, F_SETFL, fcntl(demofd, F_GETFL) | O_NONBLOCK) < 0) {
	    perror("fcntl");
	    exit(1);
	}
    }

    if (mode == M_INTERACTIVE || mode == M_REMOTECTL) {
	startup_power_feed();
    } else {
	startup_power();
    }

    /* Process X events forever. */
    XtAppMainLoop(appcontext);

    return 0;
}

static void
power_off_timeout(XtPointer data, XtIntervalId *id)
{
    exit(0);
}

static void
do_power_off(void)
{
    power_on = False;
    XtVaSetValues(power_widget, XtNbackgroundPixmap, flipper_off, NULL);
    (void) XtAppAddTimeOut(appcontext, VERY_SLOW * 2, power_off_timeout, NULL);
}

/* Callback for power button. */
static void
power_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    do_power_off();
}

/* Definitions for the toggle switches. */

static Pixmap toggle_on, toggle_off;

struct toggle {
    Widget w;
    int on;
};
struct toggle toggles[8];

/* Timeout function for the clear switch. */
static void
unclear_event(XtPointer data, XtIntervalId *id)
{
    toggles[T_CLEAR].on = 0;
    XtVaSetValues(toggles[T_CLEAR].w, XtNbackgroundPixmap, toggle_off, NULL);
}

/* Callback function for toggle switches. */
static void
toggle_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    struct toggle *t = (struct toggle *)client_data;

    if (t != &toggles[T_CLEAR]) {
	t->on = !t->on;
	XtVaSetValues(w, XtNbackgroundPixmap, t->on? toggle_on: toggle_off,
		NULL);
	return;
    }

    /* It's the clear switch. */
    if (t->on)
	return;
    t->on = !t->on;
    XtVaSetValues(w, XtNbackgroundPixmap, t->on? toggle_on: toggle_off, NULL);
    (void) XtAppAddTimeOut(appcontext, SLOW * 6, unclear_event, NULL);
    if (CARD_REGISTERED) {
	do_release(VERY_FAST);
    }
}

/* Card-image data structures. */
static card_t *ccard;
static int col = 0;
static GC gc, invgc, holegc;

static Widget container, porth, scrollw, cardw, posw_porth, posw;
static int scrollw_column;
#define SCROLLW_X()	(-(LEFT_PAD+CELL_X(scrollw_column)))

static Dimension card_width, card_height;
static Dimension hole_width, hole_height;

/* Set up the keypunch. */
static void
define_widgets(void)
{
    Dimension w, h;
    Dimension posw_x;
    XtTranslations t;
    XGCValues xgcv;
    int i;
    Pixmap pixmap, shapemask;
    Pixmap hole_shapemask;
    Pixmap column_indicator;
    Pixmap arrow;
    Dimension sx;
    XpmAttributes attributes;
    static char translations[] = "\
	<Key>Left:	Left()\n\
	<Key>BackSpace:	Left()\n\
	<Key>Right:	Right()\n\
	<Key>Home:	Home()\n\
	<Key>Return:	Release()\n\
	<Key>KP_Enter:	Home()\n\
	<Key>Down:	Release()\n\
	<Key>Tab:	Tab()\n\
	<Btn2Down>:	InsertSelection(PRIMARY)\n\
	Alt<Key>:	MultiPunchData()\n\
	Meta<Key>:	MultiPunchData()\n\
	<Key>:		Data()\n";

    /* Create a container for the whole thing. */
    container = XtVaCreateManagedWidget(
	"container", compositeWidgetClass, toplevel,
	XtNwidth, 10,	/* temporary dimensions */
	XtNheight, 10,
	XtNbackground, appres.cabinet,
	NULL);
    XtRealizeWidget(toplevel);

    /* Figure out the card image. */
    if (appres.card != NULL) {
	if ((ccardimg = find_cardimg(appres.card)) == NULL) {
	    ccardimg = default_cardimg();
	    fprintf(stderr, "No such card '%s', defaulting to '%s'\n"
			    "Use '-help' to list the types\n",
			    appres.card, cardimg_name(ccardimg));
	}
    } else {
	ccardimg = default_cardimg();
    }
    attributes.valuemask = XpmSize;
    if (XpmCreatePixmapFromData(display, XtWindow(container),
		cardimg_pixmap_source(ccardimg), &pixmap, &shapemask,
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
    h = BUTTON_GAP + POSW_HEIGHT +
	SWITCH_SKIP + card_height + 2*CARD_AIR + 2*BUTTON_GAP + 2*BUTTON_BW +
	BUTTON_HEIGHT + POWER_GAP + POWER_HEIGHT;
    XtVaSetValues(container,
	XtNwidth, w,
	XtNheight, h,
	NULL);

    /* Add the stacker count. */
    XtVaCreateManagedWidget(
	"stackerDepression", labelWidgetClass, container,
	XtNwidth, STACKER_WIDTH + 2 * POSW_FRAME,
	XtNheight, POSW_HEIGHT,
	XtNx, 0,
	XtNy, 0,
	XtNlabel, "",
	XtNborderWidth, 0,
	NULL);
    stacker = XtVaCreateManagedWidget(
	"stacker", labelWidgetClass, container,
	XtNwidth, STACKER_WIDTH,
	XtNheight, POSW_INNER_HEIGHT,
	XtNx, POSW_FRAME,
	XtNy, POSW_TFRAME,
	XtNborderWidth, 0,
	XtNlabel, "",
	XtNresize, False,
	NULL);
    display_card_count();

    /* Add the position counter. */
    if (XpmCreatePixmapFromData(display, XtWindow(container),
		ci2_xpm, &column_indicator, &shapemask,
		&attributes) != XpmSuccess) {
	    XtError("XpmCreatePixmapFromData failed");
    }
    posw_x = STACKER_WIDTH + 2 * POSW_FRAME + BUTTON_GAP;
    XtVaCreateManagedWidget(
	"depression", labelWidgetClass, container,
	XtNwidth, POSW_WIDTH + ARROW_WIDTH,
	XtNheight, POSW_HEIGHT,
	XtNy, 0,
	XtNx, posw_x,
	XtNlabel, "",
	XtNborderWidth, 0,
	NULL);
    posw_porth = XtVaCreateManagedWidget(
	"posw_porthole", portholeWidgetClass, container,
	XtNwidth, POSW_INNER_WIDTH,
	XtNheight, POSW_INNER_HEIGHT,
	XtNx, posw_x + POSW_FRAME + ARROW_WIDTH,
	XtNy, POSW_TFRAME,
	XtNborderWidth, 0,
	NULL);
    posw = XtVaCreateManagedWidget(
	"posw", compositeWidgetClass, posw_porth,
	XtNwidth, 1350,
	XtNheight, POSW_HEIGHT,
	XtNx, 0,
	XtNy, 0,
	XtNbackgroundPixmap, column_indicator,
	XtNborderWidth, 1,
	XtNborderColor, appres.background,
	NULL);
    if (XpmCreatePixmapFromData(display, XtWindow(container), arrow_xpm,
		&arrow, &shapemask, &attributes) != XpmSuccess) {
	    XtError("XpmCreatePixmapFromData failed");
    }
    (void) XtVaCreateManagedWidget(
	"arrow", compositeWidgetClass, container,
	XtNwidth, ARROW_WIDTH,
	XtNheight, POSW_INNER_HEIGHT,
	XtNx, posw_x + POSW_FRAME,
	XtNy, POSW_TFRAME,
	XtNbackgroundPixmap, arrow,
	XtNborderWidth, 0,
	NULL);

    /* Create the porthole within the container. */
    porth = XtVaCreateManagedWidget(
	"porthole", portholeWidgetClass, container,
	XtNwidth, card_width,
	XtNheight, card_height,
	XtNx, CARD_AIR,
	XtNy, BUTTON_GAP + POSW_HEIGHT + CARD_AIR,
	XtNborderWidth, 0,
	NULL);

    /* Create scrollable region within the porthole. */
    scrollw_column = SLAM_COL;
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

    /* Add the power button. */
    XtVaCreateManagedWidget(
	"powerDepression", labelWidgetClass, container,
	XtNwidth, w,
	XtNheight, POWER_HEIGHT + 2*CARD_AIR,
	XtNx, -1,
	XtNy, h - POWER_HEIGHT - 2*CARD_AIR,
	XtNlabel, "",
	NULL);
    if (XpmCreatePixmapFromData(display, XtWindow(container), flipper_on_xpm,
		    &flipper_on, &shapemask, &attributes) != XpmSuccess) {
	    XtError("XpmCreatePixmapFromData failed");
    }
    if (XpmCreatePixmapFromData(display, XtWindow(container), flipper_off_xpm,
		    &flipper_off, &shapemask, &attributes) != XpmSuccess) {
	    XtError("XpmCreatePixmapFromData failed");
    }
    power_widget = XtVaCreateManagedWidget(
	"power", commandWidgetClass, container,
	XtNbackgroundPixmap, flipper_off,
	XtNlabel, "",
	XtNwidth, POWER_WIDTH,
	XtNheight, POWER_HEIGHT,
	XtNx, w - BUTTON_GAP - POWER_WIDTH - 2*BUTTON_BW,
	XtNy, h - CARD_AIR - POWER_HEIGHT,
	XtNborderWidth, 0,
	XtNhighlightThickness, 0,
	NULL
    );
    XtAddCallback(power_widget, XtNcallback, power_callback, NULL);

    /* Add the save button. */
    key_init(&save_key, "SAVE", container,
	    BUTTON_GAP,
	    h - CARD_AIR - POWER_HEIGHT - POWER_GAP - KEY_HEIGHT,
	    save_xpm, save_pressed_xpm,
	    save_key_backend);

    /* Add the drop button. */
    key_init(&drop_key, "DROP", container,
	    BUTTON_GAP + KEY_WIDTH,
	    h - CARD_AIR - POWER_HEIGHT - POWER_GAP - KEY_HEIGHT,
	    drop_xpm, drop_pressed_xpm,
	    drop_key_backend);

    /* Add the FEED button. */
    key_init(&feed_key, "FEED", container,
	    w - (BUTTON_GAP + KEY_WIDTH + 2*BUTTON_BW),
	    h - CARD_AIR - POWER_HEIGHT - POWER_GAP - KEY_HEIGHT,
	    feed_xpm, feed_pressed_xpm,
	    feed_key_backend);
    XtVaSetValues(feed_key.widget, XtNsensitive, mode == M_INTERACTIVE,
	    NULL);

    /* Add the REL button. */
    key_init(&rel_key, "REL", container,
	    w - (BUTTON_GAP + KEY_WIDTH + 2*BUTTON_BW + KEY_WIDTH),
	    h - CARD_AIR - POWER_HEIGHT - POWER_GAP - KEY_HEIGHT,
	    rel_xpm, rel_pressed_xpm,
	    rel_key_backend);
    XtVaSetValues(rel_key.widget, XtNsensitive, mode == M_INTERACTIVE,
	    NULL);

    /* Add the switches. */
    if (XpmCreatePixmapFromData(display, XtWindow(container), off60_xpm,
		    &toggle_off, &shapemask, &attributes) != XpmSuccess) {
	    XtError("XpmCreatePixmapFromData failed");
    }
    if (XpmCreatePixmapFromData(display, XtWindow(container), on60_xpm,
		    &toggle_on, &shapemask, &attributes) != XpmSuccess) {
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
	    XtNy, h - BUTTON_GAP - POWER_HEIGHT - POWER_GAP - SWITCH_HEIGHT - 30,
	    XtNheight, SWITCH_HEIGHT,
	    XtNborderWidth, 0,
	    XtNlabel, "",
	    XtNbackgroundPixmap, toggles[i].on? toggle_on: toggle_off,
	    XtNhighlightThickness, 0,
	    NULL);
	XtAddCallback(toggles[i].w, XtNcallback, toggle_callback,
		&toggles[i]);
	(void) XtVaCreateManagedWidget(
	    "switch", labelWidgetClass, container,
	    XtNwidth, SWITCH_WIDTH,
	    XtNx, sx + i*(SWITCH_WIDTH + BUTTON_GAP),
	    XtNy, h - BUTTON_GAP - POWER_HEIGHT - POWER_GAP - SWITCH_HEIGHT - 40,
	    XtNborderWidth, 0,
	    XtNlabel, top_label[i],
	    NULL);
	(void) XtVaCreateManagedWidget(
	    "switch", labelWidgetClass, container,
	    XtNwidth, SWITCH_WIDTH,
	    XtNx, sx + i*(SWITCH_WIDTH + BUTTON_GAP),
	    XtNy, h - BUTTON_GAP - POWER_HEIGHT - POWER_GAP - 35,
	    XtNborderWidth, 0,
	    XtNlabel, bottom_label1[i],
	    NULL);
	if (bottom_label2[i] != NULL)
	    (void) XtVaCreateManagedWidget(
		"switch", labelWidgetClass, container,
		XtNwidth, SWITCH_WIDTH,
		XtNx, sx + i*(SWITCH_WIDTH + BUTTON_GAP),
		XtNy, h - BUTTON_GAP - POWER_HEIGHT - POWER_GAP - 25,
		XtNborderWidth, 0,
		XtNlabel, bottom_label2[i],
		NULL);
	if (bottom_label3[i] != NULL)
	    (void) XtVaCreateManagedWidget(
		"switch", labelWidgetClass, container,
		XtNwidth, SWITCH_WIDTH,
		XtNx, sx + i*(SWITCH_WIDTH + BUTTON_GAP),
		XtNy, h - BUTTON_GAP - POWER_HEIGHT - POWER_GAP - 15,
		XtNborderWidth, 0,
		XtNlabel, bottom_label3[i],
		NULL);
    }

    /* Create graphics contexts for drawing. */
    xgcv.foreground = appres.foreground;
    xgcv.background = appres.cardcolor;
    xgcv.font = ifontinfo->fid;
    gc = XtGetGC(toplevel, GCForeground|GCBackground|GCFont, &xgcv);
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
    for (j = 0; j < ccard->n_ov[cn]; j++) {
	if (ccard->coltxt[cn][j] == c) {
	    return True;
	}
    }

    if (toggles[T_PRINT].on) {
	if (ccard->n_ov[cn] < N_OV) {
	    ccard->coltxt[cn][ccard->n_ov[cn]] = c;
	    ++ccard->n_ov[cn];
	}
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

#if defined(XXDEBUG) /*[*/
    printf(" draw_col(col %d)\n", cn);
#endif /*]*/

    /* Draw the text at the top, possibly overstruck. */
    for (j = 0; j < ccard->n_ov[cn]; j++) {
	if (ccard->coltxt[cn][j] < ' ')
	    continue;
	XDrawString(display, card_window, gc, x, TOP_PAD + TEXT_PAD,
		(char *)&ccard->coltxt[cn][j], 1);
    }

    /* Draw the holes, top to bottom. */
    for (i = 0; i < N_ROWS; i++) {
	if (ccard->holes[cn] & (0x800>>i)) {
	    XGCValues xgcv;

	    xgcv.ts_x_origin = x;
	    xgcv.ts_y_origin = TOP_PAD + HOLE_PAD + (CELL_Y(i));
	    XChangeGC(display, holegc, GCTileStipXOrigin|GCTileStipYOrigin,
		    &xgcv);

	    XFillRectangle(display, card_window, holegc, x,
		    TOP_PAD + HOLE_PAD + (CELL_Y(i)), hole_width, hole_height);
	}
    }
}

/* Update the column indicator in the lower right. */
static void
set_posw(int c)
{
    col = c;

    if (col < N_COLS)
	XtVaSetValues(posw, XtNx, -(col * 14), NULL);
}

/* Go to the next card. */
static void
queued_newcard(int replace)
{
    int i;

    if (mode != M_INTERACTIVE)
	replace = True;

    if (!ccard || !replace) {
	card_t *c;

	c = (card_t *)XtMalloc(sizeof(card_t));
	c->prev = ccard;
	c->next = NULL;
	if (ccard) {
	    ccard->next = c;
	}
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
    if (appres.autonumber) {
	char ln_buf[9];

	(void) sprintf(ln_buf, "%08d", ccard->seq);
	for (i = 0; i < 8; i++) {
	    punch_char(72+i, ln_buf[i]);
	}
    }
}

/* Redraw the entire card image. */
static void
Redraw_action(Widget wid, XEvent *event, String *params, Cardinal *num_params)
{
    int i;
    Dimension x, y, w, h;

    action_dbg("Redraw", wid, event, params, num_params);

    if (event && event->type == Expose) {
	x = event->xexpose.x;
	y = event->xexpose.y;
	w = event->xexpose.width;
	h = event->xexpose.height;
    } else {
	x = y = 0;
	w = card_width;
	h = card_height;
    }

    /* Slice off the padding. */
    if (x < LEFT_PAD) {			/* Left. */
	if (w <= LEFT_PAD - x) {
#if defined(XDEBUG) /*[*/
	    printf("ignoring left\n");
#endif /*]*/
	    return;
	}
	w -= LEFT_PAD - x;
	x = 0;
    } else {
	x -= LEFT_PAD;
    }
    if (y < TOP_PAD) {			/* Top. */
	if (h <= TOP_PAD - y) {
#if defined(XDEBUG) /*[*/
	    printf("ignoring top\n");
#endif /*]*/
	    return;
	}
	h -= TOP_PAD - y;
	y = 0;
    } else {
	y -= TOP_PAD;
    }
    if (x >= (CELL_X(N_COLS))) {		/* Right. */
#if defined(XDEBUG) /*[*/
	printf("ignoring right\n");
#endif /*]*/
	return;
    }
    if (x + w > (CELL_X(N_COLS)))
	w = (CELL_X(N_COLS)) - x;
    if (y >= (CELL_Y(N_ROWS))) {		/* Bottom. */
#if defined(XDEBUG) /*[*/
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
DeleteWindow_action(Widget wid, XEvent *event, String *params,
	Cardinal *num_params)
{
    action_dbg("DeleteWindow", wid, event, params, num_params);

    if (wid == toplevel) {
	exit(0);
    } else {
	XtPopdown(wid);
    }
}

/* Find the first card in the deck. This is an external entry point. */
card_t *
first_card(void)
{
    card_t *c;

    for (c = ccard; c && c->prev; c = c->prev) {
    }
    if (CARD_REGISTERED && c == ccard) {
	return NULL;
    }
    return c;
}

/* Return the next card, skipping the one in the punch station. */
card_t *
next_card(card_t *c)
{
    card_t *n;

    n = c->next;
    if (CARD_REGISTERED && n == ccard) {
	return NULL;
    }
    return n;
}

static void
save_key_backend(kpkey_t *key)
{
    if (mode == M_INTERACTIVE && power_on) {
	save_popup();
    }
}

/* Clear out the stacker after a save. */
void
clear_stacker(void)
{
    card_t *c;
    card_t *prev = NULL;

    /*
     * Free everything but ccard, and free ccard if it isn't in the punch
     * station.
     */
    for (c = ccard; c != NULL; c = prev) {
	prev = c->prev;
	if (c == ccard) {
	    if (CARD_REGISTERED) {
		c->prev = NULL;
		c->next = NULL;
		continue;
	    } else {
		ccard = NULL;
	    }
	}
	free(c);
    }

    /* Update the card count display. */
    card_count = 0;
    display_card_count();
}

/*
 * Internals of functions that are enqueued with a delay.
 */

static void
queued_nothing(int ignored)
{
}

static void
queued_data(int c)
{
    if (CARD_REGISTERED && col < N_COLS) {
	if (punch_char(col, c)) {
	    draw_col(col);
#if defined(SOUND) /*[*/
	    loud_click();
#endif /*]*/
	    queued_kybd_right(0);
	} else {
	    XBell(display, 0);
	}
    }
}

static void
queued_multipunch(int c)
{
    if (col < N_COLS && punch_char(col, c)) {
	draw_col(col);
#if defined(SOUND) /*[*/
	loud_click();
#endif /*]*/
    }
}

static void
queued_left(int c)
{
    if (col) {
	queued_pan_left(0);
	set_posw(col - 1);
    } else {
	flush_typeahead();
    }
}

static void
queued_kybd_right(int do_click)
{
    if (col < N_COLS) {
	queued_pan_right(do_click);
	set_posw(col + 1);

	/* Do auto-feed. */
	if (mode == M_INTERACTIVE &&
	    toggles[T_AUTO_FEED].on &&
	    col == N_COLS) {

	    do_release(VERY_FAST);
	    do_feed(False);
	}
    } else {
	flush_typeahead();
    }
}

static void
queued_rel_right(int do_click)
{
    if (col < N_COLS) {
	queued_pan_right(do_click);
	set_posw(col + 1);
    }
}

static void
queued_pan_left(int ignored)
{
    scrollw_column--;
    XtVaSetValues(scrollw, XtNx, SCROLLW_X(), NULL);
#if defined(SOUND) /*[*/
    soft_click();
#endif /*]*/
}

static void
queued_pan_right(int do_click)
{
    scrollw_column++;
    XtVaSetValues(scrollw, XtNx, SCROLLW_X(), NULL);
#if defined(SOUND) /*[*/
    if (do_click)
	soft_click();
#endif /*]*/
}

static void
queued_pan_up(int ignored)
{
    Dimension y;

    XtVaGetValues(scrollw, XtNy, &y, NULL);
    y += CELL_HEIGHT;
    XtVaSetValues(scrollw, XtNy, y, NULL);
}

static void
queued_home(int ignored)
{
    queued_pan_left(0);
    set_posw(col - 1);
}

static void
queued_slam(int ignored)
{
    scrollw_column = SLAM_COL;
    XtVaSetValues(scrollw,
	XtNx, SCROLLW_X(),
	XtNy, -(2*card_height) + TOP_PAD,
	NULL);
}

static void
queued_flux(int ignored)
{
    card_state = C_FLUX;
}

static void
queued_registered(int ignored)
{
    card_state = C_REGISTERED;
    set_posw(0);
}

/*
 * Event queueing: Inserting artificial delays in processing certain events.
 */
enum evtype { DUMMY, DATA, MULTI, LEFT, KYBD_RIGHT, HOME,
	      PAN_RIGHT, PAN_LEFT, PAN_UP, SLAM, NEWCARD,
	      FLUX, REGISTERED, REL_RIGHT, POWER_ON, PRESS_FEED, PRESS_REL,
	      EMPTY, STACK
};
void (*eq_fn[])(int) = {
    queued_nothing,
    queued_data,
    queued_multipunch,
    queued_left,
    queued_kybd_right,
    queued_home,
    queued_pan_right,
    queued_pan_left,
    queued_pan_up,
    queued_slam,
    queued_newcard,
    queued_flux,
    queued_registered,
    queued_rel_right,
    queued_power_on,
    queued_press_feed,
    queued_press_rel,
    queued_empty,
    queued_stack
};
char *eq_name[] = {
    "DUMMY", "DATA", "MULTI", "LEFT", "KYBD_RIGHT", "HOME", "PAN_RIGHT",
    "PAN_LEFT", "PAN_UP", "SLAM", "NEWCARD", "FLUX", "REGISTERED",
    "REL_RIGHT", "POWER_ON", "PRESS_FEED", "PRESS_REL", "EMPTY", "STACK"
};
typedef struct event {
    struct event *next;
    enum evtype evtype;
    unsigned char param;
    Boolean interactive;
    int delay;
} event_t;
event_t *eq_first, *eq_last;
int eq_count;

static void
dump_queue(const char *when)
{
    event_t *e = NULL;
    event_t *prev = NULL;
    int cnt = 0;

    if (!appres.debug)
	return;

    printf("queue(%s):", when);
    if (eq_first == NULL) {
	printf(" (empty)\n");
	return;
    }

    for (e = eq_first; e != NULL; e = e->next) {
	if (prev == NULL) {
	    prev = e;
	    cnt = 1;
	    continue;
	}
	if (prev->evtype == e->evtype && prev->interactive == e->interactive && prev->delay == e->delay) {
	    cnt++;
	    continue;
	}
	printf(" %s(%d)%s", eq_name[prev->evtype], prev->delay,
		prev->interactive? "*": "");
	if (cnt > 1)
	    printf("x%d", cnt);
	prev = e;
	cnt = 1;
    }
    if (cnt) {
	printf(" %s(%d)%s", eq_name[prev->evtype], prev->delay,
		prev->interactive? "*": "");
	if (cnt > 1)
	    printf("x%d", cnt);
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

    dump_queue("before deq");

    /* Dequeue it. */
    e = eq_first;
    eq_first = e->next;
    if (eq_first == NULL)
	eq_last = NULL;
    --eq_count;

    /* Run it. */
    dbg_printf("run %s(%d)\n", eq_name[e->evtype], e->delay);
    (*eq_fn[e->evtype])(e->param);

    /* Free it. */
    XtFree((XtPointer)e);
    e = NULL;

    /*
     * If there are more events, schedule the next one.
     * Otherwise, see if the demo FSM needs cranking.
     */
    if (eq_count) {
	(void) XtAppAddTimeOut(appcontext, eq_first->delay, deq_event, NULL);
    } else {
	if (mode != M_INTERACTIVE) {
	    demo_fsm();
	}
    }
}

/* Add an event to the back of the queue. */
static Boolean
enq_event(enum evtype evtype, unsigned char param, Boolean interactive,
	int delay)
{
    event_t *e;

    if (evtype == DUMMY && delay == 0)
	return True;
    e = (event_t *)XtMalloc(sizeof(*e));
    e->evtype = evtype;
    e->param = param;
    e->interactive = interactive;
    e->delay = delay;

    e->next = NULL;
    if (eq_first == NULL)
	eq_first = e;
    if (eq_last != NULL)
	eq_last->next = e;
    eq_last = e;
    dump_queue("after enq");

    if (!eq_count++) {
	(void) XtAppAddTimeOut(appcontext, delay, deq_event, NULL);
    }
    return True;
}

/* Flush typeahead events from the queue. */
static void
flush_typeahead(void)
{
    event_t *e;
    event_t *prev = NULL;
    event_t *next;

    for (e = eq_first; e != NULL; e = next) {
	next = e->next;
	if (e->interactive) {
	    dbg_printf("Flush %s(%d)*\n", eq_name[e->evtype], e->delay);
	    if (prev != NULL) {
		prev->next = e->next;
	    } else {
		eq_first = e->next;
	    }
	    if (e->next == NULL) {
		eq_last = prev;
	    }
	    XtFree((XtPointer)e);
	    eq_count--;
	    continue;
	}
	prev = e;
    }
    dump_queue("after flush");
}

/*
 * Add a character to the current card. This is an external entry point used
 * by the paste logic and the demo logic.
 */
Boolean
add_char(char c)
{
    if (power_on && CARD_REGISTERED) {
	return enq_event(DATA, c, True, SLOW);
    } else {
	return False;
    }
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
Data_action(Widget wid, XEvent *event, String *params, Cardinal *num_params)
{
    XKeyEvent *kevent = (XKeyEvent *)event;
    char buf[10];
    KeySym ks;
    int ll;

    action_dbg("Data", wid, event, params, num_params);

    if (!power_on || !CARD_REGISTERED) {
	return;
    }

    ll = XLookupString(kevent, buf, 10, &ks, (XComposeStatus *)NULL);
    if (ll == 1) {
	enq_event(DATA, buf[0] & 0xff, True, SLOW);
    }
}

static void
MultiPunchData_action(Widget wid, XEvent *event, String *params,
	Cardinal *num_params)
{
    XKeyEvent *kevent = (XKeyEvent *)event;
    char buf[10];
    KeySym ks;
    int ll;

    action_dbg("MultiPunchData", wid, event, params, num_params);

    if (!power_on || !CARD_REGISTERED) {
	return;
    }

    ll = XLookupString(kevent, buf, 10, &ks, (XComposeStatus *)NULL);
    if (ll == 1) {
	enq_event(MULTI, buf[0], True, SLOW);
    }
}

static void
Left_action(Widget wid, XEvent *event, String *params, Cardinal *num_params)
{
    action_dbg("Left", wid, event, params, num_params);

    if (power_on && CARD_REGISTERED) {
	enq_event(LEFT, 0, True, SLOW);
    }
}

static void
Right_action(Widget wid, XEvent *event, String *params, Cardinal *num_params)
{
    action_dbg("Right", wid, event, params, num_params);

    if (power_on && CARD_REGISTERED) {
	enq_event(KYBD_RIGHT, 1, True, SLOW);
    }
}

static void
Home_action(Widget wid, XEvent *event, String *params, Cardinal *num_params)
{
    int i;

    action_dbg("Home", wid, event, params, num_params);

    if (power_on && CARD_REGISTERED) {
	flush_typeahead();
	card_state = C_FLUX;
	for (i = 0; i < col; i++) {
	    enq_event(HOME, 0, False, FAST);
	}
	enq_event(REGISTERED, 0, False, 0);
    }
}

/*
 * Scroll the current card away, and get a new one.
 * This is mapped to the Enter (X11 'Return') key, which is the 029 REL key.
 */
static void
rel_key_backend(kpkey_t *key)
{
    dbg_printf("release(%s) eq_count = %d\n",
	    CARD_REGISTERED? "card": "no card", eq_count);

    if (power_on && CARD_REGISTERED) {
	do_release(VERY_FAST);
	if (toggles[T_AUTO_FEED].on)
	    do_feed(False);
    }
}

static void
Release_action(Widget wid, XEvent *event, String *params, Cardinal *num_params)
{
    action_dbg("Release", wid, event, params, num_params);

    show_key_down(&rel_key);
    if (!power_on || !CARD_REGISTERED) {
	return;
    }
    rel_key_backend(&rel_key);
}

static void
Tab_action(Widget wid, XEvent *event, String *params, Cardinal *num_params)
{
    int i;

    action_dbg("Tab", wid, event, params, num_params);

    if (power_on && CARD_REGISTERED) {
	flush_typeahead();
	card_state = C_FLUX;
	for (i = col; i < 6; i++) {
	    enq_event(KYBD_RIGHT, 1, False, SLOW);
	}
	enq_event(REGISTERED, 0, False, 0);
    }
}

/* Throw away this card. */
static void
drop_key_backend(kpkey_t *key)
{
    int i;

    if (!power_on || !CARD_REGISTERED) {
	return;
    }

    flush_typeahead();
    card_state = C_FLUX;

    /* Scroll the previous card away. */
    for (i = 0; i <= col; i++)
	enq_event(LEFT, 0, False, FAST);
    for (i = 0; i < N_COLS/2 + 3; i++)
	enq_event(PAN_LEFT, 0, False, FAST);

    if (toggles[T_AUTO_FEED].on) {
	do_feed(True);
    } else {
	enq_event(EMPTY, 0, False, 0);
    }
}

/* Feed a new card. */
static void
feed_key_backend(kpkey_t *key)
{
    if (power_on && !eq_count && !CARD_REGISTERED) {
	do_feed(False);
    }
}

/* On-screen key support. */

/* Initialize a key. */
static void
key_init(kpkey_t *key, const char *name, Widget container, int x, int y,
	char *normal_pixmap_src[], char *pressed_pixmap_src[],
	key_backend_t backend)
{
    Pixmap shapemask;
    XpmAttributes attributes;

    memset(key, '\0', sizeof(*key));
    key->name = name;
    attributes.valuemask = XpmSize;
    if (XpmCreatePixmapFromData(display, XtWindow(container),
		normal_pixmap_src, &key->normal_pixmap, &shapemask,
		&attributes) != XpmSuccess) {
	XtError("XpmCreatePixmapFromData failed");
    }
    attributes.valuemask = XpmSize;
    if (XpmCreatePixmapFromData(display, XtWindow(container),
		pressed_pixmap_src, &key->pressed_pixmap, &shapemask,
		&attributes) != XpmSuccess) {
	XtError("XpmCreatePixmapFromData failed");
    }
    key->widget = XtVaCreateManagedWidget(
	    rel_key.name, commandWidgetClass, container,
	    XtNborderWidth, 0,
	    XtNlabel, "",
	    XtNbackgroundPixmap, key->normal_pixmap,
	    XtNheight, KEY_HEIGHT,
	    XtNwidth, KEY_WIDTH,
	    XtNx, x,
	    XtNy, y,
	    XtNhighlightThickness, 0,
	    NULL);
    XtAddCallback(key->widget, XtNcallback, key_press, (XtPointer)key);
    key->backend = backend;
}

/* Timeout callback. Pop the key back up. */
static void
pop_key(XtPointer data, XtIntervalId *id)
{
    kpkey_t *key = (kpkey_t *)data;

    XtVaSetValues(key->widget, XtNbackgroundPixmap, key->normal_pixmap, NULL);
    key->timeout_id = 0;
}

/* Handle graphical transitions for a click. */
static void
show_key_down(kpkey_t *key)
{
    XtVaSetValues(key->widget, XtNbackgroundPixmap, key->pressed_pixmap, NULL);
    if (key->timeout_id != 0) {
	XtRemoveTimeOut(key->timeout_id);
    }
    key->timeout_id = XtAppAddTimeOut(appcontext, VERY_SLOW, pop_key,
	    (XtPointer)key);
}

/*
 * Mouse click callback. Press the key down, schedule a timeout to pop it
 * back up, call the key-specific back end.
 */
static void
key_press(Widget w, XtPointer client_data, XtPointer call_data)
{
    kpkey_t *key = (kpkey_t *)client_data;

    show_key_down(key);
    if (key->backend) {
	key->backend(key);
    }
}

/*
 * Release the card in the punch station, i.e., scroll it off to the left.
 * This is just the release operation. Auto-feed needs to be implemented by the
 * caller.
 */
static void
do_release(int delay)
{
    int i;

    /* The card is now officially invisible. */
    flush_typeahead();
    card_state = C_FLUX;

    /* Space over the remainder of the card. */
    for (i = col; i < N_COLS; i++) {
	enq_event(REL_RIGHT, 0, False, delay);
    }

    /* Scroll the card out of the punch station. */
    for (i = 0; i < N_COLS/2 + 10; i++) {
	enq_event(PAN_RIGHT, 0, False, delay);
    }

    /* The punch station is now empty. */
    enq_event(EMPTY, 0, False, 0);

    /* We've saved a new card. */
    enq_event(STACK, 0, False, 0);
}

/* Pull a card from the (infinite) hopper into the punch station. */
static void
do_feed(Boolean keep_sequence)
{
    int i;

    enq_event(NEWCARD, keep_sequence, False, FAST);

    /* Scroll the new card down. */
    enq_event(SLAM, 0, False, SLOW);
    for (i = 0; i <= N_ROWS; i++) {
	    enq_event(PAN_UP, 0, False, FAST);
    }
    for (i = SLAM_COL; i < SLAM_TARGET_COL; i++) {
	    enq_event(PAN_RIGHT, 0, False, VERY_FAST);
    }

    /* The punch station is now full. */
    enq_event(REGISTERED, 0, False, 0);
}

/* Start-up sequence. */
static void
queued_power_on(int ignored)
{
    power_on = True;
    XtVaSetValues(power_widget, XtNbackgroundPixmap, flipper_on, NULL);
}

static void
queued_press_feed(int ignored)
{
    show_key_down(&feed_key);
}

static void
startup_power_feed(void)
{
    enq_event(POWER_ON, 0, False, VERY_SLOW);
    enq_event(PRESS_FEED, 0, False, VERY_SLOW);
    do_feed(False);
}

static void
startup_power(void)
{
    enq_event(POWER_ON, 0, False, VERY_SLOW);
}

static void
queued_press_rel(int ignored)
{
    show_key_down(&rel_key);
}

static void
queued_empty(int ignored)
{
    card_state = C_EMPTY;
}

static void
display_card_count(void)
{
    char label[64];

    snprintf(label, sizeof(label), "-%04d-", card_count);
    XtVaSetValues(stacker, XtNlabel, label, NULL);
}

static void
queued_stack(int ignored)
{
    card_count++;
    display_card_count();
}

/* Demo processing. */
typedef enum {
    DS_READ,	/* need to read from the file */
    DS_CHAR,	/* need to process a character from the file */
    DS_SPACE,	/* need to space over the rest of the card */
    DS_EOF	/* done */
} demo_state_t;
static const char *bs_name[] = { "READ", "CHAR", "SPACE", "EOF" };
static demo_state_t bs = DS_READ;
XtInputId read_id = 0;

/* Input is now readable. */
static void
read_more(XtPointer closure, int *fd, XtInputId *id)
{
    XtRemoveInput(read_id);
    read_id = 0;
    demo_fsm();
}

/*
 * Crank the demo FSM.
 */
static void
demo_fsm(void)
{
    static char buf[1024];
    ssize_t nr;
    static ssize_t rbsize = 0;
    static char *s = buf + sizeof(buf);
    char c;

    do {
	dbg_printf("demo_fsm: %s\n", bs_name[bs]);

	switch (bs) {

	case DS_READ:
	    /* Keep munching on the same buffer. */
	    if (s < buf + rbsize) {
		dbg_printf(" continuing, %zd more\n", buf + rbsize - s);
	    } else {
		/* Read the next card. */
		nr = read(demofd, buf, sizeof(buf));
		dbg_printf(" got %zd chars\n", nr);
		if (nr == 0) {
		    if (read_id != 0) {
			XtRemoveInput(read_id);
			read_id = 0;
		    }
		    close(demofd);
		    demofd = -1;

		    /* Next, exit. */
		    bs = DS_EOF;
		    break;
		}
		if (nr < 0) {
		    if (errno == EWOULDBLOCK) {
			if (read_id == 0)
			    read_id = XtAppAddInput(appcontext, demofd,
				    (XtPointer)XtInputReadMask, read_more,
				    NULL);
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
	    bs = DS_CHAR;
	    break;

	case DS_CHAR:
	    if (!CARD_REGISTERED) {
		/* XXX: It might be in flux? */
		static Boolean unfed = True;

		if (mode == M_BATCH && unfed) {
		    unfed = False;
		    show_key_down(&feed_key);
		}
		do_feed(False);
		if (mode == M_BATCH) {
		    enq_delay();
		}
		break;
	    }
	    c = *s++;
	    dbg_printf(" c = 0x%02x, col = %d\n", c & 0xff, col);
	    if (c == '\n') {
		/*
		 * End of input line.
		 *
		 * Delay if there is anything to see; start
		 * spacing to the end of the card.
		 */
		if (col)
		    enq_event(PRESS_REL, 0, False, VERY_SLOW);
		else
		    enq_event(PRESS_REL, 0, False, 0);
		bs = DS_SPACE;
		break;
	    }
	    add_char(c);
	    if (s >= buf + rbsize) {
		/*
		 * Ran out of buffer without a newline.
		 *
		 * Go read some more.
		 */
		bs = DS_READ;
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

	case DS_SPACE:
	    dbg_printf(" col = %d\n", col);

	    if (col >= N_COLS - 1) {
		/* End of card.  Release it. */
		do_release(FAST);

		/*
		 * In remote control mode, create a new card.
		 * In demo mode, wait for data before
		 * doing it.
		 */
		if (mode == M_REMOTECTL) {
		    do_feed(False);
		}

		bs = DS_READ;
		break;
	    }

	    /* Add a space. */
	    add_char(' ');
	    break;

	case DS_EOF:
	    /* Done. */
	    do_power_off();
	    break;
	}
    } while (eq_first == NULL && power_on);

}

/* Accessor functions for appres. */

Pixel
get_errcolor(void)
{
    return appres.errcolor;
}

Pixel
get_cabinet(void)
{
    return appres.cabinet;
}

Pixel
get_foreground(void)
{
    return appres.foreground;
}

/* Debug printing. */
static void
dbg_printf(const char *format, ...)
{
    va_list ap;

    if (!appres.debug) {
	return;
    }

    va_start(ap, format);
    vfprintf(stdout, format, ap);
    va_end(ap);
    fflush(stdout);
}

void
action_dbg(const char *name, Widget wid, XEvent *event, String *params,
	Cardinal *num_params)
{
    Cardinal i;

    if (!appres.debug) {
	return;
    }

    printf("%s(widget %p", name, (void *)wid);
    if (event) {
	if (event->type == Expose) {
	    dbg_printf(", Expose x=%d y=%d w=%d h=%d",
		    event->xexpose.x,
		    event->xexpose.y,
		    event->xexpose.width,
		    event->xexpose.height);
	} else {
	    dbg_printf(", event %d", event->type);
	}
    } else {
	dbg_printf(", no event");
    }
    for(i = 0; i < *num_params; i++) {
	printf(", %s", params[i]);
    }
    printf(")\n");
}
