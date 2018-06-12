#include "includes.h"

void _sFlash_SPI_Init(void)
{
//  call in bsp_init
	BSP_SPI_Init();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
}

void _sFlash_SPI_Tx(const uint8_t* buf, uint16_t len)
{
	BSP_SPI_Tx(buf, len);
}

void _sFlash_SPI_Rx(uint8_t* buf, uint16_t len)
{
	BSP_SPI_Rx(buf, len);
}

void _sFlash_SPI_TxRx(const uint8_t* tx_buf, uint8_t* rx_buf, uint16_t len)
{
	BSP_SPI_TxRx(tx_buf, rx_buf, len);
}

void _sFlash_CS_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;	
	GPIO_InitStruct.Pin = GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void _sFlash_CS_Low(void)
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
}

void _sFlash_CS_High(void)
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
}


void _sFlash_WP_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;	
	GPIO_InitStruct.Pin = GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void _sFlash_WP_Low(void)
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
}

void _sFlash_WP_High(void)
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
}

uint32_t _sFlash_GetTickMs(void)
{
//	return OSTimeGet(NULL);
    return xTaskGetTickCount();
}

void _sFlash_Sleep(uint32_t ms)
{
//	OS_ERR err;
//	OSTimeDlyHMSM(0u, 0u, 0u, ms, OS_OPT_TIME_HMSM_NON_STRICT, &err);
    vTaskDelay(ms);
}

#if SFLASH_MULTI_TASK > 0

static SemaphoreHandle_t  _flash_mutex;

/// <summary>
/// [接口函数]Flash互斥锁初始化
/// </summary>
/// <returns>
/// SFLASH_ERR_NONE      : 成功
/// SFLASH_ERR_LOCK_INIT : 互斥锁初始化失败
/// </returns>
uint8_t _sFlash_Lock_Init(void)
{
//	OS_ERR err;
//	OSMutexCreate (&_flash_mutex, "_flash_mutex", &err);
//	if (err == OS_ERR_NONE)    return SFLASH_ERR_NONE;
//	return SFLASH_ERR_LOCK_INIT;
    _flash_mutex = xSemaphoreCreateMutex();
    if(_flash_mutex == NULL)
        return SFLASH_ERR_LOCK_INIT;
    else
        return SFLASH_ERR_NONE;
}

/// <summary>
/// [接口函数]Flash进入临界区
/// </summary>
/// <returns>
/// SFLASH_ERR_NONE         : 成功
/// SFLASH_ERR_LOCK_TIMEOUT : 进入临界区超时
/// SFLASH_ERR_LOCK         : 进入临界区失败
/// </returns>
uint8_t _sFlash_Lock(void)
{
//	OS_ERR err;
//	OSMutexPend(&_flash_mutex, SFLASH_PEND_MS, OS_OPT_PEND_BLOCKING, NULL, &err);
//	if (err == OS_ERR_NONE)    return SFLASH_ERR_NONE;
//	if (err == OS_ERR_TIMEOUT) return SFLASH_ERR_LOCK_TIMEOUT;
//	return SFLASH_ERR_LOCK;
    BaseType_t ret = xSemaphoreTake( _flash_mutex, 1000 );
    if( ret == pdTRUE)
        return SFLASH_ERR_NONE;
    else
        return SFLASH_ERR_LOCK;
}

/// <summary>
/// [接口函数]Flash退出临界区
/// </summary>
/// <returns>
/// SFLASH_ERR_NONE         : 成功
/// SFLASH_ERR_LOCK         : 退出临界区失败
/// </returns>
uint8_t _sFlash_Unlock(void)
{
//	OS_ERR err;
//	OSMutexPost(&_flash_mutex, OS_OPT_POST_NONE, &err);
//	if (err == OS_ERR_NONE)    return SFLASH_ERR_NONE;
//	return SFLASH_ERR_LOCK;
    BaseType_t ret = xSemaphoreGive( _flash_mutex );
    if( ret == pdTRUE)
        return SFLASH_ERR_NONE;
    else
        return SFLASH_ERR_LOCK;
}

#endif /* SFLASH_MULTI_TASK > 0 */
