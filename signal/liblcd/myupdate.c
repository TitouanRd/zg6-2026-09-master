#include <stdint.h>
#include "graph.h"

#ifdef VERSION_B
void update_signal(vect_t *x, vect_t *t, uint16_t *data)
{
	real *mx=x->data;
	gsetgraph(0);
	gsetcolor(15);	// erase the old plot
	gplot(t,x);
	for (int i=0; i<x->n; i++) {
		mx[i]=(float)data[i]*3.3f/4096.0f;
	}
	gsetcolor(2);	// draw the new one
	gplot(t,x);
}
#endif

#ifdef VERSION_A
void update_spectrum(vect_t *x, vect_t *X, vect_t *freq, real fs)
{
	real *mX = X->data;
	
	gsetgraph(1);
	gsetcolor(15);	// erase the old plot
	gplot(freq,X);
		
	// calculate DTFT for a range of frequencies
	for (int i=0; i<freq->n; ++i) {
		mX[i]=c_abs(dtft(x,freq->data[i],fs))/(float)x->n;
	}
		
	gsetcolor(2);	// draw the new one
	gplot(freq,X);
}
#endif
