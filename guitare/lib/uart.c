#include <stdlib.h>
#include <stdarg.h>
#include "uart.h"
#include "io.h"
#include "util.h"
#include "tick.h"

/* Ring Buffer used for temporary storage
 *
 *   | i_pop
 *   V
 * +---+---+---+---+---+---+---+---+
 * | A | B | D | C | e |   |   |   |
 * +---+---+---+---+---+---+---+---+
 *                   ^
 *                   | i_push
 *
 * Buffer is empty if
 *                   | i_pop
 *                   V
 * +---+---+---+---+---+---+---+---+
 * | A | B | D | C | e |   |   |   |
 * +---+---+---+---+---+---+---+---+
 *                   ^
 *                   | i_push
 *
 * Buffer is full if
 *                   | i_pop
 *                   V
 * +---+---+---+---+---+---+---+---+
 * | e | s | t | C | e | c | i | _ |
 * +---+---+---+---+---+---+---+---+
 *               ^
 *               | i_push
 *
 */

#define BUF_SZ		32

typedef volatile struct {
	char 	data[BUF_SZ];	// data to write
	int		i_push;			// index where to push the next data in the buffer
	int		i_pop;			// index where to pop the next data from the buffer
} buf_t;

typedef volatile struct {
	buf_t		tx_buf;
	buf_t		rx_buf;
	int			tx_completed;
} uart_context_t;

#ifdef USE_USART1
static uart_context_t u1_ctx;

void USART1_IRQHandler(void)
{
	uint32_t sr = _USART1->SR;
	
	if ((sr & (1<<7)) && (u1_ctx.tx_buf.i_pop!=u1_ctx.tx_buf.i_push)) {	// Transmit data register empty evt
		// send next data
		_USART1->DR = u1_ctx.tx_buf.data[u1_ctx.tx_buf.i_pop];
		u1_ctx.tx_buf.i_pop = (u1_ctx.tx_buf.i_pop+1) % BUF_SZ;
		
		// if no more data to write, then disable TxE IRQ
		if (u1_ctx.tx_buf.i_pop==u1_ctx.tx_buf.i_push) {
			_USART1->CR1 &= ~(1<<7);
		}
	} else if (sr & (1<<5)) {	// Read data register not empty evt
		if (!((sr & (1<<1)) || (sr & (1<<2)))) {
			if ((u1_ctx.rx_buf.i_push+1) % BUF_SZ != u1_ctx.rx_buf.i_pop) {	// buffer  not full
				u1_ctx.rx_buf.data[u1_ctx.rx_buf.i_push]=(char)_USART1->DR;
				u1_ctx.rx_buf.i_push = (u1_ctx.rx_buf.i_push+1) % BUF_SZ;
			} else {	// data lots!
				_USART1->DR;
			}
		} else {				// Noise or framing error or break detected
			_USART1->DR;
		}
	} else if (sr & (1<<6)) {	// transmit completed evt
		_USART1->CR1 &= ~(1<<6);
		u1_ctx.tx_completed = 1;
	}
	if (sr & (1<<4)) {	// idle interrupt
		_USART1->DR;
	} else if (sr & (1<<3)) {	// overrun interrupt
		_USART1->DR;
	} else if (sr & (1<<0)) {	// parity error interrupt
		_USART1->DR;
	}
}
#endif

#ifdef USE_USART2
uart_context_t u2_ctx;

void USART2_IRQHandler(void)
{
	uint32_t sr = _USART2->SR;
	
	if ((sr & (1<<7)) && (u2_ctx.tx_buf.i_pop!=u2_ctx.tx_buf.i_push)) {	// Transmit data register empty interrupt
		// send next data
		_USART2->DR = u2_ctx.tx_buf.data[u2_ctx.tx_buf.i_pop];
		u2_ctx.tx_buf.i_pop = (u2_ctx.tx_buf.i_pop+1) % BUF_SZ;
		
		// if no more data to write, then disable TxE IRQ
		if (u2_ctx.tx_buf.i_pop==u2_ctx.tx_buf.i_push) {
			_USART2->CR1 &= ~(1<<7);
		}
	} else if (sr & (1<<5)) {			// Read data register not empty interrupt
		if (!((sr & (1<<1)) || (sr & (1<<2)))) {
			if ((u2_ctx.rx_buf.i_push+1) % BUF_SZ != u2_ctx.rx_buf.i_pop) {	// buffer  not full
				u2_ctx.rx_buf.data[u2_ctx.rx_buf.i_push]=(char)_USART2->DR;
				u2_ctx.rx_buf.i_push = (u2_ctx.rx_buf.i_push+1) % BUF_SZ;
			} else {	// data lots!
				_USART2->DR;
			}
		} else {				// Noise or framing error or break detected
			_USART2->DR;
		}
	} else if (sr & (1<<6)) {	// transmit completed evt
		_USART2->CR1 &= ~(1<<6);
		u2_ctx.tx_completed = 1;
	}
	if (sr & (1<<4)) {	// idle interrupt
		_USART2->DR;
	} else if (sr & (1<<3)) {	// overrun interrupt
		_USART2->DR;
	} else if (sr & (1<<0)) {	// parity error interrupt
		_USART2->DR;
	}
}
#endif

#ifdef USE_USART6
static uart_context_t u6_ctx;

void USART6_IRQHandler(void)
{
	uint32_t sr = _USART6->SR;
	
	if ((sr & (1<<7)) && (u6_ctx.tx_buf.i_pop!=u6_ctx.tx_buf.i_push)) {	// Transmit data register empty interrupt
		// send next data
		_USART6->DR = u6_ctx.tx_buf.data[u6_ctx.tx_buf.i_pop];
		u6_ctx.tx_buf.i_pop = (u6_ctx.tx_buf.i_pop+1) % BUF_SZ;
		
		// if no more data to write, then disable TxE IRQ
		if (u6_ctx.tx_buf.i_pop==u6_ctx.tx_buf.i_push) {
			_USART6->CR1 &= ~(1<<7);
		}
	} else if (sr & (1<<5)) {			// Read data register not empty interrupt
		if (!((sr & (1<<1)) || (sr & (1<<2)))) {
			if ((u6_ctx.rx_buf.i_push+1) % BUF_SZ != u6_ctx.rx_buf.i_pop) {	// buffer  not full
				u6_ctx.rx_buf.data[u6_ctx.rx_buf.i_push]=(char)_USART6->DR;
				u6_ctx.rx_buf.i_push = (u6_ctx.rx_buf.i_push+1) % BUF_SZ;
			} else {	// data lots!
				_USART6->DR;
			}
		} else {				// Noise or framing error or break detected
			_USART6->DR;
		}
	} else if (sr & (1<<6)) {	// transmit completed evt
		_USART6->CR1 &= ~(1<<6);
		u6_ctx.tx_completed = 1;
	}
	if (sr & (1<<4)) {	// idle interrupt
		_USART6->DR;
	} else if (sr & (1<<3)) {	// overrun interrupt
		_USART6->DR;
	} else if (sr & (1<<0)) {	// parity error interrupt
		_USART6->DR;
	}
}
#endif

#define UART_IRQMASKALL		(0x1F<<4)
/* 
 * uart_enable_irq: enable IRQs in irqmask parameter
 */
void uart_enable_irq(USART_t *u, uint32_t irqmask)
{
	u->CR1 |= (irqmask & UART_IRQMASKALL);
}

/* 
 * uart_disable_irq: enable IRQs in irqmask parameter
 */
void uart_disable_irq(USART_t *u, uint32_t irqmask)
{
	u->CR1 &= ~(irqmask & UART_IRQMASKALL);
}

static uart_context_t* uart_get_context(USART_t *u)
{
	if (u == _USART1) {
#ifdef USE_USART1
		return &u1_ctx;
#else
		return NULL;
#endif
	} else if (u == _USART2) {
#ifdef USE_USART2
		return &u2_ctx;
#else
		return NULL;
#endif
	} else if (u == _USART6) {
#ifdef USE_USART6
		return &u6_ctx;
#else
		return NULL;
#endif
	}
	return NULL;
}

/*
 * uart_init : polling Tx and IRQ Rx
 */
int uart_init(USART_t *u, uint32_t baud, uint32_t mode)
{
	IRQn_t	irq_number;
	uint32_t irq_priority;
	uart_context_t *ctx;
	if (u == _USART1) {
#ifdef USE_USART1
		// Reset USART
		_RCC->APB2RSTR |= 1<<4;
		_RCC->APB2RSTR &= ~(1<<4);
		// enable USART clocking
		_RCC->APB2ENR |= 1<<4;
		
		// configure Tx/Rx pins : Tx -->, Rx --> 
		io_configure(USART1_GPIO_PORT, USART1_GPIO_PINS, USART1_GPIO_CFG, NULL);

		ctx = &u1_ctx;
		irq_number = USART1_IRQn;
		irq_priority = USART1_IRQ_PRIORITY;
		
        //u->BRR = (sysclks.apb2_freq / baud);
        u->BRR = ((2 * sysclks.apb2_freq) + baud) / (2 * baud);
#else
		return -1;
#endif
	 } else if (u == _USART2) {
#ifdef USE_USART2
		// Reset USART
		_RCC->APB1RSTR |= 1<<17;
		_RCC->APB1RSTR &= ~(1<<17);
		// enable USART clocking
		_RCC->APB1ENR |= 1<<17;
	
		// configure Tx/Rx pins : Tx --> PA2, Rx --> PA3
		io_configure(USART2_GPIO_PORT, USART2_GPIO_PINS, USART2_GPIO_CFG, NULL);
		
		ctx = &u2_ctx;
		irq_number = USART2_IRQn;
		irq_priority = USART2_IRQ_PRIORITY;
		
        //u->BRR = (sysclks.apb1_freq / baud);
        u->BRR = ((2 * sysclks.apb1_freq) + baud) / (2 * baud);
#else
		return -1;
#endif
	} else if (u == _USART6) {
#ifdef USE_USART6
		// Reset USART
		_RCC->APB2RSTR |= 1<<5;
		_RCC->APB2RSTR &= ~(1<<5);
		// enable USART clocking
		_RCC->APB2ENR |= 1<<5;
		
		// configure Tx/Rx pins
		io_configure(USART6_GPIO_PORT, USART6_GPIO_PINS, USART6_GPIO_CFG, NULL);

		ctx = &u6_ctx;
		irq_number = USART6_IRQn;
		irq_priority = USART6_IRQ_PRIORITY;

        //u->BRR = (sysclks.apb2_freq / baud);
        u->BRR = ((2 * sysclks.apb2_freq) + baud) / (2 * baud);
#else
		return -1;
#endif
	} else {
		return -1;
	}
	
	/* Configure peripheral */
	u->GTPR = 0;
	u->CR3 = 0;
	u->CR2 = (((mode >> 4) & 0x3) << 12);
	u->CR1 = ((mode & 0x1) << 12) | (((mode >> 8) & 0x7) << 8) |
	         (1<<5) | (1<<3) | (1<<2) | (1<<13);

	/* Setup starting context */
	ctx->tx_buf.i_push = ctx->tx_buf.i_pop = 0;
	ctx->rx_buf.i_push = ctx->rx_buf.i_pop = 0;
	ctx->tx_completed = 1;
	
	/* Configure NVIC */
	NVIC_SetPriority(irq_number, irq_priority);
	NVIC_EnableIRQ(irq_number);
	
    return 0;
}

/*
 * uart_getc : get a char from the serial link (blocking)
 */
char uart_getc(USART_t *u)
{
	char c;
	uart_context_t *ctx = uart_get_context(u);
	while (ctx->rx_buf.i_pop==ctx->rx_buf.i_push) ;	// Receive buffer empty, wait.
	c = ctx->rx_buf.data[ctx->rx_buf.i_pop];
	ctx->rx_buf.i_pop = (ctx->rx_buf.i_pop+1) % BUF_SZ;
	return c;
}

/*
 * uart_getc_with_timeout : get a char from the serial link 
 * (blocking with timeout)
 */
int uart_getc_with_timeout(USART_t *u, int timeout_ms)
{
	char c;
	uint32_t t0;
	uart_context_t *ctx = uart_get_context(u);
	t0 = ticks();
	while (ctx->rx_buf.i_pop==ctx->rx_buf.i_push) {	// Receive buffer empty, wait.
		if (ticks()-t0 >= (uint32_t)timeout_ms) return -1;
	}
	c = ctx->rx_buf.data[ctx->rx_buf.i_pop];
	ctx->rx_buf.i_pop = (ctx->rx_buf.i_pop+1) % BUF_SZ;
	return (int)c;
}

/*
 * uart_getchar : check if a char has been received from the serial link
 * (non-blocking)
 */
int uart_getchar(USART_t *u, char *c)
{
	uart_context_t *ctx = uart_get_context(u);
	if (ctx->rx_buf.i_pop!=ctx->rx_buf.i_push) {
		// Receive buffer not empty
		*c = ctx->rx_buf.data[ctx->rx_buf.i_pop];
		ctx->rx_buf.i_pop = (ctx->rx_buf.i_pop+1) % BUF_SZ;
		return 1;
	}
	return 0;
}

/*
 * uart_putc : send a char over the serial link (polling)
 */
void uart_putc(USART_t *u, char c)
{
	uart_context_t *ctx = uart_get_context(u);
	ctx->tx_completed=0;
	while ((ctx->tx_buf.i_push+1) % BUF_SZ == ctx->tx_buf.i_pop) ;	// buffer full, so wait.
	ctx->tx_buf.data[ctx->tx_buf.i_push]=c;
	ctx->tx_buf.i_push = (ctx->tx_buf.i_push+1) % BUF_SZ;
	// enable TxE IRQ
//	u->CR1 |= (1<<7);
	u->CR1 |= (1<<6) | (1<<7);
}

/*
 * uart_puts : send a string over the serial link (polling)
 */
int uart_puts(USART_t *u, const char *s)
{
	char c;
	int len=0;
	if (s) {
		uart_context_t *ctx = uart_get_context(u);
		ctx->tx_completed=0;
		while ((c=*s++)!=0) {
			while ((ctx->tx_buf.i_push+1) % BUF_SZ == ctx->tx_buf.i_pop) ;	// buffer full, so wait

			ctx->tx_buf.data[ctx->tx_buf.i_push]=c;
			ctx->tx_buf.i_push = (ctx->tx_buf.i_push+1) % BUF_SZ;
			len++;
			// enable TxE IRQ
//			u->CR1 |= (1<<7);
			u->CR1 |= (1<<6) | 1<<7;
		}
	}
	return len;
}

/*
 * uart_printf : print formatted text to serial link
 */
void uart_printf(USART_t * u, const char* fmt, ...)
{
	va_list        ap;
	char          *p;
	char           ch;
	unsigned long  ul;
	char           s[34];
	
	va_start(ap, fmt);
	while (*fmt != '\0') {
		if (*fmt =='%') {
			switch (*++fmt) {
				case '%':
					uart_putc(u,'%');
					break;
				case 'c':
					ch = (char)va_arg(ap, int);
					uart_putc(u, ch);
					break;
				case 's':
					p = va_arg(ap, char *);
					uart_puts(u, p);
					break;
				case 'd':
					ul = va_arg(ap, long);
					if ((long)ul < 0) {
						uart_putc(u, '-');
						ul = -(long)ul;
					}
					num2str(s, ul, 10, 0, 0);
					uart_puts(u, s);
					break;
				case 'u':
					ul = va_arg(ap, unsigned int);
					num2str(s, ul, 10, 0, 0);
					uart_puts(u, s);
					break;
				case 'x':
					ul = va_arg(ap, unsigned int);
					num2str(s, ul, 16, 0, 0);
					uart_puts(u, s);
					break;
				default:
				    uart_putc(u, *fmt);
			}
		} else uart_putc(u, *fmt);
		fmt++;
	}
	va_end(ap);
}

/*
 * uart_read: fill a buffer with data from the serial link
 */
int uart_read(USART_t *u, uint8_t *buf, int len)
{
	if (buf) {
		uart_context_t *ctx = uart_get_context(u);
		ctx->tx_completed=0;
		for (int i=0; i<len ; i++) {
			while (ctx->rx_buf.i_pop==ctx->rx_buf.i_push) ;	// Receive buffer empty, wait.
			buf[i] = (uint8_t)ctx->rx_buf.data[ctx->rx_buf.i_pop];
			ctx->rx_buf.i_pop = (ctx->rx_buf.i_pop+1) % BUF_SZ;
		}
		return len;
	}
	return 0;
}

/*
 * uart_write: send a buffer over the serial link
 */
int uart_write(USART_t *u, uint8_t *buf, int len)
{
	if (buf) {
		uart_context_t *ctx = uart_get_context(u);
		ctx->tx_completed=0;
		for (int i=0; i<len ; i++) {
			while ((ctx->tx_buf.i_push+1) % BUF_SZ == ctx->tx_buf.i_pop) ;	// buffer full, so wait

			ctx->tx_buf.data[ctx->tx_buf.i_push]=buf[i];
			ctx->tx_buf.i_push = (ctx->tx_buf.i_push+1) % BUF_SZ;
			// enable TxE IRQ
			u->CR1 |= (1<<6) | 1<<7;
		}
		return len;
	}
	return 0;
}

/*
 * uart_tx_completed: trnsmission has been completed
 */
int uart_tx_completed(USART_t *u)
{
//	uart_context_t *ctx = uart_get_context(u);
//	return ctx->tx_completed;
	return (u->SR & (1<<6))!=0;
}
