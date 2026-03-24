/*
 * (C) 2023, E Boucharé
 */
#include "lcd.h"
#include "lcd_private.h"

#include "lib/spi.h"
#include "lib/io.h"

Display disp = {
	.orientation = PORTRAIT,
	.width = LCD_WIDTH,
	.height = LCD_HEIGHT,
	.ts_cal = {320,3800,220,3800},
	.dc = {{0,0,LCD_WIDTH,LCD_HEIGHT}, WHITE, BLACK, &fixed12, DIR_HORIZONTAL|ALIGN_SE, 1}
};

/* switch config to use one of LCD_DPY, LCD_TS, LCD_SD */
void lcd_switch_to(uint32_t cfg)
{
	static int lcd_cur = -1;
	
	if (lcd_cur == (int)cfg) return;
	
	switch (lcd_cur) {
	case LCD_DPY:
		LCD_DPY_DIS;
		break;
	case LCD_TS:
		LCD_TS_DIS;
		break;
#ifdef LCD_SD_USED
	case LCD_SD:
		LCD_SD_DIS;
		break;
#endif
	default:
		break;
	}
	
	switch (cfg) {
	case LCD_DPY:
		spi_master_init(LCD_SPI, LCD_DPY_CLOCK, LCD_DPY_CFG);
		lcd_cur = LCD_DPY;
		break;
	case LCD_TS:
		spi_master_init(LCD_SPI, LCD_TS_CLOCK, LCD_TS_CFG);
		lcd_cur = LCD_TS;
		break;
#ifdef LCD_SD_USED
	case LCD_SD:
		spi_master_init(LCD_SPI, LCD_SD_CLOCK, LCD_SD_CFG);
		lcd_cur = LCD_SD;
		break;
#endif
	default:
		break;
	}
}

/* lcd board initialisation */
void lcd_init(void)
{
	// init display
	lcd_dpy_init();
	LCD_DPY_DIS;
	
#ifdef LCD_SD_USED
	// init touchscreen
	lcd_ts_init();
	LCD_TS_DIS;
	
	// init SD card
	lcd_sd_init();
	LCD_SD_DIS;
#endif
}
