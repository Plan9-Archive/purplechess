/* Amavect! */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <cursor.h>
#include <thread.h>
#include <keyboard.h>
#include <mouse.h>
#include <guiparts.h>
#include "square.h"

static Image *on;
static Image *off;

static
void
redraw(Square *s)
{
	if(s->active == 1)
		draw(screen, s->r, on, nil, ZP);
	else
		draw(screen, s->r, off, nil, ZP);
}

Point
squareinit(Guielem*)
{
	Point p = {1,1};
	
	if(off == nil)
		off = allocimage(display, Rect(0,0,1,1), RGB24, 1, 0x777777FF);
	if(on == nil)
		on = allocimage(display, Rect(0,0,1,1), RGB24, 1, 0x88CC88FF);
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
