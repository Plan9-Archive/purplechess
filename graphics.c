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
	hstbg = alloccolormix(DPurpleblue, DWhite);
	hstfg = alloccolor(DPurpleblue);
	msgbg = alloccolor(DWhite);
	msgfg = alloccolor(DBlack);
	scrbar = alloccolor(0x999999FF);
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
	int col,i,j;
	Point a,b,c,d;

	col=1;
	for(i = 0; i < 64; i++){
		a.x = saux[i].r.min.x;
		a.y = saux[i].r.min.y;
		b.x = saux[i].r.min.x;
		b.y = saux[i].r.max.y;
		for(j = 0; j < 6; j++){
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
}
