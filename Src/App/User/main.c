#include "includes.h"

#define DHCP_SOCKET   0

void vTaskCodeComm( void * pvParameters );

SemaphoreHandle_t xGprsMutex;

__ALIGN_BEGIN USB_OTG_CORE_HANDLE  USB_OTG_dev __ALIGN_END;

int main (void)
{
    HAL_Init();
    BSP_SystemClkCfg();
    BSP_Init();

    xGprsMutex = xSemaphoreCreateMutex();
    assert(xGprsMutex != NULL);
    
    TaskHandle_t xCommHandle;
    BaseType_t errComm = xTaskCreate( vTaskCodeComm,"comm",256,NULL,2,&xCommHandle);
    assert(errComm == pdPASS);
    
    TaskHandle_t xGprsHandle;
    BaseType_t errGprs = xTaskCreate( vTaskCodeGPRS,"gprs",256,NULL,2,&xGprsHandle);
    assert(errGprs == pdPASS);
    
    vTaskStartScheduler();
    return 0;
}

uint8_t ip[4] = {0};
uint8_t dhcp_buf[1024];
void vTaskCodeComm( void * pvParameters )
{
    (void)pvParameters;
    /*先初始化SIMCOM模块，再初始化W5500*/
   
//    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);
//    vTaskDelay(pdMS_TO_TICKS(100));
//    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_SET);
//    vTaskDelay(pdMS_TO_TICKS(500));
    
    wizchip_init(NULL,NULL);
    DHCP_init(DHCP_SOCKET,dhcp_buf);
    while(1)
    {
        if(DHCP_run() == DHCP_IP_LEASED)
        {
            DHCP_stop();
            getIPfromDHCP(ip);
            TaskHandle_t xEnternetHandle;
            BaseType_t err = xTaskCreate( vTaskCodeMQTT,"EnternetMQTT",256,NULL,3,&xEnternetHandle);
            assert(err == pdPASS);
        }
//        if(report_ok)
//        {
//            USBD_CUSTOM_HID_SendReport(&USB_OTG_dev,Report_buf,64);
//            report_ok = 0;
//        }
        
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

