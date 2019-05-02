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

/*
int gtc[64] = {0,8,1,9,2,10,3,11,4,12,5,13,6,14,7,15,16,24,17,25,18,26,19,27,20,28,21,29,22,30,23,31,32,40,33,41,34,42,35,43,36,44,37,45,38,46,39,47,48,56,49,57,50,58,51,59,52,60,53,61,54,62,55,63};
*/

int gtc[64] = {56,48,57,49,58,50,59,51,60,52,61,53,62,54,63,55,40,32,41,33,42,34,43,35,44,36,45,37,46,38,47,39,24,16,25,17,26,18,27,19,28,20,29,21,30,22,31,23,8,0,9,1,10,2,11,3,12,4,13,5,14,6,15,7};

/*
int gtc[64] = {63,55,62,54,61,53,60,52,59,51,58,50,57,49,56,48,47,39,46,38,45,37,44,36,43,35,42,34,41,33,40,32,31,23,30,22,29,21,28,20,27,19,26,18,25,17,24,16,15,7,14,6,13,5,12,4,11,3,10,2,9,1,8,0};
*/

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

int
graytochess(int gray)
{
	switch(gray){
	case 0:
		return 0;
	case 1:
		return 8;
	case 2:
		return 1;
	case 3:
		return 9;
	case 4:
		return 2;
	case 5:
		return 10;
	case 6:
		return 3;
	case 7:
		return 11;	
	case 8:
		return 4;
	case 9:
		return 12;
	case 10:
		return 5;
	case 11:
		return 13;
	case 12:
		return 6;
	case 13:
		return 14;
	case 14:
		return 7;
	case 15:
		return 15;
	default:
		return gray;
		break;
	}
}

Image *
alloccolor(uint color)
{
	Image *tmp;

	tmp = allocimage(display, Rect(0,0,1,1), RGBA32, 1, color);
	if(tmp == nil)
		sysfatal("cannot allocate buffer image: %r");
	return tmp;
}

Image *
alloccolormix(uint color1, uint color2)
{
	Image *tmp;

	tmp = allocimagemix(display, color1, color2);
	if(tmp == nil)
		sysfatal("cannot allocate buffer image: %r");
	return tmp;
}

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
	Image *color;
	int chsq;
	
	chsq=gtc[s->id];
	if(s->active == 0){
		draw(screen, s->r, off, nil, ZP);
		color = pos->sq[chsq] & WHITE ? whtpc : blkpc;
		draw(screen, s->r, color, masks[pos->sq[chsq] & PC], ZP);
	}
	if(s->active == 1){
		draw(screen, s->r, on, nil, ZP);
		color = pos->sq[chsq] & WHITE ? whtpc : blkpc;
		draw(screen, s->r, color, masks[pos->sq[chsq] & PC], ZP);
	}
	if(s->active == 2){
		draw(screen, s->r, click, nil, ZP);
		color = pos->sq[chsq] & WHITE ? whtpc : blkpc;
		draw(screen, s->r, color, masks[pos->sq[chsq] & PC], ZP);
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
		baize = alloccolor(DDarkgreen);
		dark = alloccolor(DYellowgreen);
		light = alloccolor(DPaleyellow);
		blkpc = alloccolor(DBlack);
		whtpc = alloccolormix(DGreyblue, DWhite);
		hlsq = alloccolor(setalpha(DRed, 0xc8));
		hstbg = alloccolormix(DPurpleblue, DWhite);
		hstfg = alloccolor(DPurpleblue);
		msgbg = alloccolor(DWhite);
		msgfg = alloccolor(DBlack);
		scrbar = alloccolor(0x999999FF);
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
