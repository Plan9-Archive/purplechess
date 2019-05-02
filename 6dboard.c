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

int pflag = 0;

Square saux[64];
Guielem selems[64];

Guipart tree[63];
Guielem pelems[63];

Guielem *root = &pelems[0];

char *buttons3[] = {"Reset", "Exit", nil};
Menu menu3 = {buttons3};

void
usage(void)
{
	fprint(2, "usage: %s\n", argv0);
	threadexitsall("usage");
}

void
dogetwindow(void)
{
	if(getwindow(display, Refnone) < 0)
		sysfatal("Cannot reconnect to display: %r");
	draw(screen, screen->r, display->black, nil, ZP);
}

/* breadth-first generation, 6 tree layers deep only */
void
elemsinit(void)
{
	int i, a; /* index, aux index */
	Divtype dt;
	
	for(i = 0; i < 64; i++){
		saux[i].active = 0;
		selems[i].tag = i;
		selems[i].aux = &saux[i];
		selems[i].init = squareinit;
		selems[i].resize = squareresize;
		selems[i].update = squareupdate;
		selems[i].mouse = squaremouse;
		selems[i].keyboard = squarekeyboard;
	}
	
	a = 1;
	for(i = 0; i < 31; i++){
		if(i > 2)
			dt = Vdiv;
		else
			dt = Hdiv;
		tree[i].vh = dt;
		tree[i].w = 1;
		tree[i].d = 0.5;
		tree[i].lt = &pelems[a];
		tree[i].rb = &pelems[a+1];
		a += 2;
	}
	
	a = 0;
	for(i = 31; i < 63; i++){
		tree[i].vh = Hdiv;
		tree[i].w = 1;
		tree[i].d = 0.5;
		tree[i].lt = &selems[a];
		tree[i].rb = &selems[a+1];
		a += 2;
	}
	
	for(i = 0; i < nelem(pelems); i++){
		pelems[i] = (Guielem){i, &tree[i], guipartinit, guipartresize, guipartupdate, guipartmouse, guipartkeyboard};
	}
}

void
threadmain(int argc, char **argv)
{
	Keyboardctl *kctl;
	Rune r;
	Mousectl *mctl;
	Mouse m;
	int sel, oldsel, sqi, i; /* selected, old, square index, index */
	int start, goal, current, oldsq, legalclick;
	
	srand(time(0));
	sel = 0;

	ARGBEGIN{
	default:
		usage();
	}ARGEND;
	
	if(initdraw(nil, nil, argv0) < 0)
		sysfatal("%r");
	if((mctl = initmouse(nil, screen)) == nil)
		sysfatal("%r");
	if((kctl = initkeyboard(nil)) == nil)
		sysfatal("%r");

reset:
	elemsinit();
	oldsq= 999;
	legalclick=0;
	start=nrand(64);
	current=start;
	goal=63-start;
	saux[start].active = 2;
	saux[goal].active = 3;
	for(i = 0; i < 6; i++){
		sqi = start ^ (1<<i);
		saux[sqi].active = 1;
	}
	dogetwindow();
	root->init(root);
	root->resize(root, screen->r);
	enum { MOUSE, RESIZE, KEYS, NONE };
	Alt alts[] = {
		[MOUSE] =  {mctl->c, &m, CHANRCV},
		[RESIZE] =  {mctl->resizec, nil, CHANRCV},
		[KEYS] = {kctl->c, &r, CHANRCV},
		[NONE] =  {nil, nil, CHANEND}
	};
	
	for(;;){
		flushimage(display, 1);
noflush:
		switch(alt(alts)){
		case MOUSE:
			if(m.buttons == 4){
				switch(menuhit(3, mctl, &menu3, nil)){
				case 0:
					goto reset;
				case 1:
					threadexitsall(nil);
					break;
				default:
					break;
				}
				break;
			}
			if(m.buttons == 1){
				sel = root->mouse(root, m);
				if(sel < 0)
					break;
				if(saux[sel].active == 1){
					legalclick = 1;
					oldsq = current;
					current = sel;
					saux[sel].active = 2;
					selems[sel].update(&selems[sel]);
				}
				break;
			}
			oldsel = sel;
			sel = root->mouse(root, m);
			if(legalclick == 0){
				if(sel == oldsel)
					goto noflush;
			}
			legalclick = 0;
			if(oldsel >= 0){
				if(oldsq != 999){
					for(i = 0; i < 6; i++){
						sqi = oldsq ^ (1<<i);
						if(saux[sqi].active == 1)
							saux[sqi].active = 0;
						selems[sqi].update(&selems[sqi]);
					}
				}
			}
			if(sel < 0)
				break;
			for(i = 0; i < 6; i++){
				sqi = current ^ (1<<i);
				if(saux[sqi].active == 0)
					saux[sqi].active = 1;
				selems[sqi].update(&selems[sqi]);
			}
			selems[sel].update(&selems[sel]);
			break;
		case RESIZE:
			dogetwindow();
			root->resize(root, screen->r);
			break;
		case KEYS:
			if(r == Kdel)
				threadexitsall(nil);
			goto noflush;
		case NONE:
			print("I'm a woodchuck, not a woodchucker! (thanks for playing)\n");
			goto noflush;
		}
	}
}
