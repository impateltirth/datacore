/* DataCore — STM32 ADC + DMA + UART streaming driver.
 *
 * Design: ADC1 runs in continuous mode, DMA in circular double-buffer mode.
 * The CPU only handles buffer-ready flags; samples flow ADC -> DMA -> UART
 * without per-sample CPU work.
 *
 * PORTING: this file is written against STM32 HAL. Generate ADC1, DMA and
 * UART handles with STM32CubeMX and wire them up at the TODO markers below.
 */

#include "adc_dma.h"

/* TODO: replace these with your CubeMX-generated handles, e.g.:
 *   extern ADC_HandleTypeDef hadc1;
 *   extern UART_HandleTypeDef huart2;
 */
#if 0
extern ADC_HandleTypeDef hadc1;
extern UART_HandleTypeDef huart2;
#endif

static uint16_t dma_buffer[2 * DATACORE_HALF_BUFFER_LEN];
static volatile uint8_t ready_half = 0xFF; /* 0, 1, or 0xFF = none ready */

void DataCore_Init(void)
{
    /* TODO: CubeMX should generate MX_ADC1_Init(), MX_DMA_Init(),
     * MX_USART2_UART_Init(). If you hand-roll init, do it here:
     *   - ADC: continuous conversion, DMA continuous requests enabled
     *   - DMA: peripheral-to-memory, circular mode, half-word width
     *   - UART: 115200 8N1 (raise baud for high sample rates)
     */
}

void DataCore_Start(void)
{
    ready_half = 0xFF;
    /* TODO: HAL_ADC_Start_DMA(&hadc1, (uint32_t *)dma_buffer,
     *                          2 * DATACORE_HALF_BUFFER_LEN); */
}

void DataCore_Stop(void)
{
    /* TODO: HAL_ADC_Stop_DMA(&hadc1); */
}

bool DataCore_HalfReady(void)
{
    return ready_half != 0xFF;
}

uint16_t *DataCore_GetBuffer(void)
{
    if (ready_half == 0xFF)
        return 0;
    return &dma_buffer[ready_half * DATACORE_HALF_BUFFER_LEN];
}

void DataCore_ReleaseBuffer(void)
{
    ready_half = 0xFF;
}

/* --- DMA callbacks: called from ISR context --- */

void DataCore_OnHalfComplete(void)  /* TODO: call from HAL_ADC_ConvHalfCpltCallback */
{
    ready_half = 0;
    DataCore_StreamHalf(&dma_buffer[0], DATACORE_HALF_BUFFER_LEN);
}

void DataCore_OnFullComplete(void)  /* TODO: call from HAL_ADC_ConvCpltCallback */
{
    ready_half = 1;
    DataCore_StreamHalf(&dma_buffer[DATACORE_HALF_BUFFER_LEN],
                        DATACORE_HALF_BUFFER_LEN);
}
