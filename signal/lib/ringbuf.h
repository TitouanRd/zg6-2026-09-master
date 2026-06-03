#ifndef _RINGBUF_H_
#define _RINGBUF_H_

#include <stdint.h>
#include <stdbool.h>


#define RGBF_MAX_SIZE		64

typedef volatile struct {
	uint8_t		buf[RGBF_MAX_SIZE];
	int32_t		i_r;
	int32_t		i_w;
} RGBF;

#define RGBF_INIT			{.i_r=0, .i_w=0}

/* ring buffer is full/empty test */
bool rgbf_is_full(RGBF *r);
bool rgbf_is_empty(RGBF *r);

/* rgbf_send_data: write data in ring buffer */
void rgbf_send_data(RGBF *r, uint8_t data);

/* rgbf_get_data: returns 1 data from ring buffer */
uint8_t rgbf_get_data(RGBF *r);

#endif
