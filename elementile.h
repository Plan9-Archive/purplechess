/* Amavect! */
typedef struct Elementile Elementile;
typedef struct Guipart Guipart;

struct Elementile {
	void *aux; /* the entity */
	Point (*init)(Elementile*); /* electricity is the essence of consciousness */
	void (*resize)(Elementile*, Rectangle); /* the ground is where we live */
	int (*update)(Elementile*); /* the air we breathe brings us to a new state */
	int (*mouse)(Elementile*, Mouse); /* flow of input is the blood of computation */
	int (*keyboard)(Elementile*, Rune); /* strike the keyboard into flame */
	void (*free)(Elementile*); /* vapor escapes our grasp, leaving nothing in its wake */
};

/*
 * *aux is a pointer to the auxillary/internal state of the Elementile.
 * Spark is initialization. Init any internal state.
 *   Return the minimum size.
 * Earth is resize. Refresh any internal state to the current size given and redraw.
 * Air is update. Validate any state changes and redraw. 
 *   Return 1 if redraw needed, 0 if not.
 * Water is mouse. Use the Mouse to update any state and redraw. 
 *   Return 1 if redraw needed, 0 if not.
 * Fire is keyboard. Use the Rune to update any state and redraw. 
 *   Return 1 if redraw needed, 0 if not.
 * Vapor is free. Free any memory and state.
 */

typedef enum Divtype{
	Vdiv,
	Hdiv
} Divtype;

struct Guipart {
	Divtype vh; /* vertical or horizontal division */
	int w; /* division width = 2*w */
	double d; /* division ratio */
	Elementile *lt; /* left or top next element */
	Elementile *rb; /* right or bottom next element */
	/* internal */
	Point ltmin; /* minimum size of lt */
	Point rbmin; /* minimum size of rb */
	Rectangle ltrect; /* current size of lt */
	Rectangle rbrect; /* current size of rb */
	Elementile *sel; /* selected element, can be nil */
	int state; /* state machine */
};

extern Point guipartinit(Elementile*);
extern void guipartresize(Elementile*, Rectangle);
extern int guipartupdate(Elementile*);
extern int guipartmouse(Elementile*, Mouse);
extern int guipartkeyboard(Elementile*, Rune);
extern void guipartfree(Elementile*);
