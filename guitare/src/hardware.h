#ifndef _HARDWARE_H_
#define _HARDWARE_H_

#include <stdbool.h>

#include "include/board.h"
#include "lib/timer.h"

/**** servo *****************************************************
 * servo_init(void): pin and timer initialization
 * servo_position(int theta) : set the angle theta [°]
 */
#define THETA1						135
#define THETA2						155

void servo_init(void);
void servo_position(int theta);

/**** solenoid **************************************************
 * void solenoid_init(void) : pin initialization
 * solenoid_init(void) : setup required pin
 * solenoid(bool on)   : control solenoid
 */
#define ON							1
#define OFF							0

void solenoid_init(void);
void solenoid(bool on);

/**** stepper ***************************************************
 *
 */

/* flag bits */
#define STEPPER_MODE_FREERUN		(1U<<2)
#define STEPPER_MODE_TRAPEZOIDAL	(1U<<3)	/* bit 3 : 0 --> uniform speed, 1 --> trapezoidal speed profile */
#define STEPPER_MODE_UNIFORM		(0)		/* --> uniform speed, 1 --> trapezoidal speed profile */


typedef struct _stepper_t {
	/* physical and technical values */
	float			max_pos_mm;			/* maximum displacement [mm] */
	float			d_pulley_mm;		/* pulley diameter [mm] */
	int				usteps;				/* stepper motor microstep configuration */
	int		 		steps_per_rev;		/* stepper motor step resolution */
	
	/* constants */
	float			steps_per_mm;		/* number of steps per mm */
	int				steps_max;			/* maximum steps to run the whole max_dx */
	int				max_accel_steps;	/* max number of steps in acceleration/deceleration phases */
	
	/* state variables */
	uint32_t		flags;				/* clockwise and busy flag */
	int				pos;				/* absolute rotor position [in microsteps] */
	int				nsteps;				/* number of steps to perform */
	int				nusteps;			/* microstep counter */
	int				k, ksteps;			/* coarse step for profile handling */
	int				k_accel, k_steady;	/* trapezoidal profile indexes */
	pwm_cfg_regs_t*	cfg_regs_accel;		/* acceleration/deceleration profile configuration */
	pwm_cfg_regs_t	cfg_regs_steady;	/* steady configuration */
	
	/* Peripheral handling */
	TIM_t const *	tmr;				/* timer managing steps */
//	uint32_t		channel;			/* timer channel */
//	GPIO_t *		en_port;			/* enable port */
//	GPIO_t *		dir_port;			/* direction port */
//	uint16_t		en_pin;				/* enable pin */
//	uint16_t		dir_pin;			/* direction pin */
//	func_cfg_t		cfg_f;				/* config function (for PWM pin) */
} stepper_t;

void fc_cb(void);
void stepper_init(stepper_t *stepper, int freq);
void stepper_run(stepper_t *stepper, int32_t nsteps, bool freerun);

int stepper_pos(stepper_t *stepper);
void stepper_pos_reset(stepper_t *stepper);

void stepper_run_mm(stepper_t *stepper, float mm);
float stepper_pos_mm(stepper_t *stepper);

void stepper_en(stepper_t *stepper, bool on);
bool stepper_busy(stepper_t *stepper);
void stepper_stop(stepper_t *stepper);

void stepper_init_mm(stepper_t *stepper, float vel0, float vel1, float accel, uint32_t flags);
void stepper_run_mm(stepper_t *stepper, float mm);

#endif

