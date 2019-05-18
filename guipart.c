/* Amavect! */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <thread.h>
#include <mouse.h>
#include <keyboard.h>
#include "guiparts.h"

static
int
max(int a, int b)
{
	if(a > b)
		return a;
	else
		return b;
}

/* rounds only positive doubles correctly */
double
round(double d)
{
	double f, i;
	f = modf(d, &i);
	if(f >= 0.5)
		i += 1;
	return i;
}

/*
 * Calculate minimum size by the minimum sizes of the children,
 * the division type, and the division width.
 */
Point
guipartinit(Guielem *e)
{
	Guipart *gp;
	Point p;
	
	gp = e->aux;
	gp->sel = 0;
	gp->ltmin = gp->lt->init(gp->lt);
	gp->rbmin = gp->rb->init(gp->rb);
	if(gp->vh == Hdiv){
		p.x = max(gp->ltmin.x, gp->rbmin.x);
		p.y = gp->ltmin.y + gp->rbmin.y + 2*gp->w;
	}else{
		p.x = gp->ltmin.x + gp->rbmin.x + 2*gp->w;
		p.y = max(gp->ltmin.y, gp->rbmin.y);
	}
	return p;
}

/*
 * Resize while enforcing the minimum size.
 * Divide rectangle based on horizontal or vertical division.
 * Set lt size by setting max. If lt is too small, increase max.
 * Set rb size by setting min. If rb is too small, increase max.
 * Recurse into subelements.
 */
void
guipartresize(Guielem *e, Rectangle rect)
{
	Guipart *gp;
	int w;
	
	gp = e->aux;
	w = gp->w;
	
	gp->ltrect.min = rect.min;
	gp->rbrect.max = rect.max;
	
	if(gp->vh == Hdiv){
		gp->ltrect.max.x = rect.max.x;
		gp->rbrect.min.x = rect.min.x;
		gp->ltrect.max.y = (int)round(Dy(rect) * gp->d) + rect.min.y - w;
		if(Dy(rect) - Dy(gp->ltrect) < gp->rbmin.y)
			gp->ltrect.max.y = rect.max.y - gp->rbmin.y - 2 * w;
		if(Dy(gp->ltrect) < gp->ltmin.y)
			gp->ltrect.max.y = gp->ltrect.min.y + gp->ltmin.y;
		gp->rbrect.min.y = gp->ltrect.max.y + 2 * w;
		if(Dy(gp->rbrect) < gp->rbmin.y)
			gp->rbrect.max.y = gp->rbrect.min.y + gp->rbmin.y;
	}else{
		gp->ltrect.max.y = rect.max.y;
		gp->rbrect.min.y = rect.min.y;
		gp->ltrect.max.x = (int)round(Dx(rect) * gp->d) + rect.min.x - w;
		if(Dx(rect) - Dx(gp->ltrect) < gp->rbmin.x)
			gp->ltrect.max.x = rect.max.x - gp->rbmin.x - 2 * w;
		if(Dx(gp->ltrect) < gp->ltmin.x)
			gp->ltrect.max.x = gp->ltrect.min.x + gp->ltmin.x;
		gp->rbrect.min.x = gp->ltrect.max.x + 2 * w;
		if(Dx(gp->rbrect) < gp->rbmin.x)
			gp->rbrect.max.x = gp->rbrect.min.x + gp->rbmin.x;
	}
	
	gp->lt->resize(gp->lt, gp->ltrect);
	gp->rb->resize(gp->rb, gp->rbrect);
}

/*
 * Update all elements.
 */
void
guipartupdate(Guielem *e)
{
	Guipart *gp;
	
	gp = e->aux;
	gp->lt->update(gp->lt);
	gp->rb->update(gp->rb);
}

/*
 * First, send the mouse to the focused element. This allows the mouse release to be sent.
 * Then, if a mouse is not held and has exited the element rectangle, search for a new element.
 */
int
guipartmouse(Guielem *e, Mouse m)
{
	Guipart *gp;
	int inrect;
	int v;
	
	gp = e->aux;
	inrect = 0;
	v = -1;
	
	if(gp->sel == 0){
		v = gp->lt->mouse(gp->lt, m);
		inrect = ptinrect(m.xy, gp->ltrect);
	}else if(gp->sel == 1){
		v = gp->rb->mouse(gp->rb, m);
		inrect = ptinrect(m.xy, gp->rbrect);
	}
	if(m.buttons == 0 && !inrect){
		gp->sel = -1;
		if(ptinrect(m.xy, gp->ltrect)){
			gp->sel = 0;
			return v;
		}else if(ptinrect(m.xy, gp->rbrect)){
			gp->sel = 1;
			return v;
		}
	}
	return v;
}

/* 
 * Just send to the currently selected element.
 */
int
guipartkeyboard(Guielem *e, Rune r)
{
	Guipart *gp;
	int v;
	
	gp = e->aux;
	v = -1;
	
	if(gp->sel == 0)
		v = gp->lt->keyboard(gp->lt, r);
	else if(gp->sel == 1)
		v = gp->rb->keyboard(gp->rb, r);
	return v;
}
