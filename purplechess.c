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
char *buttons3[] = {"Help", "Hexa", "Binary", "Reset", "Exit", nil};
Menu menu3 = {buttons3};
int sel, sqi, start, goal, current, oldsq, chessq, legalclick, wscore, bscore, moves, pcson, clearflag, hexdisp, turnsco, totalsco;
Image *white;
Image *black;
Rectangle textrect, textrect2, boardrect;
char texbuf[512];
char texbuf2[512];

/* this array converts from gray id to chess square id and is duplicated in square.c */
int grtc[64] = {56,48,57,49,58,50,59,51,60,52,61,53,62,54,63,55,40,32,41,33,42,34,43,35,44,36,45,37,46,38,47,39,24,16,25,17,26,18,27,19,28,20,29,21,30,22,31,23,8,0,9,1,10,2,11,3,12,4,13,5,14,6,15,7};

int
ctgr(int find)
{
	int i;

	for(i=0; i <64; i++)
		if(grtc[i] == find)
			return i;
	return 0;
}

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
		saux[i].id = i;
		saux[i].isstart = 0;
		saux[i].active = 0;
		saux[i].isgoal = 0;
		saux[i].iscurrent = 0;
		saux[i].drawid = 0;
		saux[i].drawhexa = 0;
		sprint(saux[i].binid, "000000");
		if(saux[i].id & 32)
			saux[i].binid[0] = '1';
		if(saux[i].id & 16)
			saux[i].binid[1] = '1';
		if(saux[i].id & 8)
			saux[i].binid[2] = '1';
		if(saux[i].id & 4)
			saux[i].binid[3] = '1';
		if(saux[i].id & 2)
			saux[i].binid[4] = '1';
		if(saux[i].id & 1)
			saux[i].binid[5] = '1';
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
	/* in-place fisher-yates shuffle randomization of pieces on board */
	for(i = 63; i >= 0; i--){
		j=nrand(i+1);
		tmp=pos->sq[i];
		pos->sq[i]=pos->sq[j];
		pos->sq[j]=tmp;
	}
}

/* make all possible captures and accumulate score */
void
capallandscore(void)
{
	int i, sco;

	turnsco = 0;
	for(i = 0; i < 64; i++){
		sco = 0;
		if((pos->sq[i] & TARGET) && (pos->sq[i] != NOPIECE)){
			if((pos->sq[i] & PC) == PAWN){
//				print(".pawn.");
				sco = 25;
				if(saux[ctgr(i)].active != 2)
					pcson--; 
			}
			if((pos->sq[i] & PC) == KNIGHT){
//				print(".knight.");
				sco = 65;
				if(saux[ctgr(i)].active != 2)
					pcson--; 
			}
			if((pos->sq[i] & PC) == BISHOP){
//				print(".bishop.");
				sco = 70;
				if(saux[ctgr(i)].active != 2)
					pcson--; 
			}
			if((pos->sq[i] & PC) == ROOK){
//				print(".rook.");
				sco = 135;
				if(saux[ctgr(i)].active != 2)
					pcson--; 
			}
			if((pos->sq[i] & PC) == QUEEN){
//				print(".queen.");
				sco = 210;
				if(saux[ctgr(i)].active != 2)
					pcson--; 
			}
			if((pos->sq[i] & PC) == KING){
//				print(".king.");
				sco = 175;
				if(saux[ctgr(i)].active != 2)
					pcson--; 
			}
			if(pos->n == 1)
				wscore += sco;
			if(pos->n == 0)
				bscore += sco;
			turnsco += sco;
			if(i != chessq)
				pos->sq[i] = NOPIECE;
		}
	}
	totalsco += turnsco;
	sprint(texbuf2, "+ %d points                                ", turnsco);
	stringbg(screen, textrect2.min, white, ZP, font, texbuf2, black, textrect2.min);
}

void
printscore(void)
{
	sprint(texbuf, "sco: %d w: %d b: %d move: %d avg: %d  pcs: %d        ", totalsco, wscore, bscore, moves, (wscore + bscore) / moves, pcson);
	stringbg(screen, textrect.min, white, ZP, font, texbuf, black, textrect.min);
}

/* this is the main game logic which triggers on a click of a valid target square */
void
activehit(void)
{
	int i, legalsqs;

	moves++;
	saux[current].iscurrent = 0;
	oldsq = current;
	current = sel;
	chessq = grtc[sel];
	/* the chess code uses the move # pos->n to determine capture color legality */
	pos->n = 0;
	if(pos->sq[chessq] & BLACK)
		pos->n = 1;
	cleartargs();
	findtargs(chessq);
	capallandscore();
	saux[sel].active = 2;
	saux[sel].iscurrent = 1;
	/* clear previous active squares */
	for(i = 0; i < 6; i++){
		sqi = oldsq ^ (1<<i);
		if(saux[sqi].active == 1)
			saux[sqi].active = 0;
	}
	/* mark new possible targets as valid */
	legalsqs = 0;
	for(i = 0; i < 6; i++){
		sqi = current ^ (1<<i);
		if(saux[sqi].active == 0){
			saux[sqi].active = 1;
			legalsqs++;
		}
	}
	for(i = 0; i < 64; i++)
		selems[i].update(&selems[i]);
	/* print the score and goal completion info */
	if(legalsqs == 0){
		turnsco += moves * 50;
		totalsco += turnsco;
		sprint(texbuf2, "No moves, %d remain, + %d              ", 64 - moves, turnsco);
		stringbg(screen, textrect2.min, white, ZP, font, texbuf2, black, textrect2.min);
	}
	if(saux[sel].isgoal == 1){
		if((11 - moves) > 0){
			turnsco += (11 - moves) * totalsco;
		}
		turnsco += 200;
		totalsco += turnsco;
		sprint(texbuf2, "+ %d, GOAL REACHED!          ", turnsco);
		stringbg(screen, textrect2.min, white, ZP, font, texbuf2, black, textrect2.min);
	}
	if((clearflag == 0) && (pcson == 0)){
		turnsco += (64 - moves) * 50;
		totalsco += turnsco;
		sprint(texbuf2, "+ %d, ALL PIECES SCORED         ", turnsco);
		stringbg(screen, textrect2.min, white, ZP, font, texbuf2, black, textrect2.min);
		clearflag = 1;
	}
	printscore();
}

void
gamereset(void)
{
	int i;

	for(i = 0; i < 64; i++){
		saux[i].active = 0;
		saux[i].isgoal = 0;
		saux[i].iscurrent = 0;
		saux[i].drawhexa = 0;
	}
	totalsco = 0;
	hexdisp = 0;
	legalclick = 0;
	wscore = 0;
	bscore = 0;
	moves = 0;
	clearflag = 0;
	pcson = 32;
	start=nrand(64);
	current=start;
	goal=63-start;
	saux[start].active = 2;
	saux[start].iscurrent = 1;
	saux[start].isstart = 1;
	saux[start].drawhexa = 1;
	saux[goal].isgoal = 1;
	saux[goal].drawhexa = 1;
	for(i = 0; i < 6; i++){
		sqi = start ^ (1<<i);
		saux[sqi].active = 1;
	}
	sel=current;
	if(pos->sq[grtc[current]] != NOPIECE)
		pcson--;
}

void
boardsize(void)
{
	boardrect.min.x = screen->r.min.x;
	boardrect.min.y = screen->r.min.y;
	boardrect.max.x = screen->r.max.x;
	boardrect.max.y = screen->r.max.y - 60;
	textrect.min.x = screen->r.min.x + 5;
	textrect.min.y = screen->r.max.y - 45;
	textrect.max.x = screen->r.max.x;
	textrect.max.y = screen->r.max.y;
	textrect2.min.x = screen->r.min.x + 5;
	textrect2.min.y = screen->r.max.y - 25;
	textrect2.max.x = screen->r.max.x;
	textrect2.max.y = screen->r.max.y;
}

void
instructions(void)
{
	Point printat;

	printat=boardrect.min;
	sprint(texbuf, "Navigate the 6d hypercube along a Gray code path.");
	stringbg(screen, printat, white, ZP, font, texbuf, black, printat);
	printat.y +=25;
	sprint(texbuf, "Each piece you move onto captures everything it can.");
	stringbg(screen, printat, white, ZP, font, texbuf, black, printat);
	printat.y +=25;
	sprint(texbuf, "Travel from the blue starting square to the gold goal.");
	stringbg(screen, printat, white, ZP, font, texbuf, black, printat);
	printat.y +=25;
	sprint(texbuf, "Next select or capture every piece on the board as fast as you can.");
	stringbg(screen, printat, white, ZP, font, texbuf, black, printat);
	printat.y +=25;
	sprint(texbuf, "Then see how much of the rest of the board you can fill in.");
	stringbg(screen, printat, white, ZP, font, texbuf, black, printat);
	printat.y +=25;
	sprint(texbuf, "Points: P-25 Kt-65 B-70 R-135 K-175 Q-210");
	stringbg(screen, printat, white, ZP, font, texbuf, black, printat);
	printat.y +=25;
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
	hexdisp=0;
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
	boardsize();
	root->resize(root, boardrect);
	white = allocimage(display, Rect(0,0,1,1), RGB24, 1, 0xFFFFFFFF);
	black = allocimage(display, Rect(0,0,1,1), RGB24, 1, 0x000000FF);
	enum { MOUSE, RESIZE, KEYS, NONE };
	Alt alts[] = {
		[MOUSE] =  {mctl->c, &m, CHANRCV},
		[RESIZE] =  {mctl->resizec, nil, CHANRCV},
		[KEYS] = {kctl->c, &r, CHANRCV},
		[NONE] =  {nil, nil, CHANEND}
	};

	/* we behave as if the player clicked on the starting square to begin the game */
	activehit();
	for(;;){
		flushimage(display, 1);
noflush:
		switch(alt(alts)){
		case MOUSE:
			if(m.buttons == 4){
				switch(menuhit(3, mctl, &menu3, nil)){
				case 0:
					instructions();
					break;
				case 1:
					hexdisp++;
					if(hexdisp == 3){
						saux[start].drawhexa = 1;
						saux[goal].drawhexa = 1;
						selems[start].update(&selems[start]);
						selems[goal].update(&selems[goal]);
						hexdisp = 0;
						break;
					}
					if(hexdisp == 1){
						saux[start].drawhexa = 0;
						saux[goal].drawhexa = 0;
					}
					for(i = 0; i < 64; i++){
						if(saux[i].drawhexa == 0)
							saux[i].drawhexa = 1;
						else
							saux[i].drawhexa = 0;
						selems[i].update(&selems[i]);
					}
					break;
				case 2:
					for(i = 0; i < 64; i++){
						if(saux[i].drawid == 0)
							saux[i].drawid = 1;
						else
							saux[i].drawid = 0;
						selems[i].update(&selems[i]);
					}
					break;
				case 3:
					chessinit();
					gamereset();
					activehit();
					for(i = 0; i < 64; i++)
						selems[i].update(&selems[i]);
					break;
				case 4:
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
			boardsize();
			root->resize(root, boardrect);
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
