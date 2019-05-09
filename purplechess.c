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

Square saux[64];
Guielem selems[64];
Guipart tree[63];
Guielem pelems[63];
Guielem *root = &pelems[0];
Guielem *mousetarg;
char *buttons3[] = {"Help", "Hexa", "Binary", "Seed", "Reset", "Retry", "Exit", nil};
Menu menu3 = {buttons3};
int sel, sqi, start, goal, current, oldsq, chessq, legalclick, wscore, bscore, moves, pcson, clearflag, hexdisp, turnsco, totalsco, legalsqs;
long seed;
Image *white;
Image *black;
Rectangle textrect, textrect2, boardrect;
char moving[6];
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
	fprint(2, "usage: %s [-s seed]\n", argv0);
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
		if((saux[i].binid[0] == '0') && (saux[i].binid[1] == '0'))
			sprint(saux[i].engname, "purple     ");
		if((saux[i].binid[0] == '0') && (saux[i].binid[1] == '1'))
			sprint(saux[i].engname, "perfect    ");
		if((saux[i].binid[0] == '1') && (saux[i].binid[1] == '0'))
			sprint(saux[i].engname, "steeple    ");
		if((saux[i].binid[0] == '1') && (saux[i].binid[1] == '1'))
			sprint(saux[i].engname, "steep-fact ");
		if((saux[i].binid[2] == '0') && (saux[i].binid[3] == '0'))
			sprint(saux[i].engname + 11, "chess   ");
		if((saux[i].binid[2] == '0') && (saux[i].binid[3] == '1'))
			sprint(saux[i].engname+ 11, "chance  ");
		if((saux[i].binid[2] == '1') && (saux[i].binid[3] == '0'))
			sprint(saux[i].engname + 11, "press   ");
		if((saux[i].binid[2] == '1') && (saux[i].binid[3] == '1'))
			sprint(saux[i].engname + 11, "prance  ");
		if((saux[i].binid[4] == '0') && (saux[i].binid[5] == '0'))
			sprint(saux[i].engname + 19, "fish ");
		if((saux[i].binid[4] == '0') && (saux[i].binid[5] == '1'))
			sprint(saux[i].engname + 19, "far ");
		if((saux[i].binid[4] == '1') && (saux[i].binid[5] == '0'))
			sprint(saux[i].engname + 19, "wish ");
		if((saux[i].binid[4] == '1') && (saux[i].binid[5] == '1'))
			sprint(saux[i].engname + 19, "war ");
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

void
gamereset(void)
{
	int i, csum;

	for(i = 0; i < 64; i++){
		saux[i].active = 0;
		saux[i].isstart = 0;
		saux[i].isgoal = 0;
		saux[i].iscurrent = 0;
		saux[i].drawhexa = 0;
		saux[i].drawpiece = 0;
		saux[i].coin = 0;
		saux[i].moveline = 0;
	}
	start = 0;
	totalsco = 0;
	hexdisp = 0;
	legalclick = 0;
	wscore = 0;
	bscore = 0;
	moves = 0;
	clearflag = 0;
	pcson = 32;
	/* 3 coin method of hexagram generation */
	for(i=0; i < 6; i++){
		saux[i].coin = nrand(2) + 2;
		moving[i] = '0';
	}
	for(i=16; i < 22; i++)
		saux[i].coin = nrand(2) + 2;
	for(i=32; i < 38; i++)
		saux[i].coin = nrand(2) + 2;
	for(i=8; i < 14; i++)
		saux[i].line = 1;
	for(i=8; i < 14; i++)
		saux[i].line = 1;
	for(i=24; i < 30; i++)
		saux[i].line = 1;
	for(i=40; i < 46; i++)
		saux[i].line = 1;
	/* calculate starting position and moving lines */
	csum=saux[0].coin + saux[2].coin + saux[4].coin;
	switch(csum){
	case 6:
		start +=1;
		saux[10].line = 3;
		moving[5] = '1';
		break;
	case 7:
		start +=1;
		break;
	case 8:
		saux[10].line = 0;
		 break;
	case 9:
		saux[10].line = 2;
		moving[5] = '1';
		break;
	}
	csum=saux[1].coin + saux[3].coin + saux[5].coin;
	switch(csum){
	case 6:
		start += 2;
		saux[11].line = 3;
		moving[4] = '1';
		break;
	case 7:
		start += 2;
		break;
	case 8:
		saux[11].line = 0;
		 break;
	case 9:
		saux[11].line = 2;
		moving[4] = '1';
		break;
	}
	csum=saux[16].coin + saux[18].coin + saux[20].coin;
	switch(csum){
	case 6:
		start += 4;
		saux[26].line = 3;
		moving[3] = '1';
		break;
	case 7:
		start += 4;
		break;
	case 8:
		saux[26].line = 0;
		 break;
	case 9:
		saux[26].line = 2;
		moving[3] = '1';
		break;
	}
	csum=saux[17].coin + saux[19].coin + saux[21].coin;
	switch(csum){
	case 6:
		start += 8;
		saux[27].line = 3;
		moving[2] = '1';
		break;
	case 7:
		start += 8;
		break;
	case 8:
		saux[27].line = 0;
		 break;
	case 9:
		saux[27].line = 2;
		moving[2] = '1';
		break;
	}
	csum=saux[32].coin + saux[34].coin + saux[36].coin;
	switch(csum){
	case 6:
		start += 16;
		saux[42].line = 3;
		moving[1] = '1';
		break;
	case 7:
		start += 16;
		break;
	case 8:
		saux[42].line = 0;
		 break;
	case 9:
		saux[42].line = 2;
		moving[1] = '1';
		break;
	}
	csum=saux[33].coin + saux[35].coin + saux[37].coin;
	switch(csum){
	case 6:
		start += 32;
		saux[43].line = 3;
		moving[0] = '1';
		break;
	case 7:
		start += 32;
		break;
	case 8:
		saux[43].line = 0;
		 break;
	case 9:
		saux[43].line = 2;
		moving[0] = '1';
		break;
	}
	current=start;
	goal=63-start;
	for(i = 0; i < 64; i++)
		selems[i].update(&selems[i]);
	draw(screen, textrect, black, nil, ZP);
	sprint(texbuf, "starting square:  ");
	sprint(texbuf + 17, saux[start].binid);
	sprint(texbuf + 23, "  ");
	sprint(texbuf + 25, saux[start].engname);
	sprint(texbuf2, "goal square:      ");
	sprint(texbuf2 + 17, saux[goal].binid);
	sprint(texbuf2 + 23, "  ");
	sprint(texbuf2 + 25, saux[goal].engname);
	stringbg(screen, textrect.min, white, ZP, font, texbuf, black, textrect.min);
	stringbg(screen, textrect2.min, white, ZP, font, texbuf2, black, textrect2.min);
}

/* called by the first activeclick() when moves == 0 to complete boardstate setup */
void
aftercoins(void)
{
	int i;

	for(i = 0; i < 64; i++){
		saux[i].drawpiece = 1;
		saux[i].coin = 0;
		saux[i].line = 0;
	}
	for(i = 0; i < 6; i++){
		if(moving[i] == '1'){
			sqi = start ^ (1<<i);
			saux[sqi].moveline = 1;
		}
	}
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
				sco = 125;
				if(saux[ctgr(i)].active != 2)
					pcson--; 
			}
			if((pos->sq[i] & PC) == KNIGHT){
				sco = 325;
				if(saux[ctgr(i)].active != 2)
					pcson--; 
			}
			if((pos->sq[i] & PC) == BISHOP){
				sco = 350;
				if(saux[ctgr(i)].active != 2)
					pcson--; 
			}
			if((pos->sq[i] & PC) == ROOK){
				sco = 675;
				if(saux[ctgr(i)].active != 2)
					pcson--; 
			}
			if((pos->sq[i] & PC) == QUEEN){
				sco = 1050;
				if(saux[ctgr(i)].active != 2)
					pcson--; 
			}
			if((pos->sq[i] & PC) == KING){
				sco = 825;
				if(saux[ctgr(i)].active != 2)
					pcson--; 
			}
			if(pos->n == 1)
				wscore += sco;
			if(pos->n == 0)
				bscore += sco;
			if(saux[ctgr(i)].moveline == 1)
				sco = (sco * 2) + 250;
			turnsco += sco;
			if(i != chessq)
				pos->sq[i] = NOPIECE;
		}
	}
	totalsco += turnsco;
}

/* print the score and goal completion info */
void
printscore(void)
{
	if(moves == 0){
		return;
	}
	sprint(texbuf2, "+ %d points", turnsco);
	stringbg(screen, textrect2.min, white, ZP, font, texbuf2, black, textrect2.min);
	if((legalsqs == 0) && (clearflag != 2)){
		clearflag = 2;
		turnsco += moves * 375;
		if(moves == 64)
			turnsco += 10000;
		totalsco += turnsco;
		sprint(texbuf2, "No moves, %d remain, + %d", 64 - moves, turnsco);
		stringbg(screen, textrect2.min, white, ZP, font, texbuf2, black, textrect2.min);
	}
	if(saux[sel].isgoal == 1){
		if((11 - moves) > 0){
			turnsco += (11 - moves) * totalsco;
		}
		turnsco += 1000;
		totalsco += turnsco;
		sprint(texbuf2, "+ %d, GOAL REACHED!", turnsco);
		stringbg(screen, textrect2.min, white, ZP, font, texbuf2, black, textrect2.min);
	}
	if((clearflag == 0) && (pcson == 0)){
		turnsco += (64 - moves) * 500;
		totalsco += turnsco;
		sprint(texbuf2, "+ %d, ALL PIECES SCORED", turnsco);
		stringbg(screen, textrect2.min, white, ZP, font, texbuf2, black, textrect2.min);
		clearflag = 1;
	}
	sprint(texbuf, "sco: %d w: %d b: %d move: %d avg: %d  pcs: %d", totalsco, wscore, bscore, moves, (wscore + bscore) / moves, pcson);
	stringbg(screen, textrect.min, white, ZP, font, texbuf, black, textrect.min);
}

/* this is the main game logic which triggers on a click of a valid target square */
void
activehit(void)
{
	int i;

	draw(screen, textrect, black, nil, ZP);
	if(moves == 0)
		aftercoins();
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
	draw(screen, boardrect, black, nil, ZP);
	for(i = 0; i < 64; i++)
		selems[i].update(&selems[i]);
	printscore();
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

/* menu option "Info" */
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
	sprint(texbuf, "First travel from the gold square to the blue goal.");
	stringbg(screen, printat, white, ZP, font, texbuf, black, printat);
	printat.y +=25;
	sprint(texbuf, "Next select or capture every piece as fast as you can.");
	stringbg(screen, printat, white, ZP, font, texbuf, black, printat);
	printat.y +=25;
	sprint(texbuf, "Then attempt to visit all remaining squares.");
	stringbg(screen, printat, white, ZP, font, texbuf, black, printat);
	printat.y +=25;
	sprint(texbuf, "Points: P-125 Kt-325 B-350 R-675 K-825 Q-1050");
	stringbg(screen, printat, white, ZP, font, texbuf, black, printat);
	printat.y +=25;
	sprint(texbuf, "Squares marked with + score double and 250 extra");
	stringbg(screen, printat, white, ZP, font, texbuf, black, printat);
	printat.y +=25;
	sprint(texbuf, "Goal bonus 1000 + 4x score for 6 moves, 2x score for 8");
	stringbg(screen, printat, white, ZP, font, texbuf, black, printat);
	printat.y +=25;
	sprint(texbuf, "Piece clear score: 500 * (64 - moves)");
	stringbg(screen, printat, white, ZP, font, texbuf, black, printat);
	printat.y +=25;
	sprint(texbuf, "Cube fill score: 350 * squares filled");
	stringbg(screen, printat, white, ZP, font, texbuf, black, printat);
	printat.y +=25;
	sprint(texbuf, "Fill all squares bonus 10000 points");
	stringbg(screen, printat, white, ZP, font, texbuf, black, printat);
	printat.y +=25;
}

/* menu option "Hexa" 3-way toggle */
void
hexatoggle(void)
{
	int i;

	hexdisp++;
	if(hexdisp == 3){
		saux[start].drawhexa = 1;
		saux[goal].drawhexa = 1;
		selems[start].update(&selems[start]);
		selems[goal].update(&selems[goal]);
		hexdisp = 0;
		return;
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
}

/* menu option "Binary" for node/square id display */
void
binarytoggle(void)
{
	int i;

	for(i = 0; i < 64; i++){
		if(saux[i].drawid == 0)
			saux[i].drawid = 1;
		else
			saux[i].drawid = 0;
		selems[i].update(&selems[i]);
	}
}

/* menu option "Seed" to display the random seed to produce this game */
void
printseed(void)
{
	Point printhere;

	printhere=boardrect.min;
	sprint(texbuf, "seed: %ld", seed);
	stringbg(screen, printhere, white, ZP, font, texbuf, black, printhere);
}

/* menu option "Reset" to start new game */
void
menureset(int retry)
{
	int i;

	if(!retry)
		seed++;
	srand(seed);
	chessinit();
	gamereset();
	for(i = 0; i < 64; i++)
		selems[i].update(&selems[i]);
}

void
threadmain(int argc, char **argv)
{
	Keyboardctl *kctl;
	Rune r;
	Mousectl *mctl;
	Mouse m;
	char *userseed;
	int i;
	enum { MOUSE, RESIZE, KEYS, NONE };

	seed = 0;
	hexdisp = 0;
	ARGBEGIN{
	case 's':
		userseed=EARGF(usage());
		seed=strtol(userseed, 0, 10);
		break;
	default:
		usage();
	}ARGEND;
	if(initdraw(nil, nil, argv0) < 0)
		sysfatal("%r");
	if((mctl = initmouse(nil, screen)) == nil)
		sysfatal("%r");
	if((kctl = initkeyboard(nil)) == nil)
		sysfatal("%r");
	Alt alts[] = {
		[MOUSE] =  {mctl->c, &m, CHANRCV},
		[RESIZE] =  {mctl->resizec, nil, CHANRCV},
		[KEYS] = {kctl->c, &r, CHANRCV},
		[NONE] =  {nil, nil, CHANEND}
	};
	white = allocimage(display, Rect(0,0,1,1), RGB24, 1, 0xFFFFFFFF);
	black = allocimage(display, Rect(0,0,1,1), RGB24, 1, 0x000000FF);
	if(seed == 0)
		seed=time(0);
	srand(seed);
	elemsinit();
	dogetwindow();
	root->init(root);
	boardsize();
	root->resize(root, boardrect);
	chessinit();
	gamereset();

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
					hexatoggle();
					break;
				case 2:
					binarytoggle();
					break;
				case 3:
					printseed();
					break;
				case 4:
					menureset(0);
					break;
				case 5:
					menureset(1);
					break;
				case 6:
					threadexitsall(nil);
					break;
				default:
					break;
				}
				break;
			}
			/* the standard libguiparts mouse check is inconsistent; the loop of all selems is a workaround */
//			sel = root->mouse(root, m);
			for(i = 0; i < 64; i++){
				mousetarg = &selems[i];
				sel = mousetarg->mouse(&selems[i], m);
				if(sel != -1)
					break;
			}
			if(m.buttons == 1){
				if(sel < 0)
					break;
//				print(" (%d %d)", sel, grtc[sel]);
				if((saux[sel].active == 1) || (moves == 0)){
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
			printscore();
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
