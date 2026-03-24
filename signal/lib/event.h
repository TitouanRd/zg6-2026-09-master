#ifndef EVENT_H
#define EVENT_H

#include <stdint.h>
#include <stdbool.h>

/* Event types */
#define ADC_EOC_EVT				1
#define ADC_DMA_EVT				2
#define CHAR_EVT				3
#define STEPPER_EVT				4
#define TIMER_EVT				5

typedef struct {
	uint16_t	channel;
	uint16_t	value;
} ADC_Event_t;

typedef struct {
	uint16_t	id;				/* buffer id 0/1 */
	uint16_t	size;			/* number of data elements */
} ADC_DMA_Event_t;

typedef struct {
	char		c;				/* ascii code */
	uint16_t	keycode;		/* special key identifier */
} Char_Event_t;

typedef struct {
	uint32_t	status;			/* status = 1 : done */
} Stepper_Event_t;

typedef  struct {
	uint32_t	id;				/* timer id */
} Timer_Event_t;

typedef struct {
	uint32_t			type;
	union {
		ADC_Event_t		adc;
		ADC_DMA_Event_t	adc_dma;
		Char_Event_t	key;
		Stepper_Event_t stepper;
		Timer_Event_t	timer;
	};
} Event_t;


/* push an event to the event queue */
bool push_event(const Event_t *evt);

/* pop an event from the event queue */
bool pop_event(Event_t *evt);

#endif
