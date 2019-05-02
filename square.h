typedef struct Square Square;

struct Square {
	int id;
	char active; /* is currently highlighted */
	/* internal */
	Rectangle r; /* current size */
	Mouse olde; /* ye olde mouse input */
};

extern Point squareinit(Guielem*);
extern void squareresize(Guielem*, Rectangle);
extern void squareupdate(Guielem*);
extern int squaremouse(Guielem*, Mouse);
extern int squarekeyboard(Guielem*, Rune);

