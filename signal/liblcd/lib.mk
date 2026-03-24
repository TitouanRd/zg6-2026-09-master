
UDEFS += -DUSE_LCD -DLCD_ILI9341 -DSTMPE610 -DFLOAT32

SRC += liblcd/lcd.c liblcd/lcd_dpy.c \
       liblcd/lcd_ts.c liblcd/lcd_sd.c \
       liblcd/fonts/fixed8.c liblcd/fonts/fixed12.c \
       liblcd/vect.c liblcd/cplx.c liblcd/graph.c
