#ifndef _UTILS_H_
#define _UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif 

#ifndef MIN
#define MIN(a,b)	((a)<(b) ? (a) : (b))
#endif

/****************************************************************
 * convert numbers (int and float) to string
 *   FLAG_USE_ALIGN_RIGHT: align number left 
 ****************************************************************/
#define FLAG_USE_ZEROS				(1<<0)
#define FLAG_FP_FIXED_FORMAT		(1<<5)
#define FLAG_FP_EXP_FORMAT			(1<<6)
#define FLAG_FP_ENG_FORMAT			(1<<7)

// s should be S_MAX_SIZE+1 size, an size<=S_MAX_SIZE
#define S_MAX_SIZE					63

/* num2str
 *   convert a number 'value' in base 'base' to the string s (the string
 *   variable must be large enough)
 *   size : 0 (left aligned number) or minimal number of chars to display 
 *          the number on (number may be right aligned)
 *   flag : when right aligned, complete the 'size' chars with ' ' or '0' (FLAG_USE_ZEROS)
 *
 *   returns number of chars used in s.
 */
int num2str(char *s, int value, unsigned int base, int size, unsigned int flag);

/* float2str
 *   convert 'value' using at most 'precision' digits, left aligned ('size'=0),
 *   or right aligned (on, at most, 'size' chars).
 *   The 'flag' specifies the use of engineering format (sub-units with 
 *   FLAG_FP_ENG_FORMAT) or, by default, the decimal point format or 
 *   exponential format, depending on the value.
 */ 
int float2str(char *s, double value, int precision, int size, unsigned int flag);

/****************************************************************
 * convert strings to numbers (int and float)
 ****************************************************************/
int str2num(char *s, unsigned base);


#ifdef __cplusplus
}
#endif
#endif
