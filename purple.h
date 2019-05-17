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
Image *black, *white, *re, *or, *ye, *gr, *bl, *in, *vi;
Image *legaltarget, *lightsq, *darksq, *click, *destination, *purple, *orange;
Image *baize, *dark, *light, *blkpc, *altblkpc, *whtpc, *hlsq;
Image *hstbg, *hstfg, *msgbg, *msgfg, *scrbar;
Image *colorray[8];
Image *masks[7];
char *maskdir;
int visflag;
Square saux[64];
Guielem selems[64];
Guipart tree[63];
Guielem pelems[63];
Guielem *root; 
Guielem *mousetarg;
int sel, sqi, start, goal, current, oldsq, legalclick, wscore, bscore, moves, pcson, clearflag, hexdisp, turnsco, totalsco, legalsqs;
long seed;
Rectangle textrect, textrect2, textrect3, boardrect;
char moving[6];
char texbuf[512], texbuf2[512], texbuf3[512];

/* chess logic by Umbraticus */
enum {
	NOPIECE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,
	PC = 7,		/* mask for piece type */
	WHITE = 8,	/* bits for piece colour */
	BLACK = 16,
	TARGET = 32,	/* bit for whether square is targeted */
	WOO = 1,	/* bits for castling rights */
	WOOO = 2,
	BOO = 4,
	BOOO = 8,
	NMSGS = 8,	/* how many messages to show */
	SQ = 50,	/* graphics contants */
	AXISPAD = 8,
	HISTPAD = 1,
	SCROLLBAR = 12,
	MSGPAD = 4,
	OFF = 0, WAITING, MOVING	/* ai state */
};

typedef struct Position Position;
struct Position{
	char sq[64];
	char san[8];
	char castling;
	char eptarg;
	int n;
	Position *prev;
	Position *next;
};
Position *pos;