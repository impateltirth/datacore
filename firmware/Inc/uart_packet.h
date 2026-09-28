#ifndef UART_PACKET_H
#define UART_PACKET_H

#include "stm32f1xx_hal.h"
#include "datacore_config.h"

/*
 * Framed packet TX over USART1 using DMA.
 * uart_packet_send_frame() is non-blocking: if the previous DMA transfer
 * is still running, the frame is dropped and counted (backpressure is
 * explicit, never silent corruption). See uart_packet_dropped().
 */
void     uart_packet_init(UART_HandleTypeDef *huart);
void     uart_packet_send_frame(const uint16_t *samples);
uint32_t uart_packet_dropped(void);

#endif /* UART_PACKET_H */
