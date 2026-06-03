/****************************************************************/
#include "event.h"
#include "uart.h"

/* Event queue handling */
#define MAX_EVQ			10

volatile Event_t evq[MAX_EVQ];
volatile int evq_rd=0, evq_wr=0;

/* push an event to the queue */
bool push_event(const Event_t *evt)
{
	__set_PRIMASK(1);
	if ((evq_wr+1)%MAX_EVQ != evq_rd) {
		evq[evq_wr] = *evt;
		evq_wr = (evq_wr+1)%MAX_EVQ;
		__set_PRIMASK(0);
		return true;
	}
	__set_PRIMASK(0);
	return false;
}

/* pop an event from the queue */
bool pop_event(Event_t *evt)
{
	char c;
	if (uart_getchar(_USART2, &c)) {
		evt->type=CHAR_EVT;
		evt->key.c=c;
		evt->key.keycode=0;
		return true;
	} else if (evq_rd!=evq_wr) {
		*evt = evq[evq_rd];
		evq_rd = (evq_rd+1)%MAX_EVQ;
		return true;
	}
	return false;
}

