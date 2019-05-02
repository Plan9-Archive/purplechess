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
#include "target.c"

int pflag = 0;
Square saux[64];
Guielem selems[64];
Guipart tree[63];
Guielem pelems[63];
Guielem *root = &pelems[0];
char *buttons3[] = {"Reset", "Exit", nil};
Menu menu3 = {buttons3};

int grtc[64] = {56,48,57,49,58,50,59,51,60,52,61,53,62,54,63,55,40,32,41,33,42,34,43,35,44,36,45,37,46,38,47,39,24,16,25,17,26,18,27,19,28,20,29,21,30,22,31,23,8,0,9,1,10,2,11,3,12,4,13,5,14,6,15,7};

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
		saux[i].id = i;
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
chessinit(void)
{
	int i,j,tmp;
		if(pos == nil)
			pos = malloc(sizeof(Position));
		if(pos == nil)
			sysfatal("failed to malloc first move: %r");
		pos->sq[0] = pos->sq[7] = pos->sq[56] = pos->sq[63] = ROOK;
		pos->sq[1] = pos->sq[6] = pos->sq[57] = pos->sq[62] = KNIGHT;
		pos->sq[2] = pos->sq[5] = pos->sq[58] = pos->sq[61] = BISHOP;
		pos->sq[3] = pos->sq[59] = QUEEN;
		pos->sq[4] = pos->sq[60] = KING;
		for(i = 0; i < 8; i++)
			pos->sq[i+8] = pos->sq[i + 48] = PAWN;
		for(i = 0; i < 16; i++)
			pos->sq[i] |= WHITE;
		for(i = 48; i < 64; i++)
			pos->sq[i] |= BLACK;
		for(i = 16; i < 48; i++)
			pos->sq[i] = NOPIECE;

		for(i = 63; i >= 0; i--){
			j=nrand(i+1);
			tmp=pos->sq[i];
			pos->sq[i]=pos->sq[j];
			pos->sq[j]=tmp;
		}

}

void
threadmain(int argc, char **argv)
{
	Keyboardctl *kctl;
	Rune r;
	Mousectl *mctl;
	Mouse m;
	int sel, sqi, i; /* selected, square index, index */
	int start, goal, current, oldsq, chessq, legalclick;
	
	srand(time(0));

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

	elemsinit();
	chessinit();
	legalclick = 0;
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
					chessinit();
					for(i = 0; i < 64; i++){
						saux[i].active = 0;
					}
					legalclick = 0;
					start=nrand(64);
					current=start;
					goal=63-start;
					saux[start].active = 2;
					saux[goal].active = 3;
					for(i = 0; i < 6; i++){
						sqi = start ^ (1<<i);
						saux[sqi].active = 1;
					}
					for(i = 0; i < 64; i++){
						selems[i].update(&selems[i]);
					}
					break;
				case 1:
					threadexitsall(nil);
					break;
				default:
					break;
				}
				break;
			}
			sel = root->mouse(root, m);
			if(m.buttons == 1){
				if(sel < 0)
					break;
//				print(" (%d %d)", sel, grtc[sel]);
				if(saux[sel].active == 1){
					legalclick = 1;
					oldsq = current;
					current = sel;
					saux[sel].active = 2;
					chessq = grtc[sel];
					cleartargs();
					findtargs(chessq);
					for(i = 0; i < 64; i++){
						if(pos->sq[i] & TARGET)
							pos->sq[i] = NOPIECE;
					}
//					selems[sel].update(&selems[sel]);
					for(i = 0; i < 6; i++){
						sqi = oldsq ^ (1<<i);
						if(saux[sqi].active == 1)
							saux[sqi].active = 0;
//						selems[sqi].update(&selems[sqi]);
					}
					for(i = 0; i < 6; i++){
						sqi = current ^ (1<<i);
						if(saux[sqi].active == 0)
							saux[sqi].active = 1;
//						selems[sqi].update(&selems[sqi]);
					}
					for(i = 0; i < 64; i++)
						selems[i].update(&selems[i]);			
				}
				break;
			}
			if(legalclick == 0)
				goto noflush;
			legalclick = 0;
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
