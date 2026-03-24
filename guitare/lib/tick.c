#include "tick.h"

static OnTick callback=NULL;
static void * user_ptr=NULL;

static volatile uint32_t _ticks = 0;

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	_ticks++;
	_SysTick->CTRL;
	if (callback) callback(user_ptr);
}


/* tick_init
 *   setup systick timer (IRQ)
 */
int tick_init(uint32_t ms, OnTick cb, void *ptr)
{
	uint32_t tmp = ms*(sysclks.ahb_freq/1000);
	if (tmp <= (1<<24)) {	/* fck=84MHz ==> tick max = 200ms */
		_SysTick->LOAD = tmp-1;
		_SysTick->CTRL = 0x4;
	} else {				/* fck=84MHz ==> tick max = 1600ms */
		tmp = ms*(sysclks.ahb_freq/1000/8);
		if (tmp <= (1<<24)) {
			_SysTick->LOAD = tmp-1;
			_SysTick->CTRL = 0;
		} else {
			return -1;
		}
	}
	callback=cb;
	user_ptr=ptr;
	NVIC_SetPriority(SysTick_IRQn,SYSTICK_PRIORITY);
	_SysTick->CTRL |= 0x3;
	return 0;
}

/* tick_start
 *   start the systick timer
 */
void tick_start(void)
{
	_SysTick->VAL = 0;
	_SysTick->CTRL |= 1;
}


/* tick_stop
 *   stop the systick timer (polling)
 */
void tick_stop(void)
{
	_SysTick->CTRL &= ~1;
}

uint32_t ticks(void)
{
	return _ticks;
}

void delay_ms(unsigned int ms)
{
	uint32_t t0=ticks();
	while ((ticks()-t0)<ms) {__NOP;};
}

/* tick_delay
 *   delay with systick timer
 */
int tick_delay(uint32_t ms)
{
	uint32_t tmp = ms*(sysclks.ahb_freq/1000);
	if (tmp <= (1<<24)) {	/* fck=84MHz ==> tick max = 200ms */
		_SysTick->LOAD = tmp-1;
		_SysTick->CTRL = 0x4;
	} else {				/* fck=84MHz ==> tick max = 1600ms */
		tmp = ms*(sysclks.ahb_freq/1000/8);
		if (tmp <= (1<<24)) {
			_SysTick->LOAD = tmp-1;
			_SysTick->CTRL = 0;
		} else {
			return -1;
		}
	}

	_SysTick->VAL = 0;
	_SysTick->CTRL|=1;
	while ((_SysTick->CTRL & (1<<16))==0) ;
	
	return 0;
}

