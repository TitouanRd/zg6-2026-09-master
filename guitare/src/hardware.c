#include <stdlib.h>
#include <math.h>

#include "hardware.h"

#include "lib/io.h"
#include "lib/timer.h"
#include "lib/event.h"
#include "lib/uart.h"
#include "lib/term.h"

#ifndef M_PI
#define M_PI            3.14159265358979323846f
#endif

/**** servo *****************************************************/
#define SERVO_FREQ_HZ		50

/*** servo_init(void) : pin and timer initialization */
void servo_init(void)
{
    // PB6 → TIM4 CH1
    io_configure(SERVO_GPIO_PORT, SERVO_GPIO_PIN, SERVO_GPIO_CFG, NULL);

    // Init PWM à 50 Hz
    pwm_init(_TIM4, 50, NULL, NULL);

    // Activer canal 1 avec position neutre (7.5%)
    pwm_channel_enable(_TIM4, PWM_CHANNEL_1, 7.5f, 1);

	

    // Démarrer
    pwm_start(_TIM4);
}

/*** servo_position(int theta) : set the angle theta [°] */
void servo_position(int theta)
{
    if(theta < 0) theta = 0;
    if(theta > 180) theta = 180;

    // Conversion angle → duty cycle (%)
    float duty = 2.5f + ((float)theta * (12.5f - 2.5f)) / 180.0f;

    pwm_channel_enable(_TIM4, PWM_CHANNEL_1, duty, 1);
}

/**** solenoid **************************************************/
/* solenoid_init : setup required pin */
void solenoid_init(void)
{
	// SOLENOID : broche PC8 en GPIO
	io_configure(SOLENOID_GPIO_PORT, SOLENOID_GPIO_PIN, SOLENOID_GPIO_CFG, NULL);
}

/* solenoid : control solenoid */
void solenoid(bool on)
{
	if (on) {
		io_set(SOLENOID_GPIO_PORT,SOLENOID_GPIO_PIN);
	} else {
		io_clear(SOLENOID_GPIO_PORT,SOLENOID_GPIO_PIN);
	}
}

//****************************************************************
// * stepper motor definitions
//****************************************************************/
#define STEPPER_DUTY				50.0f

#define STEPPER_BUSY				(1U<<0)
#define STEPPER_CLOCKWISE			(1U<<1)

#define MAX_ACCEL_STEPS				300

static pwm_cfg_regs_t cfg_regs[MAX_ACCEL_STEPS];

/* called from */
static void stepper_cb(void *ptr)
{
	/* ... */
}

/* enable stepper motor control */
void stepper_en(stepper_t *stepper, bool on)
{
	if (on) {
		io_clear(STEN_GPIO_PORT, STEN_GPIO_PIN);
	} else {
		io_set(STEN_GPIO_PORT, STEN_GPIO_PIN);
	}
}

/* get number of steps */
int stepper_pos(stepper_t *stepper)
{
	return stepper->pos;
}

/* reset number of steps */
void stepper_pos_reset(stepper_t *stepper)
{
	stepper->pos = 0;
}

/* stepper motor busy? */
bool stepper_busy(stepper_t *stepper)
{
	return stepper->flags & STEPPER_BUSY;
}

void stepper_stop(stepper_t *stepper)
{
	pwm_stop(_TIM1);
	stepper_en(stepper,false);
	stepper->flags &= ~STEPPER_BUSY;
}

/* stepper_run(uint32_t nsteps, bool freerun)
 *   run nsteps microsteps
 */
void stepper_run(stepper_t *stepper, int32_t nsteps, bool freerun)
{
	/* ... */
}

/*** stepper_init
 *  setup peripherals to run the stepper motor at freq [steps/s] or [Hz]
 */
void stepper_init(stepper_t *stepper, int freq)
{
	stepper->pos = 0;
	stepper->flags = 0;
	stepper->nusteps = 0;
	
	// STEPPER MOTOR [DIR]  : PA9 en output 
	io_configure(STDIR_GPIO_PORT, STDIR_GPIO_PIN, STDIR_GPIO_CFG, NULL);

	// STEPPER MOTOR [EN ]  : PC7 en output
	io_configure(STEN_GPIO_PORT, STEN_GPIO_PIN, STEN_GPIO_CFG, NULL);
	io_set(STEN_GPIO_PORT, STEN_GPIO_PIN);

	// STEPPER MOTOR [STEP] : TIM1 CH1 en PWM : PA8 (AF01)
	io_configure(STSTEP_GPIO_PORT, STSTEP_GPIO_PIN, STSTEP_GPIO_CFG, NULL);
	pwm_init(_TIM1, freq, stepper_cb, stepper);
	pwm_channel_enable(_TIM1, PWM_CHANNEL_1, STEPPER_DUTY, 1);

	// limit switch sensor : irq rising sur PC9
//	io_configure(FC_GPIO_PORT, FC_GPIO_PIN, FC_GPIO_CFG, fc_cb);
}

/* stepper pos in mm */
float stepper_pos_mm(stepper_t *stepper)
{
	return (float)stepper->pos/stepper->steps_per_mm/(float)stepper->usteps;
}

/* run mm using the profile chosen in stepper_init_mm */
void stepper_run_mm(stepper_t *stepper, float mm)
{
	float nsteps=0.0;

	/* ... */

	stepper_run(stepper,(int32_t)nsteps,false);
}

/* stepper_init_mm 
 * - stepper : stepper handling structure
 * - vel0    : low speed [mm/s]
 * - vel1    : high speed [mm/s]
 * - accel   : acceleration [mm/s²]
 * - flags   : profile mode (uniform/trapezoidal)
 * If trapezoidal profile is chosen, it is calculated from vel0, vel1 and accel,
 * else, only vel0 is used.
 */
void stepper_init_mm(stepper_t *stepper, float vel0, float vel1, float accel, uint32_t flags)
{
//	stepper->steps_per_mm = 
//	stepper->steps_max = 

	// calculate the timer frequency in Hz or [step/s] corresponding to vel0
	int freq = 0;
	term_printf("freq = %d\r\n",(int)freq);
	
	stepper_init(stepper, freq);
	
	if (flags & STEPPER_MODE_TRAPEZOIDAL) {	/* use trapezoidal speed profile */

		stepper->flags |= STEPPER_MODE_TRAPEZOIDAL;
		
		/* ... */
	} else {								/* use uniform speed profile*/
		stepper->flags &= ~STEPPER_MODE_TRAPEZOIDAL;
	}
}