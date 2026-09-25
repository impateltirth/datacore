#ifndef ADC_DMA_H
#define ADC_DMA_H

#include <stdint.h>
#include <stdbool.h>

/* Number of samples per DMA half-buffer. Total buffer = 2 × this. */
#ifndef DATACORE_HALF_BUFFER_LEN
#define DATACORE_HALF_BUFFER_LEN 512
#endif

void     DataCore_Init(void);   /* configure ADC + DMA + UART (called once) */
void     DataCore_Start(void);  /* begin continuous ADC-DMA acquisition */
void     DataCore_Stop(void);
bool     DataCore_HalfReady(void);  /* true when a fresh half-buffer is ready */
uint16_t *DataCore_GetBuffer(void); /* pointer to the ready half-buffer */
void     DataCore_ReleaseBuffer(void);

/* Board-specific hook: stream one framed half-buffer over UART.
 * Implemented in main.c (framing) — the driver calls it from the
 * DMA transfer-complete context. Keep it short: start a UART DMA
 * transfer and return. */
void DataCore_StreamHalf(const uint16_t *samples, uint32_t len);

#endif /* ADC_DMA_H */
