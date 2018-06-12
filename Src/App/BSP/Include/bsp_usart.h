#ifndef  BSP_USART_H_
#define  BSP_USART_H_

#include "stm32f4xx_hal.h"
#include "string.h"

void BSP_USART_Init(void);


void USART2_Tx(uint8_t* buf,uint16_t size);
uint16_t USART2_Rx(uint8_t* buf,uint16_t len);

#endif  