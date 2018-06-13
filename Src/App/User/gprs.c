#include "MQTTClient.h"
#include "sim800.h"
#include "mqtt_gprs_interface.h"
#include "data_analysis.h"
#include <assert.h>


volatile SemaphoreHandle_t xGprsMutex;
static uint8_t gprs_buf[128] = {0};
static uint8_t gprs_len = 0;
/* MQTT服务器推送的消息 */
static void messageArrived(MessageData* data)
{
//	printf("%.*s: %.*s\n", data->topicName->lenstring.len, data->topicName->lenstring.data,
//           data->message->payloadlen, data->message->payload);
    //前六个字节 01 06 97 EC 00 00
    uint8_t* pdate = (uint8_t*)data->message->payload;
    memcpy(gprs_buf,pdate,data->message->payloadlen);
    gprs_len = data->message->payloadlen;
    if(gprs_buf[1] != 0x06)//不是发给洞口灯的
        return;
    else
        Data_decode(gprs_buf,gprs_len);
}


MQTTClient gprsclient;
void vTaskCodeGPRS( void * pvParameters )
{
    xGprsMutex = xSemaphoreCreateMutex();
    assert(xGprsMutex != NULL);
    
    (void)pvParameters;
    BSP_USART_Init(); //初始化模块通信串口
    Gsm_TurnON();      //模块开机


    
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
        if(Gsm_Init() != 0)
            continue;
        else
            break;   
    }
	Network gprs_network;
    
	uint8_t sendbuf[80], readbuf[80];
	int rc = 0;

	MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;

    gprs_NewNetwork(&gprs_network,1);
    MQTTClientInit(&gprsclient, &gprs_network, 3000,sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

	char address[] = "114.55.56.64";
    //char address[] = "218.244.156.4";
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
        if ((rc = gprs_ConnectNetwork(&gprs_network, (char*)address, 1883)) != 0)
            continue;      
        else
            break;          
    }
#if defined(MQTT_TASK)
	if ((rc = MQTTStartTask(&gprsclient)) != pdPASS)
		printf("Return code from start tasks is %d\n", rc);
#endif
    
    connectData.MQTTVersion = 3;
	connectData.clientID.cstring = "GPRS_MQTTClient";
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
        if ((rc = MQTTConnect(&gprsclient, &connectData)) != 0)
            continue; 
        else
            break;   
    }

    printf("MQTTConnect OK \r\n");
	if ((rc = MQTTSubscribe(&gprsclient, "O/60567", QOS0, messageArrived)) != 0)
		printf("Return code from MQTT subscribe is %d\n", rc);
   
    uint8_t uart_buf[128] = {0};
    uint8_t uart_len = 0;
    uint8_t ret = 0;
    
    uint8_t gprs_mqtt_buf[128] = {0};
    uint8_t gprs_mqtt_len = 0;
	while (1)
	{
#if !defined(MQTT_TASK)
		if ((rc = MQTTYield(&gprsclient, 1000)) != 0)
			printf("Return code from yield is %d\n", rc);
#endif
        if(!gprsclient.isconnected)
        {
             //MQTT Disconnect,reconnect
        }
        do
        {
            vTaskDelay(pdMS_TO_TICKS(10));
            ret = USART1_Rx(uart_buf + uart_len ,sizeof(uart_buf) - uart_len);
            uart_len += ret;
        }while(ret != 0);
        if(uart_len != 0)//串口接受到数据，解析，组包，完后发布
        {
            Data_code(uart_buf,uart_len ,gprs_mqtt_buf,&gprs_mqtt_len);
            uart_len = 0;
            
            USART1_Tx(gprs_mqtt_buf,gprs_mqtt_len);
//            MQTTMessage message;
//            message.qos = QOS0;
//            message.retained = 0;
//            message.payload = gprs_mqtt_buf;
//            message.payloadlen = gprs_mqtt_len;
//
//            if ((rc = MQTTPublish(&gprsclient, "I/ZJ", &message)) != 0)
//                printf("Return code from MQTT publish is %d\n", rc);
        }
        vTaskDelay(pdMS_TO_TICKS(50));
	}
}