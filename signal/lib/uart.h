#ifndef _UART_H_
#define _UART_H_

#ifdef __cplusplus
extern "C" {
#endif 

#include "include/board.h"


// Definitions for typical UART 'mode' settings
// CR1 bit 12
#define UART_CHAR_8       (0x00)
#define UART_CHAR_9       (0x01)

// CR2 bits 12-13
#define UART_STOP_1       (0x00<<4)
#define UART_STOP_2       (0x02<<4)

// CR1 bits 8-10
#define UART_PAR_NO       (0x0<<8)
#define UART_PAR_EVEN     (0x5<<8)
#define UART_PAR_ODD      (0x7<<8)

#define UART_8N1          (UART_CHAR_8 | UART_PAR_NO   | UART_STOP_1)
#define UART_8N2          (UART_CHAR_8 | UART_PAR_NO   | UART_STOP_2)
#define UART_8E1          (UART_CHAR_9 | UART_PAR_EVEN | UART_STOP_1)
#define UART_8E2          (UART_CHAR_9 | UART_PAR_EVEN | UART_STOP_2)
#define UART_8O1          (UART_CHAR_9 | UART_PAR_ODD  | UART_STOP_1)
#define UART_8O2          (UART_CHAR_9 | UART_PAR_ODD  | UART_STOP_2)

// IRQ masks (can be orred)
#define	UART_IDLE_IRQ		(1U<<4)
#define	UART_RXNE_IRQ		(1U<<5)
#define UART_TXCOMPLETE_IRQ	(1U<<6)
#define UART_TXE_IRQ		(1U<<7)

/* 
 * uart_enable_irq: enable IRQs in irqmask parameter
 */
void uart_enable_irq(USART_t *u, uint32_t irqmask);

/* 
 * uart_disable_irq: enable IRQs in irqmask parameter
 */
void uart_disable_irq(USART_t *u, uint32_t irqmask);

/*
 * uart_init: initialize with baud, line mode parameters,
 *             polling Tx and IRQ Rx
 */
int uart_init(USART_t *u, uint32_t baud, uint32_t mode);

/*
 * uart_getc: get a char from the serial link (blocking)
 */
char uart_getc(USART_t *u);

/*
 * uart_getc_with_timeout : get a char from the serial link 
 * (blocking with timeout)
 */
int uart_getc_with_timeout(USART_t *u, int timeout_ms);

/*
 * uart_getchar: check if a char has been received from the serial link
 * (non-blocking)
 */
int uart_getchar(USART_t *u, char *pChar);

/*
 * uart_putc: send a char over the serial link
 */
void uart_putc(USART_t *u, char c);

/*
 * uart_puts: send a string over the serial link
 */
int uart_puts(USART_t *u, const char *s);

/*
 * uart_printf: print formatted text to serial link
 */
void uart_printf(USART_t *u, const char* fmt, ...);

/*
 * uart_write: send a buffer over the serial link
 */
int uart_write(USART_t *u, uint8_t *buf, int len);

/*
 * uart_read: fill a buffer with data from the serial link
 */
int uart_read(USART_t *u, uint8_t *buf, int len);

/*
 * uart_tx_completed: trnsmission has been completed
 */
int uart_tx_completed(USART_t *u);

#ifdef __cplusplus
}
#endif
#endif
