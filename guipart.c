/* Amavect! */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <thread.h>
#include <mouse.h>
#include <keyboard.h>
#include "elementile.h"

/*
 * Calculate minimum size by the minimum sizes of the children,
 * the division type, and the division width.
 */
Point
guipartinit(Elementile *e)
{
	Guipart *gp;
	Point p;
	
	gp = e->aux;
	gp->sel = nil;
	gp->ltmin = gp->lt->init(gp->lt);
	gp->rbmin = gp->rb->init(gp->rb);
	if(gp->vh == Hdiv){
		p.x = gp->ltmin.x > gp->rbmin.x ? gp->ltmin.x : gp->rbmin.x;
		p.y = gp->ltmin.y + gp->rbmin.y + 2*gp->w;
	}else{
		p.x = gp->ltmin.x + gp->rbmin.x + 2*gp->w;
		p.y = gp->ltmin.y > gp->rbmin.y ? gp->ltmin.y : gp->rbmin.y;
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
guipartresize(Elementile *e, Rectangle rect)
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
		/* add 0.5 and cast for int rounding */
		gp->ltrect.max.y = (int)(Dy(rect) * gp->d + 0.5) + rect.min.y - w;
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
		/* add 0.5 and cast for int rounding */
		gp->ltrect.max.x = (int)(Dx(rect) * gp->d + 0.5) + rect.min.x - w;
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
int
guipartupdate(Elementile *e)
{
	Guipart *gp;
	
	gp = e->aux;
	gp->lt->update(gp->lt);
	gp->rb->update(gp->rb);
	
	return 1;
}

/*
 * The mouse input may be routed to the child Elementile, depending on some state.
 * If currently focused Elementile is not nil, the mouse is passed to it.
 * (Then, if no button is pressed, search for a new Elementile.) -tweaked
 * If a different Elementile is found, also send the mouse to it.
 * If no Elementile is found, the selected Elementile becomes nil.
 * 
 * Summary of cases for mouse input to be sent to a member Guielem:
 * mouse entry, exit, movement inside, button presses (anywhere), button releases (anywhere).
 */
int
guipartmouse(Elementile *e, Mouse m)
{
	Guipart *gp;
	int v;
	
	gp = e->aux;
	v = 0;
	
	if(gp->sel != nil)
		v = gp->sel->mouse(gp->sel, m);
	/* update position even if mouse button is held -mycro */
//	if(m.buttons == 0){
		if(ptinrect(m.xy, gp->ltrect)){
			if(gp->sel != gp->lt){
				gp->sel = gp->lt;
				v |= gp->sel->mouse(gp->sel, m);
			}
		}else if(ptinrect(m.xy, gp->rbrect)){
			if(gp->sel != gp->rb){
				gp->sel = gp->rb;
				v |= gp->sel->mouse(gp->sel, m);
			}
		}else{
			gp->sel = nil;
		}
//	}
	
	return v;
}

/* 
 * Just send to the currently selected element.
 */
int
guipartkeyboard(Elementile *e, Rune r)
{
	Guipart *gp;
	int v;
	
	gp = e->aux;
	v = 0;
	
	if(gp->sel != nil)
		v = gp->sel->keyboard(gp->sel, r);
	
	return v;
}

void
guipartfree(Elementile *e)
{
	Guipart *gp;
	
	gp = e->aux;
	
	gp->lt->free(gp->lt);
	gp->rb->free(gp->rb);
}
