/* Mycroftiv, Amavect!, Umbraticus */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <cursor.h>
#include <thread.h>
#include <keyboard.h>
#include <mouse.h>
#include "elementile.h"
#include "purple.h"

Point
squareinit(Elementile*)
{
	Point p = {1,1};
	return p;
}

int
squarekeyboard(Elementile*, Rune)
{
	return 0;
}

void
squarefree(Elementile*)
{
	return;
}

/* libelementile was designed for the mouse return value to signal the lib if a flushimage was needed but we don't do this, we just return 0 always after modifying the global sel var if the button is pressed in our area */
int
squaremouse(Elementile *e, Mouse m)
{
	Square *s;

	s = e->aux;
	if(m.buttons == 1)
		if(ptinrect(m.xy, s->r))
			sel = s->id;
	return 0;
}

/* draw base color, bonus, piece, coin, line, hexagram, id in that order */
static
void
redraw(Square *s)
{
	Image *color;
	int i, chsq, ell1, ell2;
	Point targ, dest;
	Rectangle align;

	chsq = s->id;
	align.min.x = s->r.min.x;
	align.min.y = s->r.min.y;
	if((s->r.max.y - s->r.min.y) > 75)
		align.min.y = s->r.min.y +10;
	align.max.x = s->r.max.x;
	align.max.y = s->r.max.y;
	switch(s->active){
	case 0: /* base checkerboard pattern of inactive squares */
		if(s->isgoal == 1){
			draw(screen,s->r, destination, nil, ZP);
			break;
		}
		if(visflag > 3){
			draw(screen,s->r, black, nil, ZP);
			break;
		}
		if((s->id % 2 == (s->id / 8) % 2))
			draw(screen, s->r, lightsq, nil, ZP);
		else 
			draw(screen, s->r, darksq, nil, ZP);
		break;
	case 1: /* legal target squares, green unless they are the purple destination */
		if(s->isgoal == 1)
			draw(screen,s->r, purple, nil, ZP);
		else
			draw(screen, s->r, legaltarget, nil, ZP);
		break;
	case 2: /* previously visited squares */
		if(s->isgoal == 1)
			draw(screen,s->r, purple, nil, ZP);
		else {
			if(s->iscurrent == 1)
				draw(screen,s->r, orange, nil, ZP);
			else
				draw(screen, s->r, click, nil, ZP);
		}
		break;
	}
	if(s->moveline == 1){ /* adds cross to squares given bonus via coinflips */
		color = baize;
		if(s->active == 2)
			color = purple;
		targ.x = s->r.min.x;
		targ.y = s->r.min.y + ((s->r.max.y - s->r.min.y) / 2);
		dest.x = s->r.max.x;
		dest.y = targ.y;
		line(screen, targ, dest, 0, 0, 4, color, targ);
		targ.y = s->r.min.y;
		targ.x = s->r.min.x + ((s->r.max.x - s->r.min.x) / 2);
		dest.y = s->r.max.y;
		dest.x = targ.x;
		line(screen, targ, dest, 0, 0, 4, color, targ);
	}
	if(s->drawpiece == 1){
		color = pos->sq[chsq] & WHITE ? whtpc : blkpc;
		if((s->active == 1) && (s->isgoal == 1))
			color = click;
		if((color == blkpc) && (visflag > 3))
			color = altblkpc;
		draw(screen, align, color, masks[pos->sq[chsq] & PC], ZP);
	}
	if(s->coin != 0){
		ell1 = (s->r.max.x - s->r.min.x) / 2;
		ell2 = (s->r.max.y - s->r.min.y) / 2;
		targ.x = s->r.min.x + ell1;
		targ.y = s->r.min.y + ell2;
		if(ell2 < ell1)
			ell1 = ell2;
		ell1 = ell1 - 5;
		color = blkpc;
		if((s->coin % 2) == 1)
			color = whtpc;
		fillellipse(screen, targ, ell1, ell1, color, targ);
	}
	switch(s->line){
	case 1: /* straight line across */
		color=visflag < 3 ? black : whtpc;
		targ.x = s->r.min.x;
		targ.y = s->r.min.y + ((s->r.max.y - s->r.min.y) / 2);
		dest.x = s->r.max.x;
		dest.y = targ.y;
		line(screen, targ, dest, 0, 0, 4, color, targ);
		break;
	case 2: /* line plus overlapping circle */
		color=visflag < 3 ? black : whtpc;
		targ.x = s->r.min.x;
		targ.y = s->r.min.y + ((s->r.max.y - s->r.min.y) / 2);
		dest.x = s->r.max.x;
		dest.y = targ.y;
		line(screen, targ, dest, 0, 0, 4, color, targ);
		ell1=(s->r.max.x - s->r.min.x) / 2;
		ell2=(s->r.max.y - s->r.min.y) / 2;
		targ.x = s->r.min.x + ell1;
		targ.y = s->r.min.y + ell2;
		if(ell2 < ell1)
			ell1 = ell2;
		ell1 = ell1 - 25;
		ellipse(screen, targ, ell1, ell1, 4, color, targ);
		break;
	case 3: /* line with an X across it */
		color = visflag < 3 ? black : whtpc;
		targ.x = s->r.min.x;
		targ.y = s->r.min.y + ((s->r.max.y - s->r.min.y) / 2);
		dest.x = s->r.max.x;
		dest.y = targ.y;
		line(screen, targ, dest, 0, 0, 4, color, targ);
		targ.x = s->r.min.x + ((s->r.max.x - s->r.min.x) / 4);
		targ.y = s->r.min.y + ((s->r.max.y - s->r.min.y) / 4);
		dest.x = s->r.max.x - ((s->r.max.x - s->r.min.x) / 4);
		dest.y = s->r.max.y - ((s->r.max.y - s->r.min.y) / 4);
		line(screen, targ, dest, 0, 0, 4, color, targ);
		targ.x = s->r.min.x + ((s->r.max.x - s->r.min.x) / 4);
		targ.y = s->r.max.y - ((s->r.max.y - s->r.min.y) / 4);
		dest.x = s->r.max.x - ((s->r.max.x - s->r.min.x) / 4);
		dest.y = s->r.min.y + ((s->r.max.y - s->r.min.y) / 4);
		line(screen, targ, dest, 0, 0, 4, color, targ);
		break;
	}
	if(s->drawhexa == 1){
		/* ell1 and ell2 are scaling factors setting to 2 shrinks that dimension */
		ell1 = 1;
		ell2 = 1;
		if((s->r.max.x - s->r.min.x) < 82)
			ell1 = 2;
		if((s->r.max.y - s->r.min.y) < 82)
			ell2 = 2;
		targ.x = s->r.min.x + ((s->r.max.x - s->r.min.x) / 2);
		targ.y = s->r.min.y + (60/ell2);
		dest.x = targ.x + (40/ell1);
		dest.y = targ.y;
		targ.x += ell1;
		targ.y += ell2;
		dest.x += ell1;
		dest.y += ell2;
		for(i = 0; i < 6; i++){
			color = blkpc;
			if((s->isgoal && (s->active != 2)) || s->isstart)
				color = whtpc;
			if(s->binid[i] == '1')
				line(screen, targ, dest, 0, 0, 2/ell2, color, targ);
			else {
				dest.x -= (25/ell1);
				line(screen, targ, dest, 0, 0, 2/ell2, color, targ);
				targ.x += (25/ell1);
				dest.x += (25/ell1);
				line(screen, targ, dest, 0, 0, 2/ell2, color, targ);
				targ.x -= (25/ell1);
			}
			targ.y -= (10/ell2);
			dest.y -= (10/ell2);
		}
	}
	if(s->drawid == 1){
		targ.x = s->r.min.x + 5;
		targ.y = s->r.max.y - 25;
		string(screen, targ, msgbg, ZP, font, s->binid);
	}
}

void
squareresize(Elementile *e, Rectangle r)
{
	Square *s;
	
	s = e->aux;
	s->r = r;
	redraw(s);
}

int
squareupdate(Elementile *e)
{
	Square *s;
	
	s = e->aux;
	redraw(s);
	return 1;
}
