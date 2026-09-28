#ifndef DATACORE_CONFIG_H
#define DATACORE_CONFIG_H

/*
 * DataCore configuration — single place for every tunable.
 * Target: STM32F103C8T6, ADC1 scanned by TIM2 trigger, DMA circular,
 * framed packets streamed over USART1.
 */

/* ---- Acquisition ---- */
#define DC_N_CHANNELS        4      /* ADC1_IN0..IN3 on PA0..PA3 */
#define DC_SAMPLE_RATE_HZ    10000  /* per-channel, set by TIM2 */

/* ---- Streaming ---- */
#define DC_UART_BAUD         921600
#define DC_STREAM_DECIMATION 2      /* stream every Nth frame */

/*
 * Packet format (little-endian):
 *   AA 55 | seq u16 | ch0..chN u16 | CRC-8 (poly 0x07, over all prior bytes)
 */
#define DC_BYTES_PER_FRAME   (5 + 2*DC_N_CHANNELS)
#define DC_STREAM_RATE_HZ    (DC_SAMPLE_RATE_HZ / DC_STREAM_DECIMATION)

/*
 * Wire cost is 10 bits/byte (8N1). Refuse to build if the stream would
 * need more than 80% of the UART bandwidth — raise the baud rate or
 * the decimation instead of silently dropping frames.
 *
 * Default: 13 bytes * 5000 Hz * 10 = 650 kbps wire vs 921600 baud -> OK.
 */
#if (DC_BYTES_PER_FRAME * DC_STREAM_RATE_HZ * 10 > DC_UART_BAUD * 8 / 10)
#error "UART baud too low for configured throughput: raise DC_UART_BAUD or DC_STREAM_DECIMATION"
#endif

#endif /* DATACORE_CONFIG_H */
