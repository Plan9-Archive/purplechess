/* Mycroftiv, Amavect!, Umbraticus */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <cursor.h>
#include <thread.h>
#include <keyboard.h>
#include <mouse.h>
#include <guiparts.h>
#include "square.h"
#include "chessdat.h"

static Image *on;
static Image *off;
static Image *click;
static Image *goal;

enum{NPATH = 256};

typedef struct Histrow Histrow;
struct Histrow{
	Rectangle n;	/* for move number */
	Rectangle w;	/* for white move */
	Rectangle b;	/* for black move */
};

Rectangle board, square[64], history, messages, scrollbar;
Point xaxis[8], yaxis[8], msgline[NMSGS];
Histrow *hr;
Image *baize, *dark, *light, *blkpc, *whtpc, *hlsq;
Image *hstbg, *hstfg, *msgbg, *msgfg, *scrbar;
Image *masks[7];
char *maskdir = "/sys/games/lib/chess";
int flipped, em, nhist, histborder, maxmsglen;

Image *
allocmask(char *m)
{
	Image *tmp;
	int fd;
	char path[NPATH];

	snprint(path, NPATH, "%s/%s.bit", maskdir, m);
	fd = open(path, OREAD);
	if(fd < 0) {
		fprint(2, "cannot open image file %s: %r\n", path);
		exits("image");
	}
	tmp = readimage(display, fd, 0);
	if(tmp == nil)
		sysfatal("cannot load image: %r");
	close(fd);
	return tmp;
}

static
void
redraw(Square *s)
{
	if(s->active == 0){
		draw(screen, s->r, off, nil, ZP);
		draw(screen, s->r, on, masks[pos->sq[s->id] & PC], ZP);
	}
	if(s->active == 1){
		draw(screen, s->r, on, nil, ZP);
		draw(screen, s->r, off, masks[pos->sq[s->id] & PC], ZP);
	}
	if(s->active == 2){
		draw(screen, s->r, click, nil, ZP);
		draw(screen, s->r, goal, masks[pos->sq[s->id] & PC], ZP);
	}
	if(s->active == 3){
		draw(screen, s->r, goal, nil, ZP);
		draw(screen, s->r, click, masks[QUEEN & PC], ZP);
	}
}

Point
squareinit(Guielem*)
{
	Point p = {1,1};
	
	if(off == nil)
		off = allocimage(display, Rect(0,0,1,1), RGB24, 1, 0x777777FF);
	if(on == nil)
		on = allocimage(display, Rect(0,0,1,1), RGB24, 1, 0x88CC88FF);
	if(click == nil)
		click = allocimage(display, Rect(0,0,1,1), RGB24, 1, 0x990000FF);
	if(goal == nil)
		goal = allocimage(display, Rect(0,0,1,1), RGB24, 1, 0x000099FF);
	if(masks[NOPIECE] == nil){
		masks[NOPIECE] = allocmask("nopiece");
		masks[PAWN] = allocmask("pawn");
		masks[KNIGHT] = allocmask("knight");
		masks[BISHOP] = allocmask("bishop");
		masks[ROOK] = allocmask("rook");
		masks[QUEEN] = allocmask("queen");
		masks[KING] = allocmask("king");
	}
	if(off == nil || on == nil)
		sysfatal("get more ram dude: %r");
	return p;
}

void
squareresize(Guielem *e, Rectangle r)
{
	Square *s;
	
	s = e->aux;
	s->r = r;
	redraw(s);
}

void
squareupdate(Guielem *e)
{
	Square *s;
	
	s = e->aux;
	redraw(s);
}

/*
 * Gonna break convention by not updating ->active and redrawing, for simplicity.
 */
int
squaremouse(Guielem *e, Mouse m)
{
	Square *s;
	
	s = e->aux;
	if(ptinrect(m.xy, s->r))
		return e->tag;
	return -1;
}

/*
 * Insert functionality here.
 */
int
squarekeyboard(Guielem*, Rune)
{
	return -1;
}
