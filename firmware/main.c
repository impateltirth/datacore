/* DataCore application: frame half-buffers and stream them over UART.
 *
 * Frame format (little-endian):
 *   [0xAA 0x55] [len: u16] [samples: u16 × len] [crc16]
 * A host script can sync on the 0xAA 0x55 preamble and plot the stream.
 */

#include "adc_dma.h"

/* TODO: implement with your UART handle, e.g.:
 *   HAL_UART_Transmit_DMA(&huart2, (uint8_t *)frame, frame_len);
 * Make sure the previous TX finished before starting the next one. */
static void uart_send(const uint8_t *data, uint32_t len)
{
    (void)data; (void)len;
}

static uint16_t crc16(const uint8_t *data, uint32_t len)
{
    uint16_t crc = 0xFFFF;
    for (uint32_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t b = 0; b < 8; b++)
            crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : (crc << 1);
    }
    return crc;
}

void DataCore_StreamHalf(const uint16_t *samples, uint32_t len)
{
    /* Static frame buffer: 2 preamble + 2 len + N*2 samples + 2 crc */
    static uint8_t frame[4 + 2 * DATACORE_HALF_BUFFER_LEN + 2];
    uint32_t i = 0;
    frame[i++] = 0xAA;
    frame[i++] = 0x55;
    frame[i++] = (uint8_t)(len & 0xFF);
    frame[i++] = (uint8_t)((len >> 8) & 0xFF);
    for (uint32_t s = 0; s < len; s++) {
        frame[i++] = (uint8_t)(samples[s] & 0xFF);
        frame[i++] = (uint8_t)((samples[s] >> 8) & 0xFF);
    }
    uint16_t crc = crc16(&frame[2], i - 2);
    frame[i++] = (uint8_t)(crc & 0xFF);
    frame[i++] = (uint8_t)((crc >> 8) & 0xFF);
    uart_send(frame, i);
}

int main(void)
{
    /* TODO: HAL_Init(), SystemClock_Config(), MX_GPIO_Init(), etc. */
    DataCore_Init();
    DataCore_Start();

    for (;;) {
        /* All streaming happens in DMA callbacks; the main loop is free
         * for command handling, e.g. changing sample rate on demand. */
        if (DataCore_HalfReady()) {
            /* Optional: tap the ready buffer here (stats, triggering). */
            DataCore_ReleaseBuffer();
        }
    }
    return 0;
}
