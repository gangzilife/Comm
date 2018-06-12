#include "MQTTClient.h"
#include "sim800.h"
#include "bsp_usart.h"
#include "mqtt_gprs_interface.h"

static void messageArrived(MessageData* data)
{
//	printf("%.*s: %.*s\n", data->topicName->lenstring.len, data->topicName->lenstring.data,
//           data->message->payloadlen, data->message->payload);
    
    uint8_t* pdate = (uint8_t*)data->message->payload;
    for(int i = 0 ; i < data->message->payloadlen ; i++)
    {
        printf("%02X ",pdate[i]);
    }
    printf("\r\n");
}


MQTTClient gprsclient;
void vTaskCodeGPRS( void * pvParameters )
{
    (void)pvParameters;
    BSP_USART_Init(); //初始化模块通信串口
    Gsm_TurnON();      //模块开机
    printf("turn on ok\r\n");
    Gsm_Init();
    
	Network gprs_network;
    
	uint8_t sendbuf[80], readbuf[80];
	int rc = 0;

	MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;

    gprs_NewNetwork(&gprs_network,1);
    MQTTClientInit(&gprsclient, &gprs_network, 3000,sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

	char address[] = "114.55.56.64";
	if ((rc = gprs_ConnectNetwork(&gprs_network, (char*)address, 1883)) != 0)
		printf("Return code from network connect is %d\n", rc);

#if defined(MQTT_TASK)
	if ((rc = MQTTStartTask(&gprsclient)) != pdPASS)
		printf("Return code from start tasks is %d\n", rc);
#endif
    
    connectData.MQTTVersion = 3;
	connectData.clientID.cstring = "GPRS_MQTTClient";

	if ((rc = MQTTConnect(&gprsclient, &connectData)) != 0)
		printf("Return code from MQTT connect is %d\n", rc);
	else
		printf("MQTT Connected\n");

	if ((rc = MQTTSubscribe(&gprsclient, "sensor", QOS0, messageArrived)) != 0)
		printf("Return code from MQTT subscribe is %d\n", rc);
   
	while (1)
	{
#if !defined(MQTT_TASK)
		if ((rc = MQTTYield(&gprsclient, 1000)) != 0)
			printf("Return code from yield is %d\n", rc);
#endif
        if(!gprsclient.isconnected)
        {
             printf("MQTT Disconnect\r\n");//MQTT Disconnect,reconnect
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
	}
}