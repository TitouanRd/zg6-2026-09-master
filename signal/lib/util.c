#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "util.h"

#if 0
/* num2str
 *   convert a number 'number' in base 'base' to the string s (the string
 *   variable must be large enough)
 */
void num2str(char *s, unsigned int number, unsigned int base)
{
	static char  hexChars[] = "0123456789ABCDEF";

	char *p=s;
	int cnt;
	int i;
	char tmp;

	// get digits
	do {
		*s++=hexChars[number % base];
	} while (number /= base);
	*s='\0';

	// reverse string
	cnt=s-p;
	for (i=0;i<cnt/2;i++) {
		tmp=p[i]; p[i] = p[cnt-i-1]; p[cnt-i-1]=tmp;
	}
}
#endif

static const char hex_chars[] = "0123456789ABCDEF";

static const char eng_prefix[] = {
  'y', 'z', 'a', 'f', 'p', 'n', 'u', 'm', 0,
  'k', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y'
}; 

/* num2str
 *   convert a number 'value' in base 'base' to the string s (the string
 *   variable must be large enough)
 *   size : 0 (left aligned number) or minimal number of chars to display 
 *          the number on (number may be right aligned)
 *   flag : when right aligned, complete the 'size' chars with ' ' or '0' (FLAG_USE_ZEROS)
 *
 *   returns number of chars used in s.
 */
int num2str(char *s, int value, unsigned int base, int size, unsigned int flag)
{
	char *p=s;
	int cnt;
	int i;
	char tmp;

	if (value<0) {
		value=-value;
		*s++='-';
	}
	// get digits
	do {
		*s++=hex_chars[value % base];
	} while (value /= base);
	*s='\0';

	// reverse string
	cnt=s-p;
	for (i=0;i<cnt/2;i++) {
		tmp=p[i]; p[i] = p[cnt-i-1]; p[cnt-i-1]=tmp;
	}

	// add extra space, align right
	if (cnt<size) {
		for (i=cnt;i>=0;i--) p[i+size-cnt]=p[i];
		if ((flag & FLAG_USE_ZEROS)==0) tmp=' '; else tmp='0';
		for (i=0;i<size-cnt;i++) p[i]=tmp;
	}
	return size>cnt ? size : cnt;
}

/* float2str
 *   convert 'value' using at most 'precision' digits, left aligned ('size'=0),
 *   or right aligned (on, at most, 'size' chars).
 *   The 'flag' specifies the use of engineering format (sub-units with 
 *   FLAG_FP_ENG_FORMAT) or, by default, the decimal point format or 
 *   exponential format, depending on the value.
 */ 
int float2str(char *s, double value, int precision, int size, unsigned int flag)
{
	int expof10, eexpof10, eng_idx;
	int is_neg = signbit(value);
	double pof10=1.0, inc=5.0, l,val;
	int cnt;
	char *p=s;
	if (is_neg) value = -value;
	
	switch(fpclassify(value)) {
	case FP_NORMAL:
		if (is_neg) *s++='-';
		l = log10(value);
		expof10 = l>=0.0 ? (int)(l) : (int)l-1;
//		fprintf(stderr,"%d,%g\n",expof10,pof10);
		if (flag & FLAG_FP_ENG_FORMAT) {			// %e
			val = value;
			if(expof10 > 0)
				eexpof10 = (expof10/3)*3;
			else
				eexpof10 = (-expof10+3)/3*(-3); 
		 
			value *= pow(10,-eexpof10);
			if (value >= 1000.) { value /= 1000.0; eexpof10 += 3; }
			
			// find the index in the eng_prefix array
			eng_idx = 8+eexpof10/3;
			
			if (eng_idx>=0 && eng_idx<=16) {		// found a valid prefix
				eexpof10 = expof10-eexpof10;
			} else {							// no valid prefix: fall back to standard print
				value = val;
				eexpof10 = expof10;
				flag &= ~FLAG_FP_ENG_FORMAT;
			}		
		} else {									// %g
			eexpof10=expof10;
		}

		for (int i=0; i<abs(eexpof10); i++) pof10*=10.0;
		// normalize value
		value = eexpof10>=0 ? value/pof10 : value*pof10;
		// decide if we use fixed or exp format
		eexpof10 = abs(eexpof10)<=precision ? eexpof10 : 0;

//		fprintf(stderr,"eexpof10=%d, value=%g\n",eexpof10,value);
		
		// rounding according to precision
		inc=5.0;
		for (int i=0; i<precision; i++) inc/=10.0;
		value += inc;
					
		// integer part
		if (eexpof10>=0) { 
			for (int i=eexpof10; i>=0; --i) {
				int idx=(int)value;
				value=10.0*(value-(float)idx);
				*s++ = hex_chars[idx];
			}
		} else {
			*s++='0';
		}
		if (precision-eexpof10-1>0) {	// has fractionnal part
			// decimal dot
			*s++='.';
			// fractionnal part
			if (eexpof10<0) {	// leading zeros
				for (int i=-eexpof10-1; i>0; --i) *s++='0';
				for (int i=precision-abs(eexpof10); i>0; --i) {
					int idx=(int)value;
					value=10.0*(value-(float)idx);
					*s++ = hex_chars[idx];
				}
			} else {
				for (int i=precision-abs(eexpof10)-1; i>0; --i) {
					int idx=(int)value;
					value=10.0*(value-(float)idx);
					*s++ = hex_chars[idx];
				}
			}
			// cancel ending zeros of fractionnal part
			while (*(s-1)=='0') s--;
			if (*(s-1)=='.') s--;
		}
		
		// set exponent
		if (flag & FLAG_FP_ENG_FORMAT) {
			if (eng_prefix[eng_idx]!=0) *s++=eng_prefix[eng_idx];
		} else if (expof10!=eexpof10) {
			char buf[20],*n=buf;
			num2str(buf, abs(expof10), 10, 0, 0);
			*s++='e';
			*s++=expof10>=0 ? '+' : '-'; 
			while (*n) *s++=*n++;
		}
		*s='\0';
		break;
	case FP_INFINITE:
		strcpy(s, is_neg ? "-INF" : "+INF"); s+=4;
		break;
	case FP_NAN:
		strcpy(s,"NAN"); s+=3;
		break;
	case FP_SUBNORMAL:
	case FP_ZERO:
	default:
		strcpy(s, is_neg? "-0" : "0");
		break;
	}
	
	cnt=s-p;
	// add extra space, align right
	if (size>cnt) {
		for (int i=cnt;i>=0;i--) p[i+size-cnt]=p[i];
		for (int i=0;i<size-cnt;i++) p[i]=' ';
	}
	return size>cnt ? size : cnt;
}

int str2num(char *s, unsigned int base)
{
	unsigned int u=0, d, neg=1;
	char ch;
	while ((ch=*s)==' ' || ch=='\t') s++;
	if (ch=='-') {
		neg=-1; s++;
	} else if (ch=='+') {
		s++;
	}
	while ((ch=*s++)!=0) {
		if ((ch>='0') && (ch<='9')) d=ch-'0';
		else if ((base==16) && (ch>='A') && (ch<='F')) d=ch-'A'+10;
		else if ((base==16) && (ch>='a') && (ch<='f')) d=ch-'a'+10;
		else break;
		u=d+base*u;
	}
	return neg*u;
}

