#include "graph.h"

cplx_t cplx(real r, real i)
{
	cplx_t z={r,i};
	return z;
}

cplx_t c_add(cplx_t a, cplx_t b)
{
	cplx_t z={a.r+b.r,a.i+b.i};
	return z;
}

cplx_t c_mul(cplx_t a, cplx_t b)
{
	cplx_t z={a.r*b.r-a.i*b.i,a.r*b.i+a.i*b.r};
	return z;
}

real c_abs(cplx_t a)
{
	return sqrt(a.r*a.r+a.i*a.i);
}

cplx_t c_expj(real a)
{
	cplx_t z={cos(a),sin(a)};
	return z;
}

/* Dicrete time Fourier Transform
   - for signal x(t)
   - at frequency f,
   - when using sampling frequency fs
 */
cplx_t dtft(const vect_t *x, real f, real fs)
{
	cplx_t e=c_expj(-2*(real)M_PI*f/fs), z={x->data[x->n-1],0.0};
	
	for (int i=1; i<x->n; ++i) {
		cplx_t coeff={x->data[x->n-1-i],0.0};
		z=c_add(coeff,c_mul(z,e));
	}
	return z;
}

