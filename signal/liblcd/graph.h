#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>
#include <math.h>
#include <float.h>

#define LABEL_LEN_MAX		14	/* Maximum length of an identifier */

#define NESTED_CTRL_MAX		20	/* Maximum number of nested control structures */

#define LONG	long
#define ULONG	unsigned long

#ifdef FLOAT32
typedef float			real;

#define fmod			fmodf
#define floor			floorf
#define ceil			ceilf
#define	fabs			fabsf
#define sqrt			sqrtf
#define pow				powf
#define log				logf
#define log10			log10f
#define exp				expf
#define cos				cosf
#define sin				sinf
#define tan				tanf
#define acos			acosf
#define asin			asinf
#define atan			atanf
#define atan2			atan2f
#define erf				erff
#define erfc			erfcf

#define EPSILON			FLT_EPSILON

#define UDF_LEVEL_MAX	12

#define ALIGNMENT		4

#else

typedef double			real;

#define fmod			fmod
#define floor			floor
#define ceil			ceil
#define fabs			fabs
#define sqrt			sqrt
#define pow				pow
#define log				log
#define log10			log10
#define exp				exp
#define cos				cos
#define sin				sin
#define tan				tan
#define acos			acos
#define asin			asin
#define atan			atan
#define atan2			atan2
#define erf				erf
#define erfc			erfc

#define EPSILON			DBL_EPSILON

#define UDF_LEVEL_MAX	40

#define ALIGNMENT		8

#endif

#define ALIGN(sz)		((sz)!=0 ? ((((sz)-1)/ALIGNMENT)+1)*ALIGNMENT : 0)

#ifndef M_PI
#define M_PI            3.14159265358979323846
#endif

typedef real (*func_t)(real x);

/* real vectors */
#define VECT_INIT	{NULL,0}

typedef struct {
	real *data;		/* pointer to allocated data */
	int		n;		/* size of the vector (number of allocated data) */
} vect_t;

real* vect(vect_t *v, int n);		/* allocate the data buffer */
void vect_del(vect_t *v);			/* free the data buffer */

bool vectorize(vect_t *v, real vinit, real vend, real vstep);
bool vectorize_func(vect_t *x, vect_t *y, func_t f);

/* complex numbers */
typedef struct {
	real r, i;						/* real and imaginary part */
} cplx_t;

cplx_t cplx(real r, real i);		/* complex number initialization */
									/* z1=a+jb z2=c+jd */
cplx_t c_add(cplx_t a, cplx_t b);	/* addition z=z1+z2= (a+b)+j(c+d)*/
cplx_t c_mul(cplx_t a, cplx_t b);	/* multiplication z=(z1*z2=a*c-b*d) + j(a*d+b*c) */
real c_abs(cplx_t a);				/* module |z1|=sqrt(a*a+b*b) */
cplx_t c_expj(real a);				/* exp(ja) = cos(a) + j sin(a) */

/* Dicrete time Fourier Transform
   - for signal x(t)
   - at frequency f,
   - when using sampling frequency fs
 */
cplx_t dtft(const vect_t *x, real f, real fs);

/* graphics support */

/* graph options */
#define	G_FRAME			(1U<<0)		/* draw a frame around the graph */
#define	G_AXIS			(1U<<1)		/* draw X and Y axes */
#define	G_XLOG			(1U<<2)		/* X axis is log scales */
#define G_XGRID			(1U<<3)		/* draw the X grid */
#define	G_XTICKS		(1U<<4)		/* draw ticks on the X axis */
#define G_XAUTOTICKS	(1U<<5)		/* ticks on the X axis are generated automatically according to xmin and xmax */
#define G_YLOG			(1U<<6)		/* Y axis is log scales */
#define	G_YGRID			(1U<<7)		/* draw the Y grid */
#define G_YTICKS		(1U<<8)		/* draw ticks on the Y axis */
#define G_YAUTOTICKS	(1U<<9)		/* ticks on the Y axis are generated automatically according to ymin and ymax */
#define G_WORLDUNSET	(1U<<10)	/* no scale set yet */
#define G_AUTOSCALE		(1U<<11)	/* xmin, xmax, ymin, ymax updated according to graph extent */
#define G_AUTOCOLOR		(1U<<12)	/* plot color autoincremented */
#define G_AUTOMARK		(1U<<13)	/* mark type autoincremented */
#define G_AXISUNSET		(1U<<14)	/* axis type (linear/log) is unset */

#define G_LTYPE_MSK		(0xFU<<16)
#define G_MTYPE_MSK		(0xFU<<20)
#define G_COLOR_MSK		(0xFU<<24)
#define G_LWIDTH_MSK	(0xFU<<28)

typedef enum {
	L_SOLID, L_DOTTED, L_DASHED, L_COMB, L_ARROW, L_BAR, L_FBAR, 
	L_STEP, L_FSTEP, L_NONE
} line_t;

typedef enum {
	M_CROSS, M_PLUS, M_DOT, M_STAR, M_CIRCLE, M_FCIRCLE,
	M_SQUARE, M_FSQUARE, M_DIAMOND, M_FDIAMOND, M_ARROW,
	M_TRIANGLE, M_FTRIANGLE, M_NONE
} marker_t;

#define	G_ALIGN_NE				(1U<<0)
#define	G_ALIGN_N				(1U<<1)
#define	G_ALIGN_NW				(1U<<2)
#define	G_ALIGN_W				(1U<<3)
#define	G_ALIGN_SW				(1U<<4)
#define	G_ALIGN_S				(1U<<5)
#define	G_ALIGN_SE				(1U<<6)
#define	G_ALIGN_E				(1U<<7)
#define	G_ALIGN_CENTER			(1U<<8)

#define G_TITLE					(1U<<0)
#define G_XLABEL				(1U<<1)
#define G_YLABEL				(1U<<2)

#define MAX_COLORS			16

void ginit(void);
void gsubplot(int r, int c, int index);
void gsetplot(real xmin, real xmax, real ymin, real ymax, unsigned long flags, unsigned long mask);
void ggetplot(real *xmin, real *xmax, real *ymin, real *ymax, unsigned long *flags);
void gplot(vect_t *hdx, vect_t *hdy);
void gsetxgrid(vect_t *ticks, real factor, unsigned int color);
void gsetygrid(vect_t *ticks, real factor, unsigned int color);
void gtext(real x, real y, char *text, unsigned int align, int angle, unsigned int color);
void glabel(const char *text, unsigned int type);

void gsetcolor(int index);
void gsetlinetype(line_t lt);
void gsetmarkertype(marker_t mt);

void graphic_mode (void);
/* set current graph */
void gsetgraph(int index);
void gclear (void);
void mouse (int *, int *);
void getpixel (real *x, real *y);
void scale (real s);

#endif
