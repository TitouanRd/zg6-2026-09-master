#ifndef _IO_H_
#define _IO_H_

#ifdef __cplusplus
extern "C" {
#endif 

#include "include/board.h"

/* --- GPIO pin mask definitions ---------------------------------------- */
#define PIN_0						(1 << 0)
#define PIN_1						(1 << 1)
#define PIN_2						(1 << 2)
#define PIN_3						(1 << 3)
#define PIN_4						(1 << 4)
#define PIN_5						(1 << 5)
#define PIN_6						(1 << 6)
#define PIN_7						(1 << 7)
#define PIN_8						(1 << 8)
#define PIN_9						(1 << 9)
#define PIN_10						(1 << 10)
#define PIN_11						(1 << 11)
#define PIN_12						(1 << 12)
#define PIN_13						(1 << 13)
#define PIN_14						(1 << 14)
#define PIN_15						(1 << 15)
#define PIN_ALL						0xFFFF

/* --- GPIO pin mode definitions ---------------------------------------- */
#define PIN_MODE_INPUT				(0 << 16)
#define PIN_MODE_OUTPUT				(1 << 16)
#define	PIN_MODE_ALTFUNC			(2 << 16)
#define	PIN_MODE_ANALOG				(3 << 16)

/* --- GPIO pin option definitions -------------------------------------- */
/*   none for analog pin */
#define PIN_OPT_NONE				0

/*   pull up/pull down resistor option */
#define PIN_OPT_RESISTOR_NONE		(0 << 8)
#define PIN_OPT_RESISTOR_PULLUP		(1 << 8)
#define PIN_OPT_RESISTOR_PULLDOWN	(2 << 8)

/*   push-pull/open drain output option */
#define	PIN_OPT_OUTPUT_PUSHPULL		(0 << 6)
#define PIN_OPT_OUTPUT_OPENDRAIN	(1 << 6)

/*   output speed option
 *   - LOW    ~ 4MHz max
 *   - MEDIUM ~ 25MHz max
 *   - FAST   ~ 50MHz max
 *   - HIGH   ~ 100MHz max */
#define	PIN_OPT_OUTPUT_SPEED_LOW	(0 << 4)
#define	PIN_OPT_OUTPUT_SPEED_MEDIUM (1 << 4)
#define	PIN_OPT_OUTPUT_SPEED_FAST	(2 << 4)
#define	PIN_OPT_OUTPUT_SPEED_HIGH	(3 << 4)

/*   alternate function selection option */
#define PIN_OPT_AF0					0x0
#define PIN_OPT_AF1					0x1
#define PIN_OPT_AF2					0x2
#define PIN_OPT_AF3					0x3
#define PIN_OPT_AF4					0x4
#define PIN_OPT_AF5					0x5
#define PIN_OPT_AF6					0x6
#define PIN_OPT_AF7					0x7
#define PIN_OPT_AF8					0x8
#define PIN_OPT_AF9					0x9
#define PIN_OPT_AF10				0xA
#define PIN_OPT_AF11				0xB
#define PIN_OPT_AF12				0xC
#define PIN_OPT_AF13				0xD
#define PIN_OPT_AF14				0xE
#define PIN_OPT_AF15				0xF

/*   irq pin option */
#define PIN_OPT_IRQ_EDGE_RISE		(1 << 12)
#define PIN_OPT_IRQ_EDGE_FALL		(2 << 12)
#define PIN_OPT_IRQ_EDGE_BOTH		(3 << 12)

typedef void (*OnIO)();

/* io_configure
 *
 * configure pins referenced in 'pin_mask' of specified port
 * 'gpio' according to 'pin_cfg' and associate a callback
 * function 'cb' if not NULL.
 * returns 0 if success
 */
int io_configure(GPIO_t *gpio, uint16_t pin_mask, uint32_t pin_cfg, OnIO cb);

/* io_unset_irq
 *
 * disable irq for the current pin
 */
void io_unset_irq(GPIO_t *gpio, uint16_t pin_mask);
/* io_read
 *
 * read 32 bit data from port 'gpio', filter the result with mask
 */
uint32_t io_read(GPIO_t *gpio, uint16_t mask);

/* io_write
 *
 * write 16 bit data filtered by mask to port 'gpio'
 * '1' in val are written as HIGH level on port pins
 */
void io_write(GPIO_t *gpio, uint16_t val, uint16_t mask);

/* io_write_n
 *
 * write 16 bit data filtered by mask to port 'gpio'
 * '1' in val are written as LOW level on port pins
 */
void io_write_n(GPIO_t *gpio, uint16_t val, uint16_t mask);

/* io_set/clear
 *
 * set or clear outputs according to bit mask
 */
void io_set(GPIO_t *gpio, uint16_t mask);
void io_clear(GPIO_t *gpio, uint16_t mask);

#ifdef __cplusplus
}
#endif
#endif
