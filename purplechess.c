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
char *buttons3[] = {"Help", "Hexa", "Binary", "View", "Seed", "Reset", "Retry", "Exit", nil};
Menu menu3 = {buttons3};
int sel, sqi, start, goal, current, oldsq, chessq, legalclick, wscore, bscore, moves, pcson, clearflag, hexdisp, turnsco, totalsco, legalsqs;
long seed;
Image *colorray[8];
Rectangle textrect, textrect2, textrect3, boardrect;
char moving[6];
char texbuf[512], texbuf2[512], texbuf3[512];;

int grtoch[64] = {56, 57, 48, 49, 58, 59, 50, 51, 40, 41, 32, 33, 42, 43, 34, 35, 60, 61, 52, 53, 62, 63, 54, 55, 44, 45, 36, 37, 46, 47, 38, 39, 24, 25, 16, 17, 26, 27, 18, 19, 8, 9, 0, 1, 10, 11, 2, 3, 28, 29, 20, 21, 30, 31, 22, 23, 12, 13, 4, 5, 14, 15, 6, 7};

int
chtogr(int find)
{
	int i;

	for(i=0; i <64; i++)
		if(grtoch[i] == find)
			return i;
	sysfatal("chtogr not found");
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
	int i, a, b, conv; /* index, aux index, adjust */
	Divtype dt;

	for(i = 0; i < 64; i++)
		saux[i].id = i;
	for(i = 0; i < 64; i++){
		selems[i].tag = i;
		selems[i].aux = &saux[i];
		selems[i].init = squareinit;
		selems[i].resize = squareresize;
		selems[i].update = squareupdate;
		selems[i].mouse = squaremouse;
		selems[i].keyboard = squarekeyboard;
		saux[i].isstart = 0;
		saux[i].active = 0;
		saux[i].isgoal = 0;
		saux[i].iscurrent = 0;
		saux[i].drawid = 0;
		saux[i].drawhexa = 0;
		sprint(saux[i].binid, "000000");
		conv=chtogr(i);
		if(saux[conv].id & 32)
			saux[i].binid[0] = '1';
		if(saux[conv].id & 16)
			saux[i].binid[1] = '1';
		if(saux[conv].id & 8)
			saux[i].binid[2] = '1';
		if(saux[conv].id & 4)
			saux[i].binid[3] = '1';
		if(saux[conv].id & 2)
			saux[i].binid[4] = '1';
		if(saux[conv].id & 1)
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
		if(i > 6)
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
	a = 62;
	b = -3;
	for(i = 31; i < 63; i++){
		tree[i].vh = Vdiv;
		tree[i].w = 1;
		tree[i].d = 0.5;
		tree[i].lt = &selems[a + (b * 2)];
		tree[i].rb = &selems[a+1 + (b * 2)];
		a -= 2;
		b += 2;
		if(b == 5)
			b = -3;
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

	draw(screen, boardrect, black, nil, ZP);
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
	for(i=0; i < 6; i++)
		moving[i] = '0';
	for(i=0; i < 5; i++)	
		saux[grtoch[i]].coin = nrand(2) + 2;
	saux[grtoch[6]].coin = nrand(2) + 2;
	for(i=8; i < 13; i++)
		saux[grtoch[i]].coin = nrand(2) + 2;
	saux[grtoch[14]].coin = nrand(2) + 2;	
	for(i=32; i < 37; i++)
		saux[grtoch[i]].coin = nrand(2) + 2;
	saux[grtoch[38]].coin = nrand(2) + 2;	
	for(i=16; i < 21; i++)
		saux[grtoch[i]].line = 1;
	saux[grtoch[22]].line = 1;
	for(i=24; i < 29; i++)
		saux[grtoch[i]].line = 1;
	saux[grtoch[30]].line = 1;
	for(i=48; i < 53; i++)
		saux[grtoch[i]].line = 1;
	saux[grtoch[54]].line = 1;
	/* calculate starting position and moving lines */
	csum=saux[56].coin + saux[57].coin + saux[58].coin;
	switch(csum){
	case 6:
		start +=1;
		saux[61].line = 3;
		moving[5] = '1';
		break;
	case 7:
		start +=1;
		break;
	case 8:
		saux[61].line = 0;
		 break;
	case 9:
		saux[61].line = 2;
		moving[5] = '1';
		break;
	}
	csum=saux[48].coin + saux[49].coin + saux[50].coin;
	switch(csum){
	case 6:
		start += 2;
		saux[53].line = 3;
		moving[4] = '1';
		break;
	case 7:
		start += 2;
		break;
	case 8:
		saux[53].line = 0;
		 break;
	case 9:
		saux[53].line = 2;
		moving[4] = '1';
		break;
	}
	csum=saux[40].coin + saux[41].coin + saux[42].coin;
	switch(csum){
	case 6:
		start += 4;
		saux[45].line = 3;
		moving[3] = '1';
		break;
	case 7:
		start += 4;
		break;
	case 8:
		saux[45].line = 0;
		 break;
	case 9:
		saux[45].line = 2;
		moving[3] = '1';
		break;
	}
	csum=saux[32].coin + saux[33].coin + saux[34].coin;
	switch(csum){
	case 6:
		start += 8;
		saux[37].line = 3;
		moving[2] = '1';
		break;
	case 7:
		start += 8;
		break;
	case 8:
		saux[37].line = 0;
		 break;
	case 9:
		saux[37].line = 2;
		moving[2] = '1';
		break;
	}
	csum=saux[24].coin + saux[25].coin + saux[26].coin;
	switch(csum){
	case 6:
		start += 16;
		saux[29].line = 3;
		moving[1] = '1';
		break;
	case 7:
		start += 16;
		break;
	case 8:
		saux[29].line = 0;
		 break;
	case 9:
		saux[29].line = 2;
		moving[1] = '1';
		break;
	}
	csum=saux[16].coin + saux[17].coin + saux[18].coin;
	switch(csum){
	case 6:
		start += 32;
		saux[21].line = 3;
		moving[0] = '1';
		break;
	case 7:
		start += 32;
		break;
	case 8:
		saux[21].line = 0;
		 break;
	case 9:
		saux[21].line = 2;
		moving[0] = '1';
		break;
	}
	start=grtoch[start];
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
	if(pos->sq[current] != NOPIECE)
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
				if(saux[i].active != 2)
					pcson--; 
			}
			if((pos->sq[i] & PC) == KNIGHT){
				sco = 325;
				if(saux[i].active != 2)
					pcson--; 
			}
			if((pos->sq[i] & PC) == BISHOP){
				sco = 350;
				if(saux[i].active != 2)
					pcson--; 
			}
			if((pos->sq[i] & PC) == ROOK){
				sco = 675;
				if(saux[i].active != 2)
					pcson--; 
			}
			if((pos->sq[i] & PC) == QUEEN){
				sco = 1050;
				if(saux[i].active != 2)
					pcson--; 
			}
			if((pos->sq[i] & PC) == KING){
				sco = 825;
				if(saux[i].active != 2)
					pcson--; 
			}

			if(pos->n == 1)
				wscore += sco;
			if(pos->n == 0)
				bscore += sco;
			if(saux[i].moveline == 1)
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
	if(moves == 0)
		return;
	sprint(texbuf2, "+ %d points", turnsco);
	if(saux[sel].isgoal == 1){
		clearflag++;
		if((11 - moves) > 0){
			turnsco += (11 - moves) * totalsco;
		}
		turnsco += 1000;
		totalsco += turnsco;
		sprint(texbuf2, "+ %d, GOAL REACHED!", turnsco);
		stringbg(screen, textrect2.min, white, ZP, font, texbuf2, black, textrect2.min);
	}
	if((clearflag < 2) && (pcson == 0)){
		clearflag++;
		turnsco += (64 - moves) * 500;
		totalsco += turnsco;
		sprint(texbuf2, "+ %d, ALL PIECES SCORED!", turnsco);
		stringbg(screen, textrect2.min, white, ZP, font, texbuf2, black, textrect2.min);
	}
	if((legalsqs == 0) && (clearflag != 3)){
		clearflag = 3;
		turnsco += moves * 375;
		if(moves == 64)
			turnsco += 10000;
		totalsco += turnsco;
		sprint(texbuf2, "NO MOVES, %d remain, + %d", 64 - moves, turnsco);
		stringbg(screen, textrect2.min, white, ZP, font, texbuf2, black, textrect2.min);
	}
	switch(clearflag){
	case 0:
		sprint(texbuf3, "GOAL: reach the blue/purple square");
		break;
	case 1:
		sprint(texbuf3, "GOAL: capture/select all pieces");
		break;
	case 2:
		sprint(texbuf3, "GOAL: fill all squares");
		break;
	case 3:
		sprint(texbuf3, "Game Over, reset from right-button menu");
		break;
	}
//	sprint(texbuf, "sco: %d w: %d b: %d move: %d avg: %d  pcs: %d", totalsco, wscore, bscore, moves, (wscore + bscore) / moves, pcson);
	sprint(texbuf, "score: %d  move: %d  pieces: %d", totalsco, moves, pcson);
	textrect3.min.x = screen->r.max.x - (stringwidth(font, texbuf3) + 10);
	stringbg(screen, textrect3.min, white, ZP, font, texbuf3, black, textrect3.min);
	stringbg(screen, textrect2.min, white, ZP, font, texbuf2, black, textrect2.min);
	stringbg(screen, textrect.min, white, ZP, font, texbuf, black, textrect.min);
}

/* optionally draw hypercubic connection arcs */
void
overlay(void)
{
	int col,i,j;
	Point a,b,c,d;

	col=1;
	for(i = 0; i < 64; i++){
		a.x=saux[i].r.min.x;
		a.y=saux[i].r.min.y;
		b.x=saux[i].r.min.x;
		b.y=saux[i].r.max.y;
		for(j = 0; j < 6; j++){
			b.x=saux[i].r.min.x;
			b.y=saux[i].r.max.y;
			sqi = i ^ (1<<j);
			d.x=saux[sqi].r.min.x;
			d.y=saux[sqi].r.min.y;
			c.x=saux[sqi].r.min.x;
			c.y=saux[sqi].r.max.y;
			if(a.x == d.x){
				b.x=saux[i].r.max.x;
				b.y=saux[i].r.min.y;
				c.x=saux[sqi].r.max.x;
				c.y=saux[sqi].r.min.y;
			}
			if(visflag % 2)
				col=j+2;
			bezier(screen, a, b, c, d, 0, 0, 1, colorray[col], a);
		}
	}
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
//	chessq = grtc[sel];
	chessq = sel;
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
	if(visflag != 1)
		overlay();
}

void
boardsize(void)
{
	boardrect.min.x = screen->r.min.x;
	boardrect.min.y = screen->r.min.y;
	boardrect.max.x = screen->r.max.x;
	boardrect.max.y = screen->r.max.y - 60;
	textrect.min.x = screen->r.min.x + 5;
	textrect.min.y = screen->r.max.y - 50;
	textrect.max.x = screen->r.max.x;
	textrect.max.y = screen->r.max.y;
	textrect2.min.x = screen->r.min.x + 5;
	textrect2.min.y = screen->r.max.y - 25;
	textrect2.max.x = screen->r.max.x;
	textrect2.max.y = screen->r.max.y;
	textrect3.min.x = screen->r.min.x + 300;
	textrect3.min.y = screen->r.max.y - 25;
	textrect3.max.x = screen->r.max.x;
	textrect3.max.y = screen->r.max.y;
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

/* menu option "Vis" to 5-way toggle hypercubic connection display */
void
vis(void)
{
	int i;

	visflag++;
	if(visflag == 6)
		visflag = 1;
	for(i = 0; i < 64; i++)
		selems[i].update(&selems[i]);
	if(visflag != 1)
		overlay();
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
	visflag = 1;
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
	re = allocimage(display, Rect(0,0,1,1), RGB24, 1, 0xFF3D60FF);
	or = allocimage(display, Rect(0,0,1,1), RGB24, 1, 0xFFB347FF);	
	ye = allocimage(display, Rect(0,0,1,1), RGB24, 1, 0xFFFF4FFF);
	gr = allocimage(display, Rect(0,0,1,1), RGB24, 1, 0x67FF53FF);
	bl = allocimage(display, Rect(0,0,1,1), RGB24, 1, 0x39C1FFFF);
	in = allocimage(display, Rect(0,0,1,1), RGB24, 1, 0x4026FFFF);
	vi = allocimage(display, Rect(0,0,1,1), RGB24, 1, 0xB614FFFF);
	colorray[0]=black;
	colorray[1]=white;
	colorray[2]=re;
	colorray[3]=or;
	colorray[4]=gr;
	colorray[5]=bl;
	colorray[6]=in;
	colorray[7]=vi;
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
					vis();
					break;
				case 4:
					printseed();
					break;
				case 5:
					menureset(0);
					break;
				case 6:
					menureset(1);
					break;
				case 7:
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
