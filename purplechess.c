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
char *buttons3[] = {"Reset", "Score", "Exit", nil};
Menu menu3 = {buttons3};
int sel, sqi, start, goal, current, oldsq, chessq, legalclick, wscore, bscore, moves;
Image *wheat;
Rectangle *trect;
Guipart textarg;
char texbuf[512];

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
		pos->n = 0;
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
gamereset(void)
{
	int i;

	for(i = 0; i < 64; i++){
		saux[i].id = i;
		saux[i].active = 0;
		saux[i].isgoal = 0;
	}
	legalclick = 0;
	wscore = 0;
	bscore = 0;
	moves = 0;
	start=nrand(64);
	current=start;
	goal=63-start;
	saux[start].active = 2;
	saux[goal].isgoal = 1;
	for(i = 0; i < 6; i++){
		sqi = start ^ (1<<i);
		saux[sqi].active = 1;
	}
}

void
activehit(void)
{
	int i, sco;

	moves++;
	oldsq = current;
	current = sel;
	saux[sel].active = 2;
	chessq = grtc[sel];
	pos->n = 0;
	if(pos->sq[chessq] & BLACK)
		pos->n = 1;
	cleartargs();
	findtargs(chessq);
	for(i = 0; i < 64; i++){
		sco = 0;
		if(i == chessq)
			continue;
		if(pos->sq[i] & TARGET){
			if((pos->sq[i] & PC) == PAWN){
//				print(".pawn.");
				sco = 20;
			}
			if((pos->sq[i] & PC) == KNIGHT){
//				print(".knight.");
				sco = 60;
			}
			if((pos->sq[i] & PC) == BISHOP){
//				print(".bishop.");
				sco = 70;
			}
			if((pos->sq[i] & PC) == ROOK){
//				print(".rook.");
				sco = 130;
			}
			if((pos->sq[i] & PC) == QUEEN){
//				print(".queen.");
				sco = 200;
			}
			if((pos->sq[i] & PC) == KING){
//				print(".king.");
				sco = 170;	
			}
			if(pos->n == 1)
				wscore += sco;
			if(pos->n == 0)
				bscore += sco;
			pos->sq[i] = NOPIECE;
		}
	}
	for(i = 0; i < 6; i++){
		sqi = oldsq ^ (1<<i);
		if(saux[sqi].active == 1)
			saux[sqi].active = 0;
	}
	for(i = 0; i < 6; i++){
		sqi = current ^ (1<<i);
		if(saux[sqi].active == 0)
			saux[sqi].active = 1;
	}
	for(i = 0; i < 64; i++)
		selems[i].update(&selems[i]);
	if(saux[sel].isgoal == 1){
		sprint(texbuf, "score: %d wcap: %d bcap: %d moves: %d avg: %d", (((wscore + bscore) / moves) * (11 - moves) * 100), wscore, bscore, moves, (wscore + bscore) / moves);
		string(screen, trect->min, wheat, ZP, font, texbuf);
	}
}

void
threadmain(int argc, char **argv)
{
	Keyboardctl *kctl;
	Rune r;
	Mousectl *mctl;
	Mouse m;
	int i;

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
	srand(time(0));

	elemsinit();
	chessinit();
	gamereset();
	dogetwindow();
	root->init(root);
	root->resize(root, screen->r);
	wheat = allocimage(display, Rect(0,0,1,1), RGB24, 1, 0xFFFFFFFF);
	textarg = tree[0];
	trect = &(textarg.ltrect);
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
					gamereset();
					for(i = 0; i < 64; i++)
						selems[i].update(&selems[i]);
					break;
				case 1:
					if(moves == 0){
						sprint(texbuf, "wcap: %d bcap: %d moves: %d avg: %d", wscore, bscore, moves, moves);
						string(screen, trect->min, wheat, ZP, font, texbuf);
						break;
					}
					sprint(texbuf, "wcap: %d bcap: %d moves: %d avg: %d", wscore, bscore, moves, (wscore + bscore) / moves);
					string(screen, trect->min, wheat, ZP, font, texbuf);
					break;
				case 2:
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
					activehit();
			
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
			textarg = tree[0];
			trect = &(textarg.ltrect);
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