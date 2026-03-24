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
#include "lib/adc.h"
#include "hardware.h"
#include "liblcd/lcd.h"
#include "liblcd/graph.h"

#include "include/cmsis/arm_math.h"

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

}

/****************************************************************
 * ADC variables & callback
 */
#define SMPLBUFSIZE			512

/* sample period TS [us], sample frequency FS [Hz] */
#define FS				2000.0f
#define TS				(uint32_t)(1/FS*1.0e6f+0.5f)

#ifdef USE_LCD
#define TIME_POINTS		51
#define FREQ_POINTS		100

#define FMIN			-300.0f
#define FMAX			300.0f

#define GCFG	G_FRAME|G_XTICKS|G_YTICKS|G_AUTOCOLOR|(5<<24)
#define GMASK	G_FRAME|G_XTICKS|G_YTICKS|G_AUTOCOLOR|G_COLOR_MSK

static vect_t xgrid=VECT_INIT, ygrid=VECT_INIT;
static vect_t t=VECT_INIT, x=VECT_INIT;
static vect_t xgridf=VECT_INIT, ygridf=VECT_INIT;
static vect_t f=VECT_INIT, X=VECT_INIT;
#endif

/* DMA buffers */
static volatile uint16_t smpl_buf[2*SMPLBUFSIZE];

static void on_smpl_buf_cb(uint32_t stream, uint32_t bufid)
{
#ifdef TEST_PERF
	static int state=0;
	if (state) io_set(_GPIOA,PIN_10);
	else io_clear(_GPIOA,PIN_10);
	state=!state;
#endif

	/* ... */
}

float fft_input[SMPLBUFSIZE];
float fft_output[SMPLBUFSIZE];
float mag_out[SMPLBUFSIZE/2];
arm_rfft_fast_instance_f32 fft_instance;
float frequency;

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
	real *mx, *mX;	// pointer to signal and spectrum array
	
#ifdef TEST_PERF
	// PA10 --> output
	io_configure(_GPIOA,PIN_10,PIN_MODE_OUTPUT,NULL);
	// PA11 --> output
	io_configure(_GPIOA,PIN_11,PIN_MODE_OUTPUT,NULL);
#endif

	tick_init(1,NULL,NULL);		// start tick counter 1ms
	
	// UART interface init
	uart_init(_USART2, 115200, UART_8N1);
	term_init(80, 24);
	term_clrscr();
	term_color(CL_DEFAULT,CL_NORMAL,CL_NORMAL);
	
#if USE_STEPPER
	// limit switch sensor : irq rising sur PC9
	io_configure(FC_GPIO_PORT, FC_GPIO_PIN, FC_GPIO_CFG, fc_cb);
	
	solenoid_init();
	servo_init();
	stepper_init(&stepper,STEPPER_INIT_FREQ);
	stepper_run(&stepper,-1,true);	// go to stop position
#endif
		
	// Set sampling period TS
	timer_tick_init_us(_TIM2, TS, NULL, NULL);
	
	// initialize ADC, and select channels to sample
	int max_freq = adc_init(_ADC1, ADC_MODE_12BITS|ADC_MODE_TRIGGERED|ADC_MODE_DMA, NULL);
	adc_channel_enable(_ADC1, 0, ADC_SMPL_3CYCLES);

	// set DMA double buffering
	adc_setup_dma(_ADC1, (void*)smpl_buf, SMPLBUFSIZE, on_smpl_buf_cb);
	
	// set the ADC to be triggered by TIM2's update
	adc_set_trigger_evt(_ADC1, ADC_TRIG_ON_TIM2_TRGO);
	int fs = adc_get_sampling_freq(_ADC1);
	
//	arm_rfft_fast_init_f32( );
	
#ifdef USE_LCD
	// init LCD display
	lcd_init();
	lcd_switch_to(LCD_DPY);
	lcd_set_display_orientation(PORTRAIT);
	// graphics initialization
	ginit();
	
	// allocate vectors
	vectorize(&xgrid,0.0f,250e-3f,50e-3f);
	vectorize(&ygrid,-2.0f,2.0f,1.0f);

	// time vector 't', and signal value 'x(t)' for display
	vectorize(&t,0.0f,250e-3f,1/FS);
	mx=vect(&x,t.n);
	for (int i=0; i<x.n; i++) mx[i]=0.0f;
	
	vectorize(&xgridf,0,FMAX,100.0f);
	vectorize(&ygridf,0.0f,0.1f,0.05f);
	
	// frequency vector 'f' and spectrum 'X(f)' for display
	vectorize(&f,0,FS/2.0f*(SMPLBUFSIZE-1)/SMPLBUFSIZE,FS/SMPLBUFSIZE);
	// gdb : p (float[128])*f.data
	mX=vect(&X,f.n);
	for (int i=0; i<x.n; i++) mX[i]=0.0f;
	f.n=SMPLBUFSIZE/2*300/1000+1;
	X.n=f.n;

	// plot grids
	gsubplot(2,1,1);
	gsetplot(0.0f, 250.0e-3f, -2.0f, 2.0f, GCFG, GMASK);
	gsetxgrid(&xgrid,1.0e-3f,RGB(0xB3,0xB3,0xB3));
	gsetygrid(&ygrid,1.0,RGB(0xB3,0xB3,0xB3));
	glabel("t [s]",G_XLABEL); glabel("x(t)",G_YLABEL);
	
	gsubplot(2,1,2);
	gsetplot(0, FMAX, 0.0f, 0.1f, GCFG, GMASK);
	gsetxgrid(&xgridf,1.0e3f,RGB(0xB3,0xB3,0xB3));
	gsetygrid(&ygridf,1.0f,RGB(0xB3,0xB3,0xB3));
	glabel("f [Hz]",G_XLABEL); glabel("|X(f)|",G_YLABEL);
#endif

	// start sampling
	timer_start(_TIM2);
	
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
#ifdef TEST_PERF
			io_set(_GPIOA,PIN_11);		/* start processing */
#endif
			uint16_t *data=(uint16_t*)smpl_buf + evt.adc_dma.id*SMPLBUFSIZE;

#ifdef USE_LCD
			/* time plot */
			gsetgraph(0);
			gsetcolor(15);	// erase the old plot
			gplot(&t,&x);

			gsetcolor(2);	// draw the new one
			gplot(&t,&x);
#endif
			/* FFT */
//			arm_rfft_fast_f32();
//			arm_cmplx_mag_f32();

#ifdef USE_LCD
			/* frequency plot */
			gsetgraph(1);
			gsetcolor(15);	// erase the old plot
			gplot(&f,&X);
			gsetcolor(2);	// draw the new one
			gplot(&f,&X);
#endif
#ifdef TEST_PERF
			io_clear(_GPIOA,PIN_11);	/* end processing */
#endif
			break;
		default:
			break;
		}
	}
    
	return 0;
}

#endif /* MAIN1 */

