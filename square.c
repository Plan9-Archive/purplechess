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
static Image *off2;
static Image *click;
static Image *goal;
static Image *purple;
static Image *orange;

int gtc[64] = {56,48,57,49,58,50,59,51,60,52,61,53,62,54,63,55,40,32,41,33,42,34,43,35,44,36,45,37,46,38,47,39,24,16,25,17,26,18,27,19,28,20,29,21,30,22,31,23,8,0,9,1,10,2,11,3,12,4,13,5,14,6,15,7};

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
	Point targ;

	chsq=gtc[s->id];
	/* base checkerboard pattern of inactive squares */
	if(s->active == 0){
		if(s->isgoal == 1){
			draw(screen,s->r, goal, nil, ZP);
		} else {
			if((s->id % 4 == 0) || (s->id % 4 == 3)){
				draw(screen, s->r, off, nil, ZP);
			} else 
				draw(screen, s->r, off2, nil, ZP);
		}
		color = pos->sq[chsq] & WHITE ? whtpc : blkpc;
		draw(screen, s->r, color, masks[pos->sq[chsq] & PC], ZP);
//		line(screen, s->r.min, s->r.max, 0, 0, 2, orange, f);
	}
	/* legal target squares, green unless they are the purple goal */
	if(s->active == 1){
		if(s->isgoal == 1){
			draw(screen,s->r, purple, nil, ZP);
		} else
			draw(screen, s->r, on, nil, ZP);
		color = pos->sq[chsq] & WHITE ? whtpc : blkpc;
		if(s->isgoal == 1){
			draw(screen, s->r, click, masks[QUEEN & PC], ZP);
		} else
			draw(screen, s->r, color, masks[pos->sq[chsq] & PC], ZP);
	}
	/* previously visited squares */
	if(s->active == 2){
		if(s->isgoal == 1){
			draw(screen,s->r, goal, nil, ZP);
		} else {
			if(s->iscurrent == 1) {
				draw(screen,s->r, orange, nil, ZP);
			} else
				draw(screen, s->r, click, nil, ZP);
		}
		color = pos->sq[chsq] & WHITE ? whtpc : blkpc;
		draw(screen, s->r, color, masks[pos->sq[chsq] & PC], ZP);
	}
	if(s->drawid == 1){
//		sprint(buf, "%d", s->id);
		targ.x = s->r.min.x;
		targ.y = s->r.min.y + ((s->r.max.y - s->r.min.y) / 2);
		string(screen, targ, msgbg, ZP, font, s->binid);
	}
}

Point
squareinit(Guielem*)
{
	Point p = {1,1};
	
	if(off == nil)
		off = allocimage(display, Rect(0,0,1,1), RGB24, 1, 0x666666FF);
	if(off2 == nil)
		off2 = allocimage(display, Rect(0,0,1,1), RGB24, 1, 0x888888FF);
	if(on == nil)
//		on = allocimage(display, Rect(0,0,1,1), RGB24, 1, 0x88CC88FF);
		on = allocimage(display, Rect(0,0,1,1), RGB24, 1, 0x10B754FF);
	if(click == nil)
		click = allocimage(display, Rect(0,0,1,1), RGB24, 1, 0x990000FF);
	if(goal == nil)
		goal = allocimage(display, Rect(0,0,1,1), RGB24, 1, 0x4C95FFFF);
	if(purple == nil)
		purple = allocimage(display, Rect(0,0,1,1), RGB24, 1, 0xAA55EEFF);
	if(orange == nil)
		orange = allocimage(display, Rect(0,0,1,1), RGB24, 1, 0xE4A02AFF);
	/* this is probably the wrong place for all this stuff */
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

/* Gonna break convention by not updating ->active and redrawing, for simplicity. */
int
squaremouse(Guielem *e, Mouse m)
{
	Square *s;
	
	s = e->aux;
	if(ptinrect(m.xy, s->r))
		return e->tag;
	return -1;
}

int
squarekeyboard(Guielem*, Rune)
{
	return -1;
}
