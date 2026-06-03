#ifndef _TICK_H_
#define _TICK_H_

#ifdef __cplusplus
extern "C" {
#endif 

#include "include/board.h"

typedef void (*OnTick)(void *ptr);

/* tick_init
 *   setup systick timer
 */
int tick_init(uint32_t ms, OnTick cb, void *ptr);

/* tick_start
 *   start the systick timer
 */
void tick_start();

/* tick_stop
 *   stop the systick timer
 */
void tick_stop(void);

/* ticks
 *   get current ticks
 */
uint32_t ticks(void);

/* delay_ms
 *  delay while ms 
 */
void delay_ms(unsigned int ms);

/* tick_delay
 *   delay with systick timer (polling)
 */
int tick_delay(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif
