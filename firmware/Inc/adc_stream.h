#ifndef ADC_STREAM_H
#define ADC_STREAM_H

#include "stm32f1xx_hal.h"
#include "datacore_config.h"

/*
 * ADC1 in scan mode, triggered by TIM2 TRGO at DC_SAMPLE_RATE_HZ.
 * DMA1 Channel1 moves conversions into a circular double buffer:
 * half-transfer -> adc_buf[0] ready, full-transfer -> adc_buf[1] ready.
 * frame_ready[] flags are set in the DMA callbacks (see adc_stream.c).
 */
extern uint16_t adc_buf[2][DC_N_CHANNELS];
extern volatile uint8_t frame_ready[2];

void adc_stream_init(ADC_HandleTypeDef *hadc);
void adc_stream_start(void);   /* calibrates ADC, starts DMA + expects TIM2 running */

#endif /* ADC_STREAM_H */
