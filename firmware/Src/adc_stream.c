#include "adc_stream.h"

uint16_t adc_buf[2][DC_N_CHANNELS];
volatile uint8_t frame_ready[2] = {0, 0};

static ADC_HandleTypeDef *s_hadc;

/* HAL weak callbacks overridden here — do not define them elsewhere. */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc == s_hadc)
        frame_ready[0] = 1;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc == s_hadc)
        frame_ready[1] = 1;
}

void adc_stream_init(ADC_HandleTypeDef *hadc)
{
    s_hadc = hadc;
}

void adc_stream_start(void)
{
    HAL_ADCEx_Calibration_Start(s_hadc);
    HAL_ADC_Start_DMA(s_hadc, (uint32_t *)adc_buf, 2 * DC_N_CHANNELS);
}
