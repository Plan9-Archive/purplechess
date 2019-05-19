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
//	void *aux; /* aux passed into */
//	void (*onchange)(int elemid, int active); /* event for when active changes */
	/* internal */
	Rectangle r; /* current size */
	Mouse olde; /* ye olde mouse input */
	int state;
};

extern Point squareinit(Elementile*);
extern void squareresize(Elementile*, Rectangle);
extern int squareupdate(Elementile*);
extern int squaremouse(Elementile*, Mouse);
extern int squarekeyboard(Elementile*, Rune);
extern void squarefree(Elementile*);

Image *black, *white, *re, *or, *ye, *gr, *bl, *in, *vi;
Image *legaltarget, *lightsq, *darksq, *click, *destination, *purple, *orange;
Image *baize, *dark, *light, *blkpc, *altblkpc, *whtpc, *hlsq;
Image *hstbg, *hstfg, *msgbg, *msgfg, *scrbar;
Image *colorray[8];
Image *masks[7];
char *maskdir;
// int squareaux[64];
Square saux[64];
Elementile selems[64];
Guipart tree[63];
Elementile pelems[63];
Elementile *root; 
Elementile *mousetarg;
Rectangle textrect, textrect2, textrect3, textrect4, boardrect;
int sel, start, goal, current, oldsq, legalclick, legalsqs, seqcap;
int wscore, bscore, moves, pcson, clearflag, turnsco, totalsco;
int hitot, hip1, hip2, hip3, p1sco, p2sco, p3sco;
int hexdisp, visflag, writescores;
long seed, hitotseed, hip1seed, hip2seed, hip3seed;
char moving[6];
char texbuf[512], texbuf2[512], texbuf3[512], texbuf4[512], texbuf5[512];
char scoretxt[1024];
char scorefile[1024];

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
