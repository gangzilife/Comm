#include "FreeRTOS.h"
#include "task.h"

#include "dhcp.h"
#include "stm32f4xx_hal.h"

#include "mqtt_interface.h"
void vApplicationTickHook( void )
{
    HAL_IncTick();
    DHCP_time_handler();
    //MilliTimer_Handler();
}

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
    (void)xTask;
    (void)pcTaskName;
}

//void SysTick_Handler(void)
//{
//    HAL_IncTick();
//}



