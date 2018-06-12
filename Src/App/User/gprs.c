#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "sim800.h"
#include "bsp_usart.h"
void vTaskCodeGPRS( void * pvParameters )
{
    (void)pvParameters;
    BSP_USART_Init(); //初始化模块通信串口
    Gsm_TurnON();      //模块开机

    Gsm_Init();
    while(1)
    {
        //USART2_Tx("AT\r\n",4);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}