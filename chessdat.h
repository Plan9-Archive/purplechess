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
int defpromotion, sanmoves;
char *engine, *pgnfile, ai;

