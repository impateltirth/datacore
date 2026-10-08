#include "uart_packet.h"

static UART_HandleTypeDef *s_huart;
static uint8_t tx_buf[DC_BYTES_PER_FRAME];
static volatile uint8_t tx_busy = 0;
static uint16_t seq = 0;
static uint32_t dropped = 0;

/* HAL weak callback overridden here — do not define it elsewhere. */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == s_huart)
        tx_busy = 0;
}

static uint8_t crc8(const uint8_t *data, uint16_t len)
{
    uint8_t crc = 0x00, i, b;
    for (i = 0; i < len; i++) {
        crc ^= data[i];
        for (b = 0; b < 8; b++)
            crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x07)
                               : (uint8_t)(crc << 1);
    }
    return crc;
}

void uart_packet_init(UART_HandleTypeDef *huart)
{
    s_huart = huart;
}

void uart_packet_send_frame(const uint16_t *samples)
{
    uint8_t *p = tx_buf, ch;

    if (tx_busy) {
        dropped++;
        return;
    }

    *p++ = 0xAA;
    *p++ = 0x55;
    *p++ = (uint8_t)(seq & 0xFF);
    *p++ = (uint8_t)(seq >> 8);
    for (ch = 0; ch < DC_N_CHANNELS; ch++) {
        *p++ = (uint8_t)(samples[ch] & 0xFF);
        *p++ = (uint8_t)(samples[ch] >> 8);
    }
    *p = crc8(tx_buf, (uint16_t)(p - tx_buf));
    seq++;

    tx_busy = 1;
    if (HAL_UART_Transmit_DMA(s_huart, tx_buf, DC_BYTES_PER_FRAME) != HAL_OK) {
        // Do not deadlock the stream if DMA startup fails (for example if the
        // HAL peripheral state is unexpectedly busy).
        tx_busy = 0;
        dropped++;
    }
}

uint32_t uart_packet_dropped(void)
{
    return dropped;
}
