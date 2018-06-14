#ifndef _DATA_ANALYSIS_H_
#define _DATA_ANALYSIS_H_

#include <stdint.h>
#include "bsp_usart.h"
void Get_DeviceID(void);
void Data_decode(uint8_t* buf ,uint16_t len);
void Data_code(uint8_t* inbuf ,uint16_t inlen , uint8_t* outbuf , uint8_t *outlen);

#endif