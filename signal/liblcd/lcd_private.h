#ifndef _LCD_COMMON_H_
#define _LCD_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif 

#include "include/board.h"
#include "lib/io.h"
#include "lib/spi.h"

#define LCD_SD_USED

#define LCD_SPI			_SPI1

/* clock frequency and SPI config for DPY, TS and SD devices */
#define LCD_DPY_CLOCK	42000000
#define LCD_DPY_CFG		SPI_MODE3
#define LCD_TS_CLOCK	1000000
#define LCD_TS_CFG		SPI_MODE3
#ifdef LCD_SD_USED
#define LCD_SD_CLOCK	42000000
#define LCD_SD_CFG		SPI_MODE3
#endif

/* GPIO signals */
#define LCD_DPY_EN		io_clear(LCD_DPY_CS_PORT, LCD_DPY_CS_PINS)
#define LCD_DPY_DIS		io_set(LCD_DPY_CS_PORT, LCD_DPY_CS_PINS)

#define LCD_DPY_CMD		io_clear(LCD_DPY_DC_PORT, LCD_DPY_DC_PINS)
#define LCD_DPY_DATA	io_set(LCD_DPY_DC_PORT, LCD_DPY_DC_PINS)

#define LCD_DPY_BL_ON	io_set(LCD_DPY_BL_PORT, LCD_DPY_BL_PINS)
#define LCD_DPY_BL_OFF	io_clear(LCD_DPY_BL_PORT, LCD_DPY_BL_PINS)

#define LCD_TS_EN		io_clear(LCD_TS_CS_PORT, LCD_TS_CS_PINS)
#define LCD_TS_DIS		io_set(LCD_TS_CS_PORT, LCD_TS_CS_PINS)

#ifdef LCD_SD_USED
#define LCD_SD_EN		io_clear(LCD_SD_CS_PORT, LCD_SD_CS_PINS)
#define LCD_SD_DIS		io_set(LCD_SD_CS_PORT, LCD_SD_CS_PINS)
#endif

/* global Display Context */
typedef struct _Display {
	Orientation		orientation;	/* screen orientation */
	uint16_t		width;			/* display width */
	uint16_t		height;			/* display height */
	uint16_t		ts_cal[4];		/* touchscreen calibration parameters */
	DC				dc;				/* drawing context */
	
} Display;	/* Display Context */

extern Display disp;

void lcd_dpy_init(void);
void lcd_ts_init(void);
void lcd_sd_init();

#ifdef __cplusplus
}
#endif
#endif
