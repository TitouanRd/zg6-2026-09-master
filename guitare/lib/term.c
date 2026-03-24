// Terminal function

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include <math.h>

#include "term.h"
#include "lib/util.h"

/****************************************************************************
 *  Low level functions
 ***************************************************************************/
char term_in(void);
void term_out(char c);

#include "lib/uart.h"

#if USE_UART_RINGBUF
#include "lib/ringbuf.h"

//#define WITH_RX_RINGBUF
//#define WITH_TX_RINGBUF

#ifndef WITH_RX_RINGBUF
/**** Rx part without ring buffer *******************************/
char term_in(void)
{
	return uart_getc(_USART2);
}

static OnUartRx rx_cb=NULL;

#else
/**** Rx part with ring buffer **********************************/
static RGBF rx_rgbf=RGBF_INIT;

static void on_rx_cb(char c)
{
	/* !!! WARNING: this function must not block !!! */
	
	/* if there is room to store the new received data in the 
	 * ring buffer, just do it.
	 */
	if (!rgbf_is_full(&rx_rgbf)) {
		rgbf_send_data(&rx_rgbf, c);
	}
	/* else received data is lost!!! (unlikely) */
}

char term_in(void)
{
	/* wait for new available data */
	while (rgbf_is_empty(&rx_rgbf)) {}
	/* read the abvailable data */
	return rgbf_get_data(&rx_rgbf);
}

static OnUartRx rx_cb=on_rx_cb;
#endif

#ifndef WITH_TX_RINGBUF
/**** Tx part without ring buffer *******************************/
void term_out(char c)
{
	uart_putc(_USART2,c);
}

static OnUartTx tx_cb=NULL;

#else
/**** Tx part with ring buffer **********************************/
static RGBF tx_rgbf=RGBF_INIT;

static bool on_tx_cb(char *c)
{
	if (!rgbf_is_empty(&tx_rgbf)) {
		*c = rgbf_get_data(&tx_rgbf);
		return true;
	}
	return false;		// nothing more to send
}

void term_out(char c)
{
	/* wait while transmit buffer is full */
	while (rgbf_is_full(&tx_rgbf)) {}
	/* send data to the ring buffer */
	rgbf_send_data(&tx_rgbf, c);
	/* allow IRQ when the Transmit Data Register is empty */
	uart_enable_irq(_USART2, UART_TXE_IRQ);
}

static OnUartTx tx_cb=on_tx_cb;
#endif
#else
void uart_putc(USART_t *u, char c);

char term_in(void)
{
	return 0;
}

void term_out(char c)
{
	uart_putc(_USART2,c);
}
#endif

/****************************************************************************
 *  Util function
 ***************************************************************************/
// Local variables
static unsigned int term_num_rows, term_num_cols;
static unsigned int term_cx, term_cy;
static unsigned int fg_color, bg_color;

// Helper function: send the requested string to the terminal
static void term_ansi( const char* fmt, ... )
{
	va_list ap;
	char s[20], *p;
	
	term_out('\x1B');
	term_out('[');
	va_start(ap,fmt);
	while (*fmt!='\0') {
		if (*fmt=='%') {
			switch (*++fmt) {
			case 'u':
				num2str(s, va_arg(ap, unsigned int), 10, 0, 0);
				p=s;
				while (*p) term_out(*p++);
				break;
			}
		} else term_out(*fmt);
		fmt++;
	}
	va_end(ap);
}

/****************************************************************************
 *  terminal functions (public API)
 ***************************************************************************/

//-----------------------------
//  Screen handling functions 
//-----------------------------

// Clear the screen
void term_clrscr(void)
{
	term_ansi( "2J" );
	term_ansi( "H" );
	term_cx = term_cy = 1;
}

// Clear to end of line
void term_clreol(void)
{
	term_ansi( "K" );
}

// Set the current color on the 
void term_color(unsigned int backcolor, unsigned int forecolor, unsigned int effect)
{
	if (backcolor) backcolor+=10;	/* 40 - 47 */
	if (fg_color!=forecolor || bg_color!=backcolor) {
		if (backcolor) {
			term_ansi("%u;%u;%um", effect, forecolor, backcolor);
		} else {
			term_ansi("%u;%um", effect, forecolor);
		}
		fg_color = forecolor; bg_color=backcolor;
	} else {
		term_puts("\e[0m");
	}
}

//-----------------------------
//  Cursor handling functions 
//-----------------------------

// Move cursor to (x, y)
void term_gotoxy(unsigned x, unsigned y)
{
	term_ansi("%u;%uH", y, x);
	term_cx = x;
	term_cy = y;
}

// Move cursor up "delta" lines
void term_up(unsigned delta)
{
	term_ansi("%uA",delta);
	term_cy -= delta;
}

// Move cursor down "delta" lines
void term_down(unsigned delta)
{
	term_ansi("%uB",delta);
	term_cy += delta;
}

// Move cursor right "delta" chars
void term_right(unsigned delta)
{
	if (delta) {
		term_ansi("%uC", delta);
		term_cx += delta;
	}
}

// Move cursor left "delta" chars
void term_left(unsigned delta)
{
	if (delta) {
		term_ansi("%uD", delta);
		term_cx -= delta;
	}
}

// Return the number of terminal rows
unsigned term_get_rows(void)
{
	return term_num_rows;
}

// Return the number of terminal columns
unsigned term_get_cols(void)
{
	return term_num_cols;
}

// Return the cursor "x" position
unsigned term_get_cx(void)
{
	return term_cx;
}

// Return the cursor "y" position
unsigned term_get_cy(void)
{
	return term_cy;
}

void term_show_cursor(bool show)
{
	if (show) {
		term_ansi("?25h");
	} else {
		term_ansi("?25l");
	}
}
//-----------------------------
//  I/O handling functions (terminal level)
//-----------------------------

// Return a char read from the terminal
int term_getc(void)
{
	int c=term_in();
	if(isprint(c)) return c;
	else if (c == 0x04) return KC_EOT;	// ctrl D
	else if( c == 0x1B ) {		// escape sequence
		c=term_in();
		if(c=='[') {
			c = term_in();
			switch( c ) {
			case 0x31:
				c=term_in();
				switch (c) {
				case '5':
					if (term_in()=='~') return KC_F5;
					else return KC_UNKNOWN;
				case '7':
					if (term_in()=='~') return KC_F6;
					else return KC_UNKNOWN;
				case '8':
					if (term_in()=='~') return KC_F7;
					else return KC_UNKNOWN;
				case '9':
					if (term_in()=='~') return KC_F8;
					else return KC_UNKNOWN;
				default:
					return KC_UNKNOWN;
				}
			case 0x32:
				c=term_in();
				switch (c) {
				case 0x30:
					if (term_in()=='~') return KC_F9;
					else return KC_UNKNOWN;
				case 0x31:
					if (term_in()=='~') return KC_F10;
					else return KC_UNKNOWN;
				case 0x33:
					if (term_in()=='~') return KC_F11;
					else return KC_UNKNOWN;
				case 0x34:
					if (term_in()=='~') return KC_F12;
					else return KC_UNKNOWN;
				case '~':
					return KC_INSERT;
				default:
					return KC_UNKNOWN;
				}
			case 0x33:
				if (term_in()=='~')	return KC_SUPPR;
				else return KC_UNKNOWN;
			case 0x36:
				if (term_in()=='~') return KC_PAGEDOWN;
				else return KC_UNKNOWN;
			case 0x35:
				if (term_in()=='~') return KC_PAGEUP;
				else return KC_UNKNOWN;
			case 0x41:
				return KC_UP;
			case 0x42:
				return KC_DOWN;
			case 0x43:
				return KC_RIGHT;
			case 0x44:
				return KC_LEFT;               
			case 0x05:
			case 0x46:
				return KC_END;
			case 0x02:
			case 0x48:
				return KC_HOME;
			}
		} else if (c==0x4f) {
			c=term_in();
			switch(c) {
			case 0x05:
			case 0x46:
				return KC_END;
			case 0x02:
			case 0x48:
				return KC_HOME;
			case 0x50:
				return KC_F1;
			case 0x51:
				return KC_F2;
			case 0x52:
				return KC_F3;
			case 0x53:
				return KC_F4;
			default:
				return KC_UNKNOWN;
			}
		} else return KC_ESC;
	} else {
		switch(c) {
		case 0x0D:
		case 0x0A:		// CR/LF sequence, read the second char (LF) if applicable
			return KC_ENTER;
		case 0x09:
			return KC_TAB;
		case 0x7F:
		case 0x08:
			return KC_BACKSPACE;
		default:
			return KC_UNKNOWN;
		}
	}
	return KC_UNKNOWN;
}

// Write a character to the terminal
void term_putc(char ch)
{
	if (ch == '\n') {
		if (term_cy < term_num_rows)
			term_cy ++;
		term_cx = 1;
	} else {
		term_cx++;
	}
	term_out( ch );
}

// Write a string to the terminal
void term_puts(const char* str)
{
	while (*str) {
		term_putc( *str++ );
	}
}

void term_printf(const char* fmt, ...)
{
	va_list			ap;
	int				precision=6;		
	double			f;
	char			*p;
	char			ch;
	unsigned long	ul;
	unsigned long	size;
	unsigned int	flag;
	char			s[S_MAX_SIZE+1];
	
	va_start(ap, fmt);
	while (*fmt != '\0') {
		if (*fmt =='%') {
			size=0; flag=0;
			if (*++fmt=='0') {fmt++; flag|=FLAG_USE_ZEROS;}	// parse %04d --> sp=0
			ch=*fmt;
			if ((ch>'0') && (ch<='9')) {	// parse %4d --> size=4
				for (int i=0; (ch>='0') && (ch<='9') && i<4; i++) {
					size=10*size+ch-'0';
					ch=*++fmt;
				}
			}
			if (ch=='.') ch=*++fmt;
			if ((ch>'0') && (ch<='9')) {	// parse %4d --> size=4
				precision=0;
				for (int i=0; (ch>='0') && (ch<='9') && i<3; i++) {
					precision=10*precision+ch-'0';
					ch=*++fmt;
				}
			}
			
			switch (ch) {
				case '%':
					term_putc('%');
					break;
				case 'c':
					ch = (char)va_arg(ap, int);
					term_putc(ch);
					break;
				case 's':
					p = va_arg(ap, char *);
					term_puts(p);
					break;
				case 'd':
					ul = va_arg(ap, long);
					if ((long)ul < 0) {
						term_putc('-');
						ul = -(long)ul;
						size--;
					}
					num2str(s, ul, 10, size, flag);
					term_puts(s);
					break;
				case 'u':
					ul = va_arg(ap, unsigned int);
					num2str(s, ul, 10, size, flag);
					term_puts(s);
					break;
				case 'o':
					ul = va_arg(ap, unsigned int);
					num2str(s, ul, 8, size, flag);
					term_puts(s);
					break;
				case 'p':
					term_putc('0');
					term_putc('x');
					ul = va_arg(ap, unsigned int);
					num2str(s, ul, 16, size, flag);
					term_puts(s);
					break;
				case 'x':
					ul = va_arg(ap, unsigned int);
					num2str(s, ul, 16, size, flag);
					term_puts(s);
					break;
				case 'f':
					// value promoted to 'double' even if it was a 'float'
					f = (double)va_arg(ap, double);
					float2str(s,f,precision,size,flag|FLAG_FP_FIXED_FORMAT);
					term_puts(s);
					break;
				case 'e':
					// value promoted to 'double' even if it was a 'float'
					f = (double)va_arg(ap, double);
					float2str(s,f,precision,size,flag|FLAG_FP_ENG_FORMAT);
					term_puts(s);
					break;
				case 'g':
					// value promoted to 'double' even if it was a 'float'
					f = (double)va_arg(ap, double);
					float2str(s,f,precision,size,flag);
					term_puts(s);
					break;
				default:
				    term_putc(*fmt);
			}
		} else term_putc(*fmt);
		fmt++;
	}
	va_end(ap);
}


//-----------------------------
//  readline functions
//-----------------------------

static char* hist[HISTORY_MAX];			// table of inputs
static int  hist_last=0;				// index of the last line copied in index
static int  hist_id=0;					// index of the current line pointed to in the history

static char *buffer=NULL;
static const char *prompt=NULL;
static int  maxlen=0;
static int  pos=0;
static int  len=0;

static unsigned tmp_cx, tmp_cy;

static void term_save_pos(void)
{
	term_ansi("s");
	tmp_cx=term_cx;
	tmp_cy=term_cy;
}

static void term_restore_pos(void)
{
	term_cx=tmp_cx;
	term_cy=tmp_cy;
	term_ansi("u");
}

/*
void dbg(int x, int y, int c)
{
	term_ansi("s");
	term_gotoxy(x,y);
	term_ansi("K");term_printf("pos = %d",pos);
	term_gotoxy(x,y+1);
	term_ansi("K");term_printf("len = %d",len);
	term_gotoxy(x,y+2);
	term_ansi("K");term_printf("char = %x",c);
	term_ansi("u");
}*/

int term_readline (const char *prompt_str, char *buf, int maxlength) 
{
	int c;
	
	if (!buf) return 0;
  
	prompt=prompt_str;
	buffer=buf;
	*buffer = '\0';
	maxlen=maxlength;
	
	pos=0;
	len=0;

	term_clreol();
	if (prompt) term_puts(prompt);

	while (1) {
		c=term_getc();
  		if (c<255) {
			if (len < maxlen) {
				char *s = &buffer[pos];
				unsigned l=strlen(s);
      
				memmove(s+1, s, l+1);
				buffer[pos] = (char)c;
				term_clreol();				// erase to end of line
				pos++;
				len++;
				term_save_pos();				// save cursor pos
				term_puts(s);
				term_restore_pos();				// restore cursor pos
				term_right(1);
			}
  	
  		} else switch (c) {
  			case KC_SUPPR:
				if (buffer[pos]!='\0') {
					char * s=&buffer[pos];
					strcpy(s,s+1);
					term_clreol();			//   erase to end of line
        			len--;
					term_save_pos();        	//   save cursor pos
					term_puts(s);
					term_restore_pos();     	//   restore cursor pos
				}
				break;
			case KC_BACKSPACE:				// back space
				if (pos > 0) {
					char *s = &buffer[pos];
					strcpy(s-1, s);
			        term_putc('\b');
					term_clreol();			//   erase to end of line
					pos--;
					len--;
					term_save_pos();			//   save cursor pos
					term_puts(s-1);
					term_restore_pos();			//   restore cursor pos
				}
				break;
			case KC_HOME:					// cursor to begin of line
				if (pos>0) {
					term_left(pos);
					pos=0;
				}
				break;
			case KC_END:					// cursor to end of line
				if (pos<len) {
					term_right(len-pos);
					pos=len;
				}
				break;
			case KC_RIGHT:
				if (pos<len) {
					term_right(1);
					pos++;
				}
				break;
			case KC_LEFT:
				if (pos>0) {
					term_left(1);
					pos--;
				}
				break;
			case KC_UP:
				if (hist_id>0) {
					hist_id--;
				}
				if (hist[hist_id]) {
					term_left(pos); term_clreol();
					strcpy(buffer,hist[hist_id]);
					term_puts(buffer);
					len=pos=strlen(buffer);
				}
				break;
			case KC_DOWN:
				if (hist_id<MIN(HISTORY_MAX-1,hist_last)) {
					hist_id++;
				}
				if (hist[hist_id]) {
					term_left(pos); term_clreol();
					strcpy(buffer,hist[hist_id]);
					term_puts(buffer);
					len=pos=strlen(buffer);
				}
				break;
			case KC_ENTER:
				// save buffer if not empty, not the same as last line
				if (hist[hist_last] && !hist[hist_last][0]) {
					// an empty line, so use it
					strcpy(hist[hist_last],buffer);
				} else if (!hist[hist_last]) {
					// allocate
					hist[hist_last]=(char*)malloc(maxlength);
					strcpy(hist[hist_last],buffer);
				} else if (hist_last<HISTORY_MAX && strcmp(buffer,hist[hist_last])) {
					hist_last++;
					if (hist_last<HISTORY_MAX && !hist[hist_last]) {		// allocate
						hist[hist_last]=(char*)malloc(maxlength);
						hist[hist_last][0]='\0';
					} else {		// shift references
						char *tmp=hist[0];
						tmp[0]='\0';
						for (int i=0; i<HISTORY_MAX-1; i++) {
							hist[i]=hist[i+1];
						}
						hist_last=HISTORY_MAX-1;
						hist[hist_last]=tmp;
						hist_id= (hist_id-1>=0) ? hist_id-1 : 0;
					}
					strcpy(hist[hist_last],buffer);
				}
				return len;
			case KC_EOT:					// ctrl D
				return -1;
			default:
				break;
		}
	}
	return len;
}


//-----------------------------
// init function
//-----------------------------
void term_init(unsigned rows, unsigned cols)
{
//	uart_init(_USART2, 115200, UART_8N1, rx_cb, tx_cb);
	
	term_num_rows = rows;
	term_num_cols = cols;
	term_cx = term_cy = 1;

	for (int i=0; i<HISTORY_MAX; i++) hist[i]=NULL;
}                

