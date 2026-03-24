#ifndef _I2C_H_
#define _I2C_H_

#ifdef __cplusplus
extern "C" {
#endif 

#include <stdbool.h>

#include "include/board.h"
#include "lib/dma.h"

/* ADC
 * 
 * 1. Initialize the ADC for regular channels only
 *
 *    adc_init(adc, mode, cb)
 *    
 *    adc can be configured in
 *    - single scan mode: only 1 scan is triggered
 *    - continuous scan mode: inputs are continuously resampled
 *    - triggered scan mode: the sampling is triggered by an external source
 *      (a timer). Extra information on the triggering source is given by
 *      adc_set_trigger_event.
 *
 *    The ADC resolution can be configured as well: 12, 10, 8 or 6 bits.
 *
 *
 * 2. Add channels to the scan loop: each time the sampling is triggered, all
 *    the channels enabled are sampled according to steadying time configured.
 *    
 *    adc_channel_enable(adc, channel, ADC_SMPL_480CYCLES);
 *
 *    Channel configuration
 *    - each call add the given channel to the scan loop. Up to 16 channels
 *      can be added among the 18 available channels: 0 to 15 for regular 
 *      channels, 16 for internal temperature sensor channel, 17 for the
 *      ADC reference channel and 18 for the Battery monitoring channel.
 *    - a channel can be added several times, but the sample time is a 
 *      channel parameter ie only the last configured value will be used
 *      for all the instances of the same channel in the scan sequence.
 *
 *
 * 3. In triggered mode, set the triggering source
 *
 *    adc_set_trigger_evt(adc, triggering_event)
 *
 *    Only one triggering event can be used at a time.
 *
 *
 * 4. Start sampling
 *
 *    adc_start(adc)
 *
 *    In continuous mode, the ADC has to be stopped manually.
 *
 *    adc_stop(adc)
 *
 *
 * Alternatively, a simpler method to sample a single channel 
 *
 *    uint16_t adc_channel_sample(ADC_t *adc, uint32_t channel, uint32_t sample_time);
 *
 *    If a callback was defined during ADC init, an IRQ is set up, calling
 *    the callback on the end of conversion event.
 *    If not, the ADC is used in polling mode, the sampled value returned by
 *    the function
 *
 *
 * Helper functions:
 *
 *    adc_get_sampling_freq(adc)
 *
 *      returns the sampling frequency by channel. The more the channels in 
 *      the scan loop, the less the sampling frequency is high. The frequency
 *      calculation is relevant for continuous and timer triggered modes only.
 *
 *    adc_sample_cycles(adc, uint32_t smpltime_us)
 *
 *      returns the sample time configuration (that can be used by
 *      adc_channel_enable) to meet the time constraint or 0xFFFFFFFF if it 
 *      cannot be met.
 */

/* ADC modes */
#define ADC_MODE_12BITS					(0<<24)
#define ADC_MODE_10BITS					(1<<24)
#define ADC_MODE_8BITS					(2<<24)
#define ADC_MODE_6BITS					(3<<24)

#define ADC_MODE_SINGLE					(0)
#define ADC_MODE_CONTINUOUS				(1)
#define ADC_MODE_TRIGGERED				(2)
#define ADC_MODE_DMA					(4)

/* ADC channel 0 to 15 for regular input channels + specific channels */
#define ADC_CHANNEL_0					0
#define ADC_CHANNEL_1					1
#define ADC_CHANNEL_2					2
#define ADC_CHANNEL_3					3
#define ADC_CHANNEL_4					4
#define ADC_CHANNEL_5					5
#define ADC_CHANNEL_6					6
#define ADC_CHANNEL_7					7
#define ADC_CHANNEL_8					8
#define ADC_CHANNEL_9					9
#define ADC_CHANNEL_10					10
#define ADC_CHANNEL_11					11
#define ADC_CHANNEL_12					12
#define ADC_CHANNEL_13					13
#define ADC_CHANNEL_14					14
#define ADC_CHANNEL_15					15
#define ADC_CHANNEL_TEMP				16
#define ADC_CHANNEL_VREFINT				17
#define ADC_CHANNEL_VBAT				18


/* ADC sample timing 
 * for ADC_MODE_12BIT mode and ADC_SMPL_28_CYCLES, conversion time is
 * 12 + 28 = 40 cycles long
 */
#define	ADC_SMPL_3CYCLES				0
#define	ADC_SMPL_15CYCLES				1
#define	ADC_SMPL_28CYCLES				2
#define	ADC_SMPL_56CYCLES				3
#define	ADC_SMPL_84CYCLES				4
#define	ADC_SMPL_112CYCLES				5
#define	ADC_SMPL_144CYCLES				6
#define	ADC_SMPL_480CYCLES				7


/* ADC triggering events */
#define ADC_TRIG_ON_TIM1_CH1			(0x10)
#define ADC_TRIG_ON_TIM1_CH2			(0x11)
#define ADC_TRIG_ON_TIM1_CH3			(0x12)
#define ADC_TRIG_ON_TIM2_CH2			(0x13)
#define ADC_TRIG_ON_TIM2_CH3			(0x14)
#define ADC_TRIG_ON_TIM2_CH4			(0x15)
#define ADC_TRIG_ON_TIM2_TRGO			(0x16)
#define ADC_TRIG_ON_TIM3_CH1			(0x17)
#define ADC_TRIG_ON_TIM3_TRGO			(0x18)
#define ADC_TRIG_ON_TIM4_CH4			(0x19)
#define ADC_TRIG_ON_TIM5_CH1			(0x1A)
#define ADC_TRIG_ON_TIM5_CH2			(0x1B)
#define ADC_TRIG_ON_TIM5_CH3			(0x1C)
#define ADC_TRIG_ON_PA11_RISING			(0x1F)
#define ADC_TRIG_ON_PA11_FALLING		(0x2F)
#define ADC_TRIG_ON_PA11_BOTH			(0x3F)
#define ADC_TRIG_ON_PC11_RISING			(0x21F)
#define ADC_TRIG_ON_PC11_FALLING		(0x22F)
#define ADC_TRIG_ON_PC11_BOTH			(0x23F)


typedef void (*OnSample)(uint16_t channel, uint16_t val, bool ovr);

/* adc_init
 *   intialize ADC, returns adc_freq
 */
int adc_init(ADC_t *adc, uint32_t mode, OnSample cb);

/* adc_channel_sample
 *   sample the specified channel and generate an interrupt when it is done
 *   if a callback was defined during initialization
 */
uint16_t adc_channel_sample(ADC_t *adc, uint32_t channel, uint32_t sample_time);

/* adc_channel_enable
 *   add adc channel to the sampling sequence
 *   sample _time is a channelwise parameter, ie a channel can't
 *   be scanned several times in a sequence width different
 *   sampling time. The last defined will prevail.
 */
int adc_channel_enable(ADC_t *adc, uint32_t channel, uint32_t sample_time);

/* adc_channel_disable
 *   disable last occurence of channel in the sequence
 */
int adc_channel_disable(ADC_t *adc, uint32_t channel);

/* adc_channel_reset
 *   reset the sequence
 */
int adc_channel_reset(ADC_t *adc);

/* adc_set_trigger_evt
 *   set the timer trigger event to trigger a sampling scan loop
 */
int adc_set_trigger_evt(ADC_t *adc, uint32_t evt);

/* adc_setup_dma
 *   setup DMA in double-buffer, and set end of transfer interrupt callback
 */
int adc_setup_dma(ADC_t *adc, void *buf, uint16_t bufsize, OnTC cb);

/* adc_start
 *   start sampling
 */
void adc_start(ADC_t *adc);

/* adc_stop
 *   stop sampling
 */
void adc_stop(ADC_t *adc);

/* adc_get_sampling_freq
 *   get the actual sampling frequency
 */
uint32_t adc_get_sampling_freq(ADC_t *adc);

/* adc_sample_cycles
 *   get the best sample time configuration from time constraint
 *   smpltime_us_max = 480/adc_freq*1e6 [us]
 */
uint32_t adc_sample_cycles(ADC_t *adc, uint32_t smpltime_us);


#ifdef __cplusplus
}
#endif
#endif
