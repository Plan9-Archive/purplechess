typedef struct Square Square;
struct Square {
	Rectangle r; /* current size */
	int id; /* decimal id follows chess labeling 0 on lower left 63 upper right */
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
	char binid[7]; /* binary id generated after gray code remapping */
	char engname[64]; /* english 3 word name */
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
Image *colorray[8];
Image *masks[7];
char *maskdir;
Square saux[64];
Elementile selems[64];
Guipart tree[63];
Elementile pelems[63];
Elementile *root;
Rectangle textrect, textrect2, textrect3, textrect4, boardrect;
int sel, start, goal, current, oldsq, legalsqs, seqcap;
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
