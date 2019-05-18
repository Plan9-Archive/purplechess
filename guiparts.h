/* Amavect! */

typedef struct Guielem Guielem;
typedef struct Guipart Guipart;

typedef enum Divtype{
	Vdiv,
	Hdiv
} Divtype;

struct Guielem {
	int tag; /* tag for switch statements */
	void *aux;
	Point (*init)(Guielem*); /* initialize state and return minimum size */
	void (*resize)(Guielem*, Rectangle); /* passes the Rectangle that it should fit in */
	void (*update)(Guielem*); /* update based on changed values */
	int (*mouse)(Guielem*, Mouse); /* takes Mouse, must update and redraw, returns updated element tag */
	int (*keyboard)(Guielem*, Rune); /* takes a rune, must update and redraw */
};

struct Guipart {
	Divtype vh; /* vertical or horizontal division */
	int w; /* division width = 2*w */
	double d; /* division ratio */
	Guielem *lt; /* left or top next element */
	Guielem *rb; /* right or bottom next element */
	/* internal */
	Point ltmin; /* minimum size of lt */
	Point rbmin; /* minimum size of rb */
	Rectangle ltrect; /* current size of lt */
	Rectangle rbrect; /* current size of rb */
	int sel; /* selected element, 0 = lt, 1 = rb, -1 = none */
};

extern Point guipartinit(Guielem*);
extern void guipartresize(Guielem*, Rectangle);
extern void guipartupdate(Guielem*);
extern int guipartmouse(Guielem*, Mouse);
extern int guipartkeyboard(Guielem*, Rune);
