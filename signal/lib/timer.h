#ifndef _TIMER_H_
#define _TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif 

#include "include/board.h"

typedef void (*OnTick)(void *ptr);

/***************************************************************************/
/* timer_wait_ms
 *   wait for ms millisecoonds function
 */
int timer_wait_ms(TIM_t *tmr, uint32_t ms);

/* timer_wait_us
 *   wait for us microsecoonds function
 */
int timer_wait_us(TIM_t *tmr, uint32_t us);

/***************************************************************************/
/* timer_tick_init
 *   setup timer to call cb function periodically, each tick_ms
 */
int timer_tick_init(TIM_t *tmr, uint32_t tick_ms, OnTick cb, void *usr_ptr);

/* timer_tick_period
 *   change the tick_ms period
 */
int timer_tick_period(TIM_t *tmr, uint32_t tick_ms);

/* timer_tick_init_us
 *   setup timer to call cb function periodically, each tick_us
 */
int timer_tick_init_us(TIM_t *tmr, uint32_t tick_us, OnTick cb, void *user_ptr);

/* timer_count_init
 *   setup timer to count each timebase_us us
 */
int timer_count_init(TIM_t *tmr, uint32_t timebase_us);

/* timer_start
 *   start counting to generate ticks
 */
void timer_start(TIM_t *tmr);

/* timer_stop
 *   stop and reset counting
 */
void timer_stop(TIM_t *tmr);

/* timer_read
 *   return the counter value
 */
uint32_t timer_read(TIM_t *tmr);

/***************************************************************************/
#define PWM_CHANNEL_1		0
#define PWM_CHANNEL_2		1
#define PWM_CHANNEL_3		2
#define PWM_CHANNEL_4		3

typedef struct {uint16_t psc, arr;} pwm_cfg_regs_t;

/* pwm_init
 *   setup pwm timer with period_us us period
 */
int pwm_init(PWM_t *tmr, uint32_t freq_Hz, OnTick cb, void *usr_ptr);

/* pwm_get_freq_cfg
 *   calculate reg_cfg configuration according to freq_Hz param
 */
int pwm_get_freq_cfg(PWM_t *tmr, uint32_t freq_Hz, pwm_cfg_regs_t *regs);

/* pwm_set_freq_cfg
 *   set frequency according to reg_cfg param, dutycycle unchanged
 */
int pwm_set_freq_cfg(PWM_t *tmr, const pwm_cfg_regs_t *regs);

/* pwm_channel_enable
 *   set up pwm channel
 */
int pwm_channel_enable(PWM_t *tmr, uint32_t channel, float dutycycle, uint32_t oe);

/* pwm_channel_disable
 *   disable pwm channel
 */
int pwm_channel_disable(PWM_t *tmr, uint32_t channel);

/* pwm_channel_set
 *   set up dutycycle for pwm channel
 */
int pwm_channel_set(PWM_t *tmr, uint32_t channel, float dutycycle);

/* pwm_start
 *   start counting
 */
#define pwm_start(tmr)	timer_start(tmr)

/* pwm_stop
 *   stop and reset counting
 */
#define pwm_stop(tmr)	timer_stop(tmr)

/***************************************************************************/

#ifdef __cplusplus
}
#endif
#endif
