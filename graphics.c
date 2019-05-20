/* alloc functions by Umbraticus, other stuff by Mycroftiv */

Image *
alloccolor(uint color)
{
	Image *tmp;

	tmp = allocimage(display, Rect(0,0,1,1), RGBA32, 1, color);
	if(tmp == nil)
		sysfatal("cannot allocate buffer image: %r");
	return tmp;
}

Image *
alloccolormix(uint color1, uint color2)
{
	Image *tmp;

	tmp = allocimagemix(display, color1, color2);
	if(tmp == nil)
		sysfatal("cannot allocate buffer image: %r");
	return tmp;
}

Image *
allocmask(char *m)
{
	Image *tmp;
	int fd;
	char path[256];

	snprint(path, 256, "%s/%s.bit", maskdir, m);
	fd = open(path, OREAD);
	if(fd < 0) {
		fprint(2, "cannot open image file %s: %r\n", path);
		exits("image");
	}
	tmp = readimage(display, fd, 0);
	if(tmp == nil)
		sysfatal("cannot load image: %r");
	close(fd);
	return tmp;
}

/* the dog et the window, get it back */
void
dogetwindow(void)
{
	if(getwindow(display, Refnone) < 0)
		sysfatal("Cannot reconnect to display: %r");
	draw(screen, screen->r, display->black, nil, ZP);
}

/* allocate colors and piece image masks */
void
setupimages(void)
{
	lightsq = alloccolor(0x666666FF);
	darksq = alloccolor(0x888888FF);
	legaltarget = alloccolor(0x10B754FF);
	click = alloccolor(0x990000FF);
	destination = alloccolor(0x4C95FFFF);
	purple = alloccolor(0xAA55EEFF);
	orange = alloccolor(0xE4A02AFF);
	altblkpc = alloccolor(0x4026FFFF);
	baize = alloccolor(DDarkgreen);
	dark = alloccolor(DYellowgreen);
	light = alloccolor(DPaleyellow);
	blkpc = alloccolor(DBlack);
	whtpc = alloccolormix(DGreyblue, DWhite);
	hlsq = alloccolor(setalpha(DRed, 0xc8));
	white = alloccolor(0xFFFFFFFF);
	black = alloccolor(0x000000FF);
	re = alloccolor(0xFF3D60FF);
	or = alloccolor(0xFFB347FF);
	ye = alloccolor(0xFFFF4FFF);
	gr = alloccolor(0x67FF53FF);
	bl = alloccolor(0x39C1FFFF);
	in = alloccolor(0x0000FFFF);
	vi = alloccolor(0xB614FFFF);
	colorray[0] = black;
	colorray[1] = white;
	colorray[2] = in;
	colorray[3] = or;
	colorray[4] = bl;
	colorray[5] = gr;
	colorray[6] = re;
	colorray[7] = vi;
	masks[NOPIECE] = allocmask("nopiece");
	masks[PAWN] = allocmask("pawn");
	masks[KNIGHT] = allocmask("knight");
	masks[BISHOP] = allocmask("bishop");
	masks[ROOK] = allocmask("rook");
	masks[QUEEN] = allocmask("queen");
	masks[KING] = allocmask("king");
}

/* optionally draw hypercubic connection arcs */
void
overlay(void)
{
	int sqi, col, i, j;
	Point a, b, c, d;

	col = 1;
	for(i=0; i < 64; i++){
		a.x = saux[i].r.min.x;
		a.y = saux[i].r.min.y;
		b.x = saux[i].r.min.x;
		b.y = saux[i].r.max.y;
		for(j=0; j < 6; j++){
			b.x = saux[i].r.min.x;
			b.y = saux[i].r.max.y;
			sqi = i ^ (1<<j);
			d.x = saux[sqi].r.min.x;
			d.y = saux[sqi].r.min.y;
			c.x = saux[sqi].r.min.x;
			c.y = saux[sqi].r.max.y;
			if(a.x == d.x){
				b.x = saux[i].r.max.x;
				b.y = saux[i].r.min.y;
				c.x = saux[sqi].r.max.x;
				c.y = saux[sqi].r.min.y;
			}
			if(visflag % 2)
				col = j + 2;
			bezier(screen, a, b, c, d, 0, 0, 1, colorray[col], a);
		}
	}
}

/* calculate window divisions into board and text areas */
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
	textrect4.min.x = screen->r.min.x + 300;
	textrect4.min.y = screen->r.max.y - 50;
	textrect4.max.x = screen->r.max.x;
	textrect4.max.y = screen->r.max.y;
}

/* menu option "Info" */
void
instructions(void)
{
	Point printat;

	printat = boardrect.min;
	sprint(texbuf, "Navigate the 6d hypercube along a Gray code path.");
	stringbg(screen, printat, white, ZP, font, texbuf, black, printat);
	printat.y += 25;
	sprint(texbuf, "Click on green squares to move to them.");
	stringbg(screen, printat, white, ZP, font, texbuf, black, printat);
	printat.y += 25;
	sprint(texbuf, "Each piece you move onto captures everything it can.");
	stringbg(screen, printat, white, ZP, font, texbuf, black, printat);
	printat.y += 25;
	sprint(texbuf, "First travel from the gold square to the blue/purple goal.");
	stringbg(screen, printat, white, ZP, font, texbuf, black, printat);
	printat.y += 25;
	sprint(texbuf, "Next select or capture every piece as fast as you can.");
	stringbg(screen, printat, white, ZP, font, texbuf, black, printat);
	printat.y += 25;
	sprint(texbuf, "Then attempt to visit all remaining squares.");
	stringbg(screen, printat, white, ZP, font, texbuf, black, printat);
	printat.y += 25;
	sprint(texbuf, "Points: P-125 Kt-325 B-350 R-675 K-825 Q-1050");
	stringbg(screen, printat, white, ZP, font, texbuf, black, printat);
	printat.y += 25;
	sprint(texbuf, "Squares marked with + score double and 250 extra");
	stringbg(screen, printat, white, ZP, font, texbuf, black, printat);
	printat.y += 25;
	sprint(texbuf, "Goal bonus 500 + 4x score for 6 clicks, 2x score for 8");
	stringbg(screen, printat, white, ZP, font, texbuf, black, printat);
	printat.y += 25;
	sprint(texbuf, "Sequential capture bonus by 100 per capture");
	stringbg(screen, printat, white, ZP, font, texbuf, black, printat);
	printat.y += 25;
	sprint(texbuf, "Piece clear score: 500 * (64 - moves)");
	stringbg(screen, printat, white, ZP, font, texbuf, black, printat);
	printat.y += 25;
	sprint(texbuf, "Cube fill score: 350 * squares filled");
	stringbg(screen, printat, white, ZP, font, texbuf, black, printat);
	printat.y += 25;
	sprint(texbuf, "Fill all squares bonus 10000 points");
	stringbg(screen, printat, white, ZP, font, texbuf, black, printat);
	printat.y += 25;
	flushimage(display, 1);
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
	for(i=0; i < 64; i++){
		if(saux[i].drawhexa == 0)
			saux[i].drawhexa = 1;
		else
			saux[i].drawhexa = 0;
		selems[i].update(&selems[i]);
	}
	flushimage(display, 1);
}

/* menu option "Binary" for node/square id display */
void
binarytoggle(void)
{
	int i;

	for(i=0; i < 64; i++){
		if(saux[i].drawid == 0)
			saux[i].drawid = 1;
		else
			saux[i].drawid = 0;
		selems[i].update(&selems[i]);
	}
	flushimage(display, 1);
}

/* menu option "Seed" to display the random seed to produce this game */
void
printseed(void)
{
	sprint(texbuf, "seed: %ld", seed);
	stringbg(screen, boardrect.min, white, ZP, font, texbuf, black, boardrect.min);
	flushimage(display, 1);
}

/* menu option "View" to 5-way toggle hypercubic connection display */
void
vis(void)
{
	int i;

	visflag++;
	if(visflag == 6)
		visflag = 1;
	for(i=0; i < 64; i++)
		selems[i].update(&selems[i]);
	if(visflag != 1)
		overlay();
	flushimage(display, 1);
}

/* menu option "Scores" to view hiscores and seeds */
void
scores(void)
{
	Point printat;

	printat = boardrect.min;
	sprint(texbuf, "High score: %d seed: %ld", hitot, hitotseed);
	stringbg(screen, printat, white, ZP, font, texbuf, black, printat);
	printat.y += 25;
	sprint(texbuf, "p1 high score: %d seed: %ld", hip1, hip1seed);
	stringbg(screen, printat, white, ZP, font, texbuf, black, printat);
	printat.y += 25;
	sprint(texbuf, "p2 high score: %d seed: %ld", hip2, hip2seed);
	stringbg(screen, printat, white, ZP, font, texbuf, black, printat);
	printat.y += 25;
	sprint(texbuf, "p3 high score: %d seed: %ld", hip3, hip3seed);
	stringbg(screen, printat, white, ZP, font, texbuf, black, printat);
	printat.y += 25;
	flushimage(display, 1);
}
