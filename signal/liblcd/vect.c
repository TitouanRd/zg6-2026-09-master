#include <stdlib.h>
#include <limits.h>

#include "graph.h"

real* vect(vect_t *v, int n) {
	if (!v) return false;
	
	v->data = (real*)malloc(n*sizeof(real));
	if (v->data) {
		v->n=n;
		return v->data;
	}
	return NULL;
}

void vect_del(vect_t *v)
{
	if (v && v->data) {
		free(v->data);
		v->data=NULL;
		v->n=0;
	}
}

bool vectorize(vect_t *v, real vinit, real vend, real vstep)
{
	int count;
	if (vstep==0.0) return false;
	if (1.0+fabs((vend-vinit)/vstep)*(1+EPSILON)>INT_MAX) return false;
	if ((vend>vinit && vstep<0) || (vend<vinit && vstep>0)) return false;
	count=1+(int)(floor(fabs(vend-vinit)/fabs(vstep)*(1+EPSILON)));
	if (v->data && v->n!=count) {
		v->data=(real*)realloc(v->data,count*sizeof(real));
	} else if (!v->data) {
		vect(v,count);
	}
	if (!v->data) return false;
	// vectorize
	for (real *m=v->data; count>0; count--) {
		*m++=vinit; vinit+=vstep;
	}
	return true;
}

bool vectorize_func(vect_t *x, vect_t *y, func_t f)
{
	real *m;
	
	if (!(x->data && f)) return false;
	
	if (y->data && y->n!=x->n) {
		m=y->data=(real*)realloc(y->data,x->n*sizeof(real));
	} else if (!y->data) {
		m=vect(y,x->n);
	} else {
		m=y->data;
	}
	if (!m) return false;
	
	for (int i=0; i<x->n; i++) {
		m[i]=f(x->data[i]);
	}

	return true;
}
