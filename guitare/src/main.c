#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include "include/board.h"
#include "lib/tick.h"
#include "lib/io.h"
#include "lib/timer.h"
#include "lib/uart.h"
#include "lib/event.h"
#include "lib/term.h"
#include "lib/util.h"
#include "hardware.h"

#define MAIN1


#ifdef MAIN1
/****************************************************************
 * App state
 */
typedef enum {
	APP_RESET,
	APP_READY,
	APP_PLAYING,
} app_state_t;

app_state_t app_state = APP_RESET;

int servo_state = 0;

/****************************************************************
 * Stepper
 ****************************************************************/
#define STEPPER_LOW_SPEED			20.0f	/* mm/s */
#define STEPPER_HIGH_SPEED			450.0f	/* mm/s */
#define STEPPER_ACCEL				5000.0f	/* mm/s² */

#define STEPPER_INIT_FREQ			3000	/* Hz */

stepper_t stepper = {
	.max_pos_mm		= 320.0f,	/* maximum linear distance */
	.d_pulley_mm	= 12.7f,	/* pulley diameter */
	.steps_per_rev	= 200,		/* number of steps per revolution */
	.usteps			= 32,		/* number of microsteps per step */
	
	.tmr 			= (TIM_t *)TIM1_BASE		/* timer used */
};

float speed_low=STEPPER_LOW_SPEED, speed_high=STEPPER_HIGH_SPEED, accel=STEPPER_ACCEL;

/**** limit switch sensor callback ******************************/
void fc_cb(void)
{
	// when at end position get 50 steps forward
}

/****************************************************************
 * Command handling
 ****************************************************************/

#define BUF_MAX_LEN		80

static char buf[BUF_MAX_LEN];

bool cmd_append_done(char *buf, int len, char c)
{
	static int pos=0;

	if (pos<len-1 && c!='\r') {
		buf[pos++]=c;
		uart_putc(_USART2,c);
		return false;
	}
	buf[pos]='\0';
	pos=0;
	return true;
}

/**
 * @brief Interprets and executes the command stored in the 'command' buffer.
 *        Commands are sent via UART in text form and start with a character identifier.
 */
void cmd_process(char *buf)
{
	int val;
	float mm;
	
	switch (buf[0]) {
	case 'R':
		app_state = APP_RESET;
		// limit switch sensor : irq rising sur PC9
		io_configure(FC_GPIO_PORT, FC_GPIO_PIN, FC_GPIO_CFG, fc_cb);
		
		solenoid(OFF);
		servo_position(THETA1);
		stepper_init(&stepper,STEPPER_INIT_FREQ);
		// go to stop position
		stepper_run(&stepper,-1,true);
		break;
	case 'T':   // TEST servo
        term_printf("Executing test command T...\r\n");
        solenoid(ON);
        term_printf("Solenoid ON\r\n");

        servo_position(170);
        term_printf("Servo at 170°\r\n");
		delay_ms(1000);

		servo_position(80);
        term_printf("Servo at 100°\r\n");
		delay_ms(500);
		
        solenoid(OFF);
        term_printf("Solenoid OFF\r\n");
        term_printf("Test T completed\r\n");
        break;
	default:
//		term_printf("\r\nUnknown command\r\n");
		break;
	}
}

/****************************************************************
 * Main
 ****************************************************************/
int main(void)
{
	// UART interface init
	tick_init(1,NULL,NULL);
	
	// UART interface init
	uart_init(_USART2, 115200, UART_8N1);
	term_init(80, 24);
	term_clrscr();
	term_color(CL_DEFAULT,CL_NORMAL,CL_NORMAL);
	
	// limit switch sensor : irq rising sur PC9
	io_configure(FC_GPIO_PORT, FC_GPIO_PIN, FC_GPIO_CFG, fc_cb);
	
	solenoid_init();
	//solenoid(ON);     // active le solénoïde
	
	
	servo_init();
	// stepper_init(&stepper,STEPPER_INIT_FREQ);
	
	/* Boucle d'événements pour les commandes interactives */
	term_printf("System ready. Type 'T' for servo test or 'R' for reset.\r\n");
	term_printf("> ");
	while (1){
		Event_t evt;
		while (!pop_event(&evt)) ;
		switch (evt.type) {
		case CHAR_EVT:
			if (!cmd_append_done(buf,BUF_MAX_LEN,evt.key.c)) break;
			cmd_process(buf);
			uart_puts(_USART2, "\r\n> ");
			break;
		case STEPPER_EVT:
			break;
		case TIMER_EVT:
			break;
		case ADC_DMA_EVT:
			break;
		default:
			break;
		}
	}
    
	return 0;
}

#endif /* MAIN1 */

