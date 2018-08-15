#include "data_analysis.h"

/* 平台定义的相关命令ID */

/* 下发命令 */
#define GETLIGHTSTATUS   0x0B   //1个参数id（char）
#define SETLIGHTSTATUS   0x0C   //6个参数id（都为char）
#define SETCONTROLFREQ   0x0D   //3个参数id（short,char,char）
#define GETCONTROLINFO   0x0E   //一个参数id（char）
#define GETDEVICESTATUS  0x0F   //一个参数id（char）
#define DEVICERESET      0x10   //一个参数id（char）
#define ROADISERR        0x11   //一个参数id（char）
/* 解析命令 */

//id	名称	单位	                            数据类型	分类
#define local_id               0			        //short
#define WhiteLightMode          11			        //char
#define WhiteLightValue         12			        //char
#define YellowLightMode         13			        //char
#define YellowLightValue        14			        //char
#define RedLightMode            15			        //char
#define RedLightValue           16			        //char
#define SetOK                   17                  //char
#define ControlFreq             18			        //short
#define Threshold1              19			        //char
#define Threshold2              20                  //char
#define EnvironmentLight        21			        //int
#define AutoOrManual            22			        //char
#define Current1                23			        //int
#define Current2                24			        //int
#define Current3                25			        //int
#define CarNum                  27			        //int


typedef struct {
	uint16_t head;					// 帧头0xAA, 0x55
	uint16_t length;				// 数据长度: 数据帧中所有数据长度
	uint16_t cmd;					// 命令
	uint16_t srcID;				    // 源ID
	uint16_t destID;				// 目的ID
	uint16_t number;				// 帧序号
	uint8_t data[308];  			// 数据: 最长不得超过308字节(包括校验位)
}*pDataFrame_t, DataFrame_t;

enum {
	// 0x0030--0x004F
	CMD_GET_ID = 0x0030,				// 0x0030 查询ID
	CMD_SET_ID,				       // 0x0031 设置ID
	CMD_GET_LIGHT,					// 0x0032 查询灯光
	CMD_SET_LIGHT,					// 0x0033 设置灯光
	CMD_GET_CONTFREQ,					// 0x0034 查询控制频率
	CMD_SET_CONTFREQ,				   // 0x0035 设置控制频率
	CMD_GET_STATUS,				   // 0x0036 查询设备运行状态
	CMD_UP_DATA,				      // 0x0037 上报数据（不用应答）
	CMD_RESET,				         // 0x0038 复位
    CMD_ROADERR				         // 0x0039 道路异常
};
enum {
	ACK_GET_ID = 0x8030,				// 0x8030 查询ID应答
	ACK_SET_ID,						// 0x8031 设置ID应答
	ACK_GET_LIGHT,				   // 0x8032 查询灯光应答
	ACK_SET_LIGHT,				   // 0x8033 设置灯光应答
	ACK_GET_CONTFREQ,			      //  0x8034 查询控制频率应答
	ACK_SET_CONTFREQ,			      //  0x8035 设置控制频率应答
	ACK_GET_STATUS,               //  0x8036 查询设备运行状态应答
	ACK_RESET ,                   //  0x8038 复位应答
    ACK_ROADERR                    //  0x8039道路异常应答
};  
//static uint8_t get_datalen(uint8_t data)
//{
//    switch(data)
//    {
//        case 1: //char
//            return 1;
//        case 2: //short
//            return 2;
//        case 3: //int
//            return 4;
//        default:
//            return 0;
//    }
//}
uint16_t DeviceID = 0;

void Get_DeviceID(void)
{
    uint8_t buf[12] = {0xAA, 0x55, 0x0C, 0x00, 0x30, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x01, 0x00 };
    USART1_Tx(buf,sizeof(buf));
}
void Data_decode(uint8_t* buf ,uint16_t len)
{
    static uint16_t data_sn = 0;
    uint8_t* pdata = buf;
    DataFrame_t dataframe = {0};
    switch (pdata[7])
    {
        case GETLIGHTSTATUS :
        dataframe.head   = 0x55AA;
        dataframe.length = 12;
        dataframe.cmd    = CMD_GET_LIGHT;
        dataframe.srcID  = 0;
        dataframe.destID = 0xFFFF;
        dataframe.number = data_sn++;
        USART1_Tx((uint8_t*)&dataframe,dataframe.length);
        break;
        case SETLIGHTSTATUS :
        dataframe.head   = 0x55AA;
        dataframe.length = 20;
        dataframe.cmd    = CMD_SET_LIGHT;
        dataframe.srcID  = 0;
        dataframe.destID = 0xFFFF;
        dataframe.number = data_sn++;
        /* 白光 */
        dataframe.data[0]= pdata[11];
        dataframe.data[1]= pdata[15];
        /* 黄光 */
        dataframe.data[2]= pdata[19];
        dataframe.data[3]= pdata[23];
        /* 红光 */
        dataframe.data[4]= pdata[27];
        dataframe.data[5]= pdata[31];
        /* 控制时常 */
        dataframe.data[6]= pdata[35];
        dataframe.data[7]= pdata[36];
        USART1_Tx((uint8_t*)&dataframe,dataframe.length);
        break;
        case SETCONTROLFREQ :
        dataframe.head   = 0x55AA;
        dataframe.length = 16;
        dataframe.cmd    = CMD_SET_CONTFREQ;
        dataframe.srcID  = 0;
        dataframe.destID = 0xFFFF;
        dataframe.number = data_sn++;
        /* 闪烁频率 */
        dataframe.data[0]= pdata[11];
        dataframe.data[1]= pdata[12];
        /* 两个控制阈值 */
        dataframe.data[2]= pdata[16];
        dataframe.data[3]= pdata[20];
        USART1_Tx((uint8_t*)&dataframe,dataframe.length);
        break;
        case GETCONTROLINFO :
        dataframe.head   = 0x55AA;
        dataframe.length = 12;
        dataframe.cmd    = CMD_GET_CONTFREQ;
        dataframe.srcID  = 0;
        dataframe.destID = 0xFFFF;
        dataframe.number = data_sn++;
        USART1_Tx((uint8_t*)&dataframe,dataframe.length);
        break;
        case GETDEVICESTATUS :
        dataframe.head   = 0x55AA;
        dataframe.length = 12;
        dataframe.cmd    = CMD_GET_STATUS;
        dataframe.srcID  = 0;
        dataframe.destID = 0xFFFF;
        dataframe.number = data_sn++;
        USART1_Tx((uint8_t*)&dataframe,dataframe.length);
        break;
        case DEVICERESET :
        dataframe.head   = 0x55AA;
        dataframe.length = 12;
        dataframe.cmd    = CMD_RESET;
        dataframe.srcID  = 0;
        dataframe.destID = 0xFFFF;
        dataframe.number = data_sn++;
        USART1_Tx((uint8_t*)&dataframe,dataframe.length);
        break;
        case ROADISERR:
        dataframe.head   = 0x55AA;
        dataframe.length = 13;
        dataframe.cmd    = CMD_ROADERR;
        dataframe.srcID  = 0;
        dataframe.destID = 0xFFFF;
        dataframe.number = data_sn++;
        dataframe.data[0]= pdata[11];
        USART1_Tx((uint8_t*)&dataframe,dataframe.length);
        default : break;       
    }
}


void Data_code(uint8_t* inbuf ,uint16_t inlen , uint8_t* outbuf , uint8_t *outlen)
{
    DataFrame_t* pbuf = (DataFrame_t*)inbuf;
    if(pbuf->cmd == ACK_GET_ID )
    {
        DeviceID = pbuf->data[0] | (pbuf->data[1] << 8);
        *outlen = 0;
        return ;
    }
    
    uint8_t index = 0;
    outbuf[0] = 0x01;
    outbuf[1] = 0x06;
    outbuf[2] = DeviceID & 0x00FF; //设备ID
    outbuf[3] = (DeviceID >> 8);
    outbuf[4] = 0x00;
    outbuf[5] = 0x00;
    outbuf[6] = 0x00;
    outbuf[7] = 0x00;
    outbuf[8] = 0x00;
    index = 9;
    switch(pbuf->cmd)
    {
        case ACK_GET_LIGHT :
        outbuf[index++] = WhiteLightMode;
        outbuf[index++] = pbuf->data[0];
        outbuf[index++] = WhiteLightValue;
        outbuf[index++] = pbuf->data[1];
        outbuf[index++] = YellowLightMode;
        outbuf[index++] = pbuf->data[2];
        outbuf[index++] = YellowLightValue;
        outbuf[index++] = pbuf->data[3];
        outbuf[index++] = RedLightMode;
        outbuf[index++] = pbuf->data[4];
        outbuf[index++] = RedLightValue;
        outbuf[index++] = pbuf->data[5];
        break;
        case ACK_SET_LIGHT :
        outbuf[index++] = SetOK;
        outbuf[index++] = pbuf->data[0];
        break;
        case ACK_GET_CONTFREQ :
        outbuf[index++] = ControlFreq;
        outbuf[index++] = pbuf->data[0];
        outbuf[index++] = pbuf->data[1];
        outbuf[index++] = Threshold1;
        outbuf[index++] = pbuf->data[2];
        outbuf[index++] = Threshold2;
        outbuf[index++] = pbuf->data[3];
        break;
        case ACK_SET_CONTFREQ :
        outbuf[index++] = SetOK;
        outbuf[index++] = pbuf->data[0];
        break;
        case ACK_GET_STATUS : //这个状态较多
        outbuf[index++] = CarNum;                       //车流量
        outbuf[index++] = pbuf->data[0];
        outbuf[index++] = pbuf->data[1];
        outbuf[index++] = WhiteLightValue;              //环境光照度
        outbuf[index++] = pbuf->data[2];
        outbuf[index++] = pbuf->data[3];
        outbuf[index++] = pbuf->data[4];
        outbuf[index++] = pbuf->data[5];
        outbuf[index++] = YellowLightMode;              //控制模式
        outbuf[index++] = pbuf->data[6];
        outbuf[index++] = WhiteLightMode;               //灯光信息
        outbuf[index++] = pbuf->data[7];
        outbuf[index++] = WhiteLightValue;
        outbuf[index++] = pbuf->data[8];
        outbuf[index++] = YellowLightMode;
        outbuf[index++] = pbuf->data[9];
        outbuf[index++] = YellowLightValue;
        outbuf[index++] = pbuf->data[10];
        outbuf[index++] = RedLightMode;
        outbuf[index++] = pbuf->data[11];
        outbuf[index++] = RedLightValue;
        outbuf[index++] = pbuf->data[12];      
        outbuf[index++] = ControlFreq;                  //控制信息
        outbuf[index++] = pbuf->data[13];
        outbuf[index++] = pbuf->data[14];
        outbuf[index++] = Threshold1;
        outbuf[index++] = pbuf->data[15];
        outbuf[index++] = Threshold2;
        outbuf[index++] = pbuf->data[16];
        for(int i = 0 ; i< 3;i++)
        {
            outbuf[index++] = Current1 + i;              //环境光照度
            outbuf[index++] = pbuf->data[17 + 4*i];
            outbuf[index++] = pbuf->data[18 + 4*i];
            outbuf[index++] = pbuf->data[19 + 4*i];
            outbuf[index++] = pbuf->data[20 + 4*i];            
        }
        break;
        case ACK_RESET :
        outbuf[index++] = SetOK;
        outbuf[index++] = pbuf->data[0];
        break;
        case ACK_ROADERR :
        outbuf[index++] = SetOK;
        outbuf[index++] = pbuf->data[0];
        break;
        default :break;
    }
    *outlen = index;
}