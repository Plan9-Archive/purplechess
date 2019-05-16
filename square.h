typedef struct Square Square;

struct Square {
	int id;
	int isstart;
	int isgoal;
	int iscurrent;
	int active;
	int drawid;
	int drawhexa;
	int drawpiece;
	int coin;
	int line;
	int moveline;
	char binid[7];
	char engname[64];
	/* internal */
	Rectangle r; /* current size */
	Mouse olde; /* ye olde mouse input */
};

extern Point squareinit(Guielem*);
extern void squareresize(Guielem*, Rectangle);
extern void squareupdate(Guielem*);
extern int squaremouse(Guielem*, Mouse);
extern int squarekeyboard(Guielem*, Rune);
extern Image * alloccolor(uint color);
int visflag;
Image *black, *white, *re, *or, *ye, *gr, *bl, *in, *vi;
