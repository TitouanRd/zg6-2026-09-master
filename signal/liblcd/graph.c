#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "include/board.h"
#include "lib/util.h"
#include "liblcd/graph.h"

#include "lcd.h"

typedef struct _Graph {
	int 			pxmin, pymin, pxmax, pymax, pxorg, pyorg;	// graph window in pixel coordinates
	real 			xorg, xmin, xmax, xfactor, xtick;	// x axis origin, limits and scaling
	real			yorg, ymin, ymax, yfactor, ytick;	// y axis origin, limits and scaling
	int				xscaleexp,yscaleexp;				// x and y axis factor exponent
	unsigned int	flags;								// graph style
	int				color;								// default color for next plot
	int				ltype;								// default line type for next plot
	int				lwidth;								// default line width
	int				mtype;								// default marker type for next plot
	int				msize;								// marker size
} Graph;

typedef struct GraphWindow {
	Graph			graph[9];
	int				pch, pcw;		// char pixel size
	int				n;				// number of graph used
	int				rows,cols;		// subplot layout: nb of rows, nb of columns
	int				cur;			// current graph used
} GraphWindow;

static GraphWindow gw;

static Color gcolors[MAX_COLORS] = {
	BLACK,
	BLUE,
	BRED,
	GRED,
	GBLUE,
	RED,
	MAGENTA,
	GREEN,
	CYAN,
	YELLOW,
	BROWN,
	RGB(0x00,0xAE,0x00),
	RGB(0xFF,0x99,0x66),
	RGB(0x00,0x99,0xFF),
	RGB(0x99,0x66,0xCC),
	WHITE
};
static Color gridcolor = RGB(0xB3,0xB3,0xB3);

const Font *gsfont=&fixed8;

#define MAXPOINTS	512

SPoint shdata[2*MAXPOINTS];

#define TICKSIZE		5
#define SUBTICKSIZE		3

static void g_xgrid(Graph *g, vect_t *xticks, real factor, unsigned int color)
{
	real *ticks=xticks->data;
	int n=xticks->n;
	DC dc;
	
	lcd_get_default_DC(&dc);
	lcd_set_alignment(&dc,ALIGN_S);
	
	if (!(g->flags & G_XLOG)) {
		g->xfactor = (real)(g->pxmax - g->pxmin) / (g->xmax - g->xmin);
		for (int i=0 ; i<n ; i++) {
			shdata[2*i].x = (short)((real)g->pxmin + (ticks[i] - g->xmin) * g->xfactor);
			shdata[2*i].y = (short)g->pymin;
			shdata[2*i+1].x = shdata[2*i].x;
			shdata[2*i+1].y = (short)g->pymax;
		}
		
		if (g->flags & G_XGRID) {	/* draw the grid */
			lcd_draw_segments(shdata,n,gridcolor);
		}
				
		for (int i=0 ; i<n ; i++) {
			shdata[2*i+1].y = (short)(g->pymin+TICKSIZE);
		}
		lcd_draw_segments(shdata,n,BLACK);
		
		for (int i=0 ; i<n ; i++) {
			char s[32];
			float2str(s,fabs(ticks[i]/factor) < 1e-6 ? 0.0 : ticks[i]/factor,6,0,0);
//			snprintf(s,32,"%g",fabs(ticks[i]/factor) < 1e-6 ? 0.0 : ticks[i]/factor);
			shdata[2*i+1].y = (short)g->pymax;
			shdata[2*i].y = (short)(shdata[2*i+1].y-TICKSIZE);
			lcd_draw_string(&dc,shdata[2*i+1].x,shdata[2*i+1].y+fixed12.height/2,s);
		}
		lcd_draw_segments(shdata,n,BLACK);
		
		if (factor!=1.0) {
			char s[32]="x";
			float2str(s+1,factor,6,0,0);
//			snprintf(s,32,"x%g", factor);
			lcd_set_alignment(&dc,ALIGN_SW);
			lcd_draw_string(&dc,g->pxmax+fixed12.width,g->pymax+2*fixed12.height,s);
		}
	} else {
		g->xfactor = (real)(g->pxmax - g->pxmin) / log10(g->xmax / g->xmin);
		for (int i=0 ; i<n ; i++) {
			shdata[2*i].x = (short)((real)g->pxmin + g->xfactor*log10(ticks[i]/g->xmin));
			shdata[2*i].y = (short)g->pymin;
			shdata[2*i+1].x = shdata[2*i].x;
			shdata[2*i+1].y = (short)g->pymax;
		}
		
		if (g->flags & G_XGRID) {
			lcd_draw_segments(shdata,n,gridcolor);
		}
				
		for (int i=0 ; i<n ; i++) {
			real d = ticks[i]/pow(10.0,floor(log10(ticks[i])));
			if (d!=1.0) {
				shdata[2*i+1].y = (short)(g->pymin+SUBTICKSIZE);
			} else {
				shdata[2*i+1].y = (short)(g->pymin+TICKSIZE);
			}
		}
		lcd_draw_segments(shdata,n,BLACK);
		
		for (int i=0 ; i<n ; i++) {
			char s[32];
			shdata[2*i+1].y = (short)g->pymax;
			
			real e = floor(log10(ticks[i]));
			real d = ticks[i]/pow(10.0,e);
			if (d!=1.0) {
				shdata[2*i].y = (short)(shdata[2*i+1].y-SUBTICKSIZE);
			} else {
				shdata[2*i].y = (short)(shdata[2*i+1].y-TICKSIZE);
				float2str(s,e,6,0,0);
//				snprintf(s,32,"%g", e);
				lcd_set_font(&dc,&fixed12);
				lcd_set_alignment(&dc,ALIGN_S);
				lcd_draw_string(&dc,shdata[2*i+1].x,shdata[2*i+1].y+3*fixed12.height/4,"10");
				lcd_set_font(&dc,&fixed8);
				lcd_set_alignment(&dc,ALIGN_E);
				lcd_draw_string(&dc,shdata[2*i+1].x+fixed12.width,shdata[2*i+1].y+3*fixed12.height/4,s);
			}
		}
		lcd_draw_segments(shdata,n,BLACK);
	}
}

static void g_ygrid(Graph *g, vect_t *yticks, real factor, unsigned int color)
{
	real *ticks=yticks->data;
	int n=yticks->n;
	DC dc;
	
	lcd_get_default_DC(&dc);
	lcd_set_alignment(&dc,ALIGN_W);
	
	if (!(g->flags & G_YLOG)) {
		g->yfactor = (real)(g->pymax - g->pymin) / (g->ymax - g->ymin);
		for (int i=0 ; i<n ; i++) {
			shdata[2*i].x = (short)g->pxmin;
			shdata[2*i].y = (short)((real)g->pymax - g->yfactor*(ticks[i]-g->ymin));
			shdata[2*i+1].x = (short)g->pxmax;
			shdata[2*i+1].y = shdata[2*i].y;
		}
		
		if (g->flags & G_YGRID) {	/* draw the grid */
			lcd_draw_segments(shdata,n,gridcolor);
		}
		
		for (int i=0 ; i<n ; i++) {
			char s[32];
			float2str(s,fabs(ticks[i]/factor) < 1e-6 ? 0.0 : ticks[i]/factor,6,0,0);
//			snprintf(s,fabs(ticks[i]/factor) < 1e-6 ? 0.0 : ticks[i]/factor);
			shdata[2*i+1].x = (short)g->pxmin+TICKSIZE;
			lcd_draw_string(&dc,shdata[2*i].x-fixed12.width/2,shdata[2*i].y,s);
		}
		lcd_draw_segments(shdata,n,BLACK);
		
		for (int i=0 ; i<n ; i++) {
			shdata[2*i+1].x = (short)g->pxmax;
			shdata[2*i].x = (short)(shdata[2*i+1].x-TICKSIZE);
		}
		
		lcd_draw_segments(shdata,n,BLACK);
		
		if (factor!=1.0) {
			char s[32]="x";
			float2str(s+1,factor,6,0,0);
//			snprintf(s,32,"x%g", factor);
			lcd_set_alignment(&dc,ALIGN_SE);
			lcd_draw_string(&dc,g->pxmin-5*fixed12.width,g->pymin-fixed12.height,s);
		}
		
	} else {
		g->yfactor = (real)(g->pymax - g->pymin) / log10(g->ymax / g->ymin);
		for (int i=0 ; i<n ; i++) {
			shdata[2*i].x = (short)g->pxmin;
			shdata[2*i].y = (short)((real)g->pymax - g->yfactor*log10(ticks[i]/g->ymin));
			shdata[2*i+1].x = (short)g->pxmax;
			shdata[2*i+1].y = shdata[2*i].y;
		}
		
		if (g->flags & G_YGRID) {
			lcd_draw_segments(shdata,n,gridcolor);
		}
		
		for (int i=0 ; i<n ; i++) {
			char s[32];
			real e = floor(log10(ticks[i]));
			real d = ticks[i]/pow(10.0,e);
			if (d!=1.0) {
				shdata[2*i+1].x = (short)(g->pxmin+SUBTICKSIZE);
			} else {
				shdata[2*i+1].x = (short)(g->pxmin+TICKSIZE);
				int len = float2str(s,e,6,0,0);
//				int len = snprintf(s,32,"%g", e);
				lcd_set_font(&dc,&fixed12);
				lcd_set_alignment(&dc,ALIGN_W);
				lcd_draw_string(&dc,shdata[2*i].x-fixed12.width/2-len*fixed8.width,shdata[2*i].y,"10");
				lcd_set_font(&dc,&fixed8);
				lcd_set_alignment(&dc,ALIGN_E);
				lcd_draw_string(&dc,shdata[2*i].x-fixed12.width/2-len*fixed8.width,shdata[2*i].y-fixed12.height/2,s);
			}
		}
		lcd_draw_segments(shdata,n,BLACK);
		
		for (int i=0 ; i<n ; i++) {
			shdata[2*i+1].x = (short)g->pxmax;
			real d = ticks[i]/pow(10.0,floor(log10(ticks[i])));
			if (d!=1.0) {
				shdata[2*i].x = (short)(shdata[2*i+1].x-SUBTICKSIZE);
			} else {
				shdata[2*i].x = (short)(shdata[2*i+1].x-TICKSIZE);
			}
		}
		lcd_draw_segments(shdata,n,BLACK);
	}
}

static void g_draw_path2d(Graph *g, SPoint *curve, int n, Color c)
{
	short x0, x1, y0, y1;
	switch (g->ltype) {
	case L_SOLID:
	case L_DOTTED:
	case L_DASHED:
		x0=curve[0].x; y0=curve[0].y;
		for (int i=1;i<n;i++) {
			x1=curve[i].x; y1=curve[i].y;
			lcd_line(x0,y0,x1,y1,c);
			x0=x1; y0=y1;
		}
		break;
	case L_COMB:
		for (int i=0;i<n;i++) {
			lcd_line(curve[i].x,g->pyorg,curve[i].x,curve[i].y,c);
		}
		break;
	case L_ARROW:
#if 0
		for (int i=1;i<n;i++) {
			real c1=(real)curve[i-1].x, r1=(real)curve[i-1].y;
			real c2=(real)curve[i].x, r2=(real)curve[i].y;
			real dx = c2 - c1;
			real dy = r2 - r1;
			real norme = sqrt(dx*dx+dy*dy);
			real cs = dx/norme;
			real sn = dy/norme;
			real a = 0.3*norme;
			real b = 0.6*a;
			short x0 = (short)(-a*cs - b*sn + c2);
			short y0 = (short)(-a*sn + b*cs + r2);
			short x1 = curve[i].x;
			short y1 = curve[i].y;
			short x2 = (short)(-a*cs + b*sn + c2);
			short y2 = (short)(-a*sn - b*cs + r2);
		}
#endif
		break;
	case L_BAR:
		for (int i=0;i<n-1;i++) {
			if (curve[i].y>=g->pyorg) {
				lcd_draw_rect(curve[i].x,g->pyorg,curve[i+1].x-curve[i].x+1,curve[i].y-g->pyorg+1,c);
			} else {
				lcd_draw_rect(curve[i].x,curve[i].y,curve[i+1].x-curve[i].x+1,g->pyorg-curve[i].y+1,c);
			}
		}
		break;
	case L_FBAR:
		for (int i=0;i<n-1;i++) {
			if (curve[i].y>=g->pyorg) {
				lcd_fill_rect(curve[i].x,g->pyorg,curve[i+1].x-curve[i].x-1,curve[i].y-g->pyorg+1,c);
			} else {
				lcd_fill_rect(curve[i].x,curve[i].y,curve[i+1].x-curve[i].x-1,g->pyorg-curve[i].y+1,c);
			}
		}
		break;
	case L_STEP:
		for (int i=0; i<n-1; i++) {
			lcd_line(curve[i].x,curve[i].y,curve[i+1].x,curve[i].y,c);
			lcd_line(curve[i+1].x,curve[i].y,curve[i+1].x,curve[i+1].y,c);
		}
		break;
	case L_FSTEP:
		for (int i=0;i<n-1;i++) {
			if (curve[i].y>=g->pyorg) {
				lcd_fill_rect(curve[i].x,g->pyorg,curve[i+1].x-curve[i].x,curve[i].y-g->pyorg+1,c);
			} else {
				lcd_fill_rect(curve[i].x,curve[i].y,curve[i+1].x-curve[i].x,g->pyorg-curve[i].y+1,c);
			}
		}
		break;
	case L_NONE:
	default:
		break;
	}

	if (g->mtype!=M_NONE) {
		if (g->mtype==M_DOT) {
			for (int i=0;i<n;i++) {
				if (curve[i].x>g->pxmin && curve[i].x<g->pxmax && curve[i].y>g->pymin && curve[i].y<g->pymax) {
					lcd_draw_point(curve[i].x,curve[i].y,c);
				}
			}
		} else {
			for (int i=0;i<n;i++) {
				if (curve[i].x>g->pxmin && curve[i].x<g->pxmax && curve[i].y>g->pymin && curve[i].y<g->pymax) {
					switch (g->mtype) {
					case M_CROSS: {
						lcd_draw_line(curve[i].x-g->msize/2,curve[i].y-g->msize/2,curve[i].x+g->msize/2,curve[i].y+g->msize/2,c);
						lcd_draw_line(curve[i].x-g->msize/2,curve[i].y+g->msize/2,curve[i].x+g->msize/2,curve[i].y-g->msize/2,c);
						break;
					}
					case M_PLUS: {
						lcd_draw_line(curve[i].x-g->msize/2,curve[i].y,curve[i].x+g->msize/2,curve[i].y,c);
						lcd_draw_line(curve[i].x,curve[i].y+g->msize/2,curve[i].x,curve[i].y-g->msize/2,c);
						break;
					}
					case M_STAR: {
						int d1=g->msize/2*239/338;			// d->msize/2*0.707
						int d2=(g->msize/2+1)*239/338;		// d->msize/2*0.707
						lcd_draw_line(curve[i].x-g->msize/2,curve[i].y,curve[i].x+g->msize/2,curve[i].y,c);
						lcd_draw_line(curve[i].x,curve[i].y+g->msize/2,curve[i].x,curve[i].y+g->msize/2,c);
						lcd_draw_line(curve[i].x-d1,curve[i].y-d1,curve[i].x+d2,curve[i].y+d2,c);
						lcd_draw_line(curve[i].x-d1,curve[i].y+d1,curve[i].x+d2,curve[i].y-d2,c);
						break;
					}
					case M_SQUARE:
						lcd_draw_rect(curve[i].x-g->msize/2,curve[i].y-g->msize/2,g->msize,g->msize,c);
						break;
					case M_FSQUARE:
						lcd_fill_rect(curve[i].x-g->msize/2,curve[i].y-g->msize/2,g->msize,g->msize,c);
						break;
					case M_DOT:
					case M_FCIRCLE:
					case M_CIRCLE:
						lcd_draw_circle(curve[i].x,curve[i].y,g->msize/2,c);
						break;
					case M_DIAMOND:
					case M_FDIAMOND:
						lcd_draw_line(curve[i].x-g->msize/2,curve[i].y,curve[i].x,curve[i].y-g->msize/2,c);
						lcd_draw_line(curve[i].x,curve[i].y-g->msize/2,curve[i].x+g->msize/2,curve[i].y,c);
						lcd_draw_line(curve[i].x+g->msize/2,curve[i].y,curve[i].x,curve[i].y+g->msize/2,c);
						lcd_draw_line(curve[i].x,curve[i].y+g->msize/2,curve[i].x-g->msize/2,curve[i].y,c);
						break;
					case M_ARROW:
						lcd_draw_line(curve[i].x,curve[i].y,curve[i].x+g->msize*13/38,curve[i].y-g->msize,c);
						lcd_draw_line(curve[i].x+g->msize*13/38,curve[i].y-g->msize,curve[i].x-g->msize*13/38,curve[i].y-g->msize,c);
						lcd_draw_line(curve[i].x-g->msize*13/38,curve[i].y-g->msize,curve[i].x,curve[i].y,c);
						break;
					case M_TRIANGLE:
					case M_FTRIANGLE:
						lcd_draw_line(curve[i].x,curve[i].y+g->msize/2,curve[i].x+g->msize/2,curve[i].y-g->msize/2,c);
						lcd_draw_line(curve[i].x+g->msize/2,curve[i].y-g->msize/2,curve[i].x-g->msize/2,curve[i].y-g->msize/2,c);
						lcd_draw_line(curve[i].x-g->msize/2,curve[i].y-g->msize/2,curve[i].x,curve[i].y+g->msize/2,c);
						break;
					default:
						break;
					}
				}
			}
		}
	}
}

static int g_draw_plot(Graph *g, vect_t *hdx, vect_t *hdy)
{
	real *x=hdx->data, *y=hdy->data;
	int rx=1, ry=1, cx=hdx->n, cy=hdy->n;
	
	// no more than MAXPOINTS allowed
	if (cx>MAXPOINTS) return 0;
	
	if (g->flags & G_AXIS) {
		lcd_draw_line(g->pxmin,g->pyorg,g->pxmax,g->pyorg,gridcolor);
		lcd_draw_line(g->pxorg,g->pymin,g->pxorg,g->pymax,gridcolor);
	}
	if (g->flags & G_FRAME) {
		lcd_draw_rect(g->pxmin,g->pymin,g->pxmax-g->pxmin+1,g->pymax-g->pymin+1,BLACK);
	}

	// clip frame
	lcd_clip(g->pxmin,g->pymin,g->pxmax,g->pymax);
	
	// precalculate the x value for the first line
	if (!(g->flags & G_XLOG)) {
		for (int i=0 ; i<cx ; i++) {
			shdata[i].x = (short)((real)g->pxmin + (x[i] - g->xmin) * g->xfactor);
		}
	} else {
		for (int i=0 ; i<cx ; i++) {
			shdata[i].x = (short)((real)g->pxmin + log10(x[i] / g->xmin) * g->xfactor);
		}
	}
	
	if (!(g->flags & G_YLOG)) {
		for (int k=0; k<ry; k++) {
			// if the x matrix has several lines, process x matrix 1 line at a time
			if (k && rx>1) {
				x+=cx;
				if (!(g->flags & G_XLOG)) {
					for (int i=0 ; i<cx ; i++) {
						shdata[i].x = (short)((real)g->pxmin + (x[i] - g->xmin) * g->xfactor);
					}
				} else {
					for (int i=0 ; i<cx ; i++) {
						shdata[i].x = (short)((real)g->pxmin + log10(x[i] / g->xmin) * g->xfactor);
					}
				}
			}
			// process y matrix 1 line at a time
			for (int i=0 ; i<cx ; i++) {
				if (y[i]>g->ymax)			/* avoid int16 overflow */
					shdata[i].y = (short)(g->pymin - 5);
				else if (y[i]<g->ymin)		/* avoid int16 overflow */
					shdata[i].y = (short)(g->pymax + 5);
				else						/* calculate standard case */
					shdata[i].y = (short)((real)g->pymin + (g->ymax - y[i]) * g->yfactor);
			}
			g_draw_path2d(g,shdata,cx,gcolors[g->color]);
			y+=cy;
			if (g->flags & G_AUTOCOLOR) {
				g->color=(g->color+1) % MAX_COLORS;
			}
			if (g->mtype!=M_NONE)
				g->mtype=(g->mtype+1) % (M_NONE-1);
			
//			if (sys_test_key()==escape) break;
		}
	} else {
		// G_YLOG
		for (int k=0; k<ry; k++) {
			// if the x matrix has several lines, process x matrix 1 line at a time
			if (k && rx>1) {
				x+=cx;
				if (!(g->flags & G_XLOG)) {
					for (int i=0 ; i<cx ; i++) {
						shdata[i].x = (short)((real)g->pxmin + (x[i] - g->xmin) * g->xfactor);
					}
				} else {
					for (int i=0 ; i<cx ; i++) {
						shdata[i].x = (short)((real)g->pxmin + log10(x[i] / g->xmin) * g->xfactor);
					}
				}
			}
			for (int i=0 ; i<cx ; i++) {
				shdata[i].y = (short)((real)g->pymax - log10(y[i] /g->ymin) * g->yfactor);
			}
			g_draw_path2d(g,shdata,cx,gcolors[g->color]);
			y+=cy;
			if (g->flags & G_AUTOCOLOR) {
				g->color=(g->color+1) % MAX_COLORS;
			}
			if (g->mtype!=M_NONE) {
				g->mtype=(g->mtype+1) % (M_NONE-1);
			}
			
//			if (sys_test_key()==escape) break;
		}
	}
	lcd_unclip();
	
	return ry;
}

void graphic_mode (void)
/***** graphic_mode
 * switch to graphic mode
 *****/
{
}

void gsubplot(int r, int c, int i);

static void gupdate(Graph *g)
{
	// update g->pxorg and g->pyorg
	if (!(g->flags & G_XLOG)) {
		g->xfactor = (real)(g->pxmax - g->pxmin) / (g->xmax - g->xmin);
		if (g->xorg>g->xmin && g->xorg<g->xmax) {
			g->pxorg = (int)((real)g->pxmin + (g->xorg - g->xmin)*g->xfactor);
		} else if (g->xorg<=g->xmin) {
			g->pxorg = g->pxmin;
		} else {
			g->pxorg = g->pxmax;
		}
	} else {
		g->xfactor = (real)(g->pxmax - g->pxmin) / log10(g->xmax / g->xmin);
		if (g->xorg>g->xmin && g->xorg<g->xmax) {
			g->pxorg = (int)((real)g->pxmin + log10(g->xorg / g->xmin)*g->xfactor);
		} else if (g->xorg<=g->xmin) {
			g->pxorg = g->pxmin;
		} else {
			g->pxorg = g->pxmax;
		}
	}
	if (!(g->flags & G_YLOG)) {
		g->yfactor = (real)(g->pymax - g->pymin) / (g->ymax - g->ymin);
		if (g->yorg>g->ymin && g->yorg<g->ymax) {
			g->pyorg = (int)((real)g->pymin + (g->ymax - g->yorg) * g->yfactor);
		} else if (g->yorg<=g->ymin) {
			g->pyorg = g->pymax;
		} else {
			g->pyorg = g->pymin;
		}
	} else {
		g->yfactor = (real)(g->pymax - g->pymin) / log10(g->ymax / g->ymin);
		if (g->yorg>g->ymin && g->yorg<g->ymax) {
			g->pyorg = (int)((real)g->pymax - log10(g->yorg /g->ymin) * g->yfactor);
		} else if (g->yorg<=g->ymin) {
			g->pyorg = g->pymax;
		} else {
			g->pyorg = g->pymin;
		}
	}
}

void ginit(void)
{
	gw.pch=fixed12.height;
	gw.pcw=fixed12.width;

	gw.cur=0;
	gw.n=1;
	gw.rows=1;
	gw.cols=1;
	
	gsubplot(1,1,1);
}

/* subplot callback
 * r:nb of rows, c:nb of columns, i: index of the current graph
 */
void gsubplot(int r, int c, int i)
{
	if (i==1) {
		lcd_clear_screen(WHITE);
		// setup new layout
		gw.rows=r; gw.cols=c; gw.n=r*c;
		int w=lcd_get_display_width();
		int h=lcd_get_display_height()-3*gw.pch;
		for (int k=0; k<gw.n; k++) {
			Graph *g=gw.graph+k;
			g->pxmin=((k)%c)*w/c+6*gw.pcw;
			g->pxmax=(((k)%c)+1)*w/c-2*gw.pcw;
			g->pymin=((k)/c)*h/r+3*gw.pch/2;
			g->pymax=(((k)/c)+1)*h/r-3*gw.pch/2;
			g->xorg=0.0; g->yorg=0.0;
			g->xmin=-1.0; g->xmax=1.0; g->ymin=-1.0; g->ymax=1.0;
			g->flags=G_WORLDUNSET|G_AXISUNSET|G_AUTOSCALE;
			g->flags|=(M_NONE<<20)|(1<<28);
			g->color=0;
			g->ltype=L_SOLID;
			g->lwidth=1;
			g->mtype=M_NONE;
		}
	} else if (gw.rows!=r || gw.cols!=c) {
		// error(calc,"Plot layout changed, but index not 1");
	}
	gw.cur=i-1;
	Graph *g=gw.graph+gw.cur;
	int w=lcd_get_display_width();
	int h=lcd_get_display_height()-3*gw.pch;
	// subplot may have been called before, so cleanup if it has.
	g->pxmin=((i-1)%c)*w/c+6*gw.pcw;
	g->pxmax=(((i-1)%c)+1)*w/c-2*gw.pcw;
	g->pymin=((i-1)/c)*h/r+3*gw.pch/2;
	g->pymax=(((i-1)/c)+1)*h/r-3*gw.pch/2;
	g->xorg=0.0; g->yorg=0.0;
	g->xmin=-1.0; g->xmax=1.0; g->ymin=-1.0; g->ymax=1.0;
	g->flags=G_WORLDUNSET|G_AXISUNSET|G_AUTOSCALE;
	g->flags|=(M_NONE<<20)|(1<<28);
	g->color=0;
	g->ltype=L_SOLID;
	g->lwidth=1;
	g->mtype=M_NONE;
}

/* setplot callback:
 * setup the limits of the plot (xmin,xmax,ymin,ymax), and flags.
 */
void gsetplot(real xmin, real xmax, real ymin, real ymax, unsigned long flags, unsigned long mask)
{
	if (gw.n) {
		Graph *g=gw.graph+gw.cur;
		// update flags
		g->flags=(g->flags & ~mask) | flags;
		if (mask & G_LTYPE_MSK) g->ltype=(flags & G_LTYPE_MSK)>>16;
		if (mask & G_MTYPE_MSK) g->mtype=(flags & G_MTYPE_MSK)>>20;
		if (mask & G_COLOR_MSK) g->color=(flags & G_COLOR_MSK)>>24;
		if (mask & G_LWIDTH_MSK) g->lwidth=(flags & G_LWIDTH_MSK)>>28;
		
		if (g->xmin!=xmin || g->xmax!=xmax || g->ymin!=ymin || g->ymax!=ymax) {
			if (g->flags & G_WORLDUNSET) {
				g->xmin=xmin; g->xmax=xmax; g->ymin=ymin; g->ymax=ymax;
				g->flags &= ~G_WORLDUNSET;
			} else if (g->flags & G_AUTOSCALE) {
				g->xmin=(xmin<g->xmin) ? xmin : g->xmin;
				g->xmax=(xmax>g->xmax) ? xmax : g->xmax;
				g->ymin=(ymin<g->ymin) ? ymin : g->ymin;
				g->ymax=(ymax>g->ymax) ? ymax : g->ymax;
			} else {
				g->xmin=xmin; g->xmax=xmax; g->ymin=ymin; g->ymax=ymax;
			}
		}
		
		if ((g->flags & G_XLOG) && g->xorg<=0.0) g->xorg=1.0;
		if ((g->flags & G_YLOG) && g->yorg<=0.0) g->yorg=1.0;
		gupdate(g);
		if (g->flags & G_AXIS) {
			lcd_draw_line(g->pxmin,g->pyorg,g->pxmax,g->pyorg,gridcolor);
			lcd_draw_line(g->pxorg,g->pymin,g->pxorg,g->pymax,gridcolor);
		}
		if (g->flags & G_FRAME) {
			lcd_draw_rect(g->pxmin,g->pymin,g->pxmax-g->pxmin+1,g->pymax-g->pymin+1,BLACK);
		}
	}
}

/* getplotlimits callback:
 * get the limits of the plot (xmin,xmax,ymin,ymax), and flags.
 */
void ggetplot(real *xmin, real *xmax, real *ymin, real *ymax, unsigned long *flags)
{
	if (gw.n) {
		Graph *g=gw.graph+gw.cur;
		*xmin=g->xmin; *xmax=g->xmax; *ymin=g->ymin; *ymax=g->ymax;
		*flags=g->flags;
	}
}

/* plot callback
 * plot the y=f(x) function
 */
void gplot(vect_t *hdx, vect_t *hdy)
{
	if (gw.n) {
		Graph *g=gw.graph+gw.cur;
		g->ltype  = (g->flags & G_LTYPE_MSK)>>16;
		g->lwidth = (g->flags & G_LWIDTH_MSK)>>28;
		g->msize     = 6;
		int r=g_draw_plot(g,hdx,hdy);
		// to allow color rotation for each plot
		if (g->flags & G_AUTOCOLOR) {
			g->color=(g->color+r) % MAX_COLORS;
		}
		if (((g->flags & G_MTYPE_MSK)>>20)!=M_NONE) {
			g->mtype=(g->mtype+r) % (M_NONE-1);
		}
	}
}

/* gxgrid calback
 *   setup a manual grid for X axis
 */
void gsetxgrid(vect_t *ticks, real factor, unsigned int color)
{
	if (gw.n) {
		Graph *g=gw.graph+gw.cur;
		g_xgrid(g,ticks,factor,color);
	}
}

/* gygrid calback
 *   setup a manual grid for Y axis
 */
void gsetygrid(vect_t *ticks, real factor, unsigned int color)
{
	if (gw.n) {
		Graph *g=gw.graph+gw.cur;
		g_ygrid(g,ticks,factor,color);
	}
}

/* gtext callback
 *   draw the "text" string at position [x,y] with the defined attributes
 */
void gtext (real x, real y, char *text, unsigned int align, int angle, unsigned int color)
{
	DC dc;
	short px, py;
	
	lcd_get_default_DC(&dc);
	lcd_set_alignment(&dc,align<<4);
	
	if (gw.n) {
		Graph *g=gw.graph+gw.cur;
//		if (!(g->flags & G_AXISUNSET)) {
			if (!(g->flags & G_XLOG)) {
				g->xfactor = (real)(g->pxmax - g->pxmin) / (g->xmax - g->xmin);
				px = (short)((real)g->pxmin + (x - g->xmin) * g->xfactor);
			} else {
				g->xfactor = (real)(g->pxmax - g->pxmin) / log10(g->xmax / g->xmin);
				px = (short)((real)g->pxmin + log10(x / g->xmin) * g->xfactor);
			}
			if (!(g->flags & G_YLOG)) {
				g->yfactor = (real)(g->pymax - g->pymin) / (g->ymax - g->ymin);
				py = (short)((real)g->pymin + (g->ymax - y) * g->yfactor);
			} else {
				g->yfactor = (real)(g->pymax - g->pymin) / log10(g->ymax / g->ymin);
				py = (short)((real)g->pymax - log10(y /g->ymin) * g->yfactor);
			}
			
			lcd_draw_string(&dc,px,py,text);
//		}
	}
}

/* glabel callback
 *   draw the "text" string as a standard label according to second parameter
 */
void glabel(const char *text, unsigned int type)
{
	DC dc;
	
	lcd_get_default_DC(&dc);
	lcd_set_alignment(&dc,ALIGN_S);
	
	if (type & G_TITLE) {
		lcd_draw_string(&dc,lcd_get_display_width()/2,2,text);
	}
	if (gw.n) {
		Graph *g=gw.graph+gw.cur;
		if (type & G_XLABEL) {
			lcd_draw_string(&dc,(g->pxmin+g->pxmax)/2,g->pymax+2*gw.pch,text);
		}
		if (type & G_YLABEL) {
			lcd_set_direction(&dc,DIR_VERTICAL_INV);
			lcd_draw_string(&dc,g->pxmin-6*gw.pcw,(g->pymin+g->pymax)/2,text);
			lcd_set_direction(&dc,DIR_HORIZONTAL);
		}
	}

}

void gsetcolor(int index)
{
	unsigned long flags, mask;
	real *m,xmin,xmax,ymin,ymax;

	ggetplot(&xmin,&xmax,&ymin,&ymax,&flags);
	flags = (flags & ~(G_COLOR_MSK)) | (index<<24);
	mask = G_COLOR_MSK;
	gsetplot(xmin,xmax,ymin,ymax,flags,mask);
}

void gsetlinetype(line_t lt)
{
	unsigned long flags, mask;
	real *m,xmin,xmax,ymin,ymax;

	ggetplot(&xmin,&xmax,&ymin,&ymax,&flags);
	flags = (flags & ~(G_LTYPE_MSK)) | (lt<<16);
	mask = G_LTYPE_MSK;
	gsetplot(xmin,xmax,ymin,ymax,flags,mask);
}

void gsetmarkertype(marker_t mt)
{
	unsigned long flags, mask;
	real *m,xmin,xmax,ymin,ymax;

	ggetplot(&xmin,&xmax,&ymin,&ymax,&flags);
	flags = (flags & ~(G_MTYPE_MSK)) | (mt<<20);
	mask = G_MTYPE_MSK;
	gsetplot(xmin,xmax,ymin,ymax,flags,mask);
}

void gsetgraph(int index)
{
	if (index>=gw.n) return;	// bad value
	
	gw.cur=index;
}

/* clear the current graph content */
void gclear (void)
{
	if (gw.n) {
		Graph *g=gw.graph+gw.cur;
		lcd_fill_rect(g->pxmin,g->pymin,g->pxmax-g->pxmin,g->pymax-g->pymin,WHITE);
	}
}

void mouse (int* x, int* y)
/****** mouse
	wait, until the user marked a screen point with the mouse.
	Return screen coordinates.
******/
{	*x=0; *y=0;
}

void getpixel (real *x, real *y)
/***** Compute the size of pixel in screen coordinates.
******/
{	*x=1;
	*y=1;
}

void gflush (void)
/***** Flush out remaining graphic commands (for multitasking).
This serves to synchronize the graphics on multitasking systems.
******/
{
}
