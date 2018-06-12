#include "string.h"
#include "sFlash.h"
#include "sFlash_port.h"

#define DUMMY_BYTE         0xA5

#define ERASE_CMD_4KB      0x20
#define ERASE_CMD_32KB     0x52
#define ERASE_CMD_64KB     0xD8
#define ERASE_CMD_CHIP     0x60

#define SIZE_4KB          (4 * 1024)
#define SIZE_32KB         (32 * 1024)
#define SIZE_64KB         (64 * 1024)

void     _sFlash_SPI_Init(void);
void     _sFlash_SPI_Tx(const uint8_t* buf, uint16_t len);
void     _sFlash_SPI_Rx(uint8_t* buf, uint16_t len);
void     _sFlash_SPI_TxRx(const uint8_t* tx_buf, uint8_t* rx_buf, uint16_t len);
void     _sFlash_CS_Init(void);
void     _sFlash_CS_Low(void);
void     _sFlash_CS_High(void);
void     _sFlash_WP_Init(void);
void     _sFlash_WP_Low(void);
void     _sFlash_WP_High(void);
uint32_t _sFlash_GetTickMs(void);
void     _sFlash_Sleep(uint32_t ms);



#if SFLASH_MULTI_TASK == 0

uint8_t _sFlash_Lock_Init(void) { return SFLASH_ERR_NONE; }
uint8_t _sFlash_Lock(void)      { return SFLASH_ERR_NONE; }
uint8_t _sFlash_Unlock(void)    { return SFLASH_ERR_NONE; }

#else

uint8_t _sFlash_Lock_Init(void);
uint8_t _sFlash_Lock(void);
uint8_t _sFlash_Unlock(void);

#endif /* SFLASH_MULTI_TASK == 0 */


#define _LOCK    uint8_t __lock_unlock_result = _sFlash_Lock(); \
                 if (__lock_unlock_result != SFLASH_ERR_NONE) \
                 	return __lock_unlock_result
				 
#define _UNLOCK  __lock_unlock_result = _sFlash_Unlock(); \
                 if (__lock_unlock_result != SFLASH_ERR_NONE) \
                 	return __lock_unlock_result



static uint32_t _get_jedec_id(void)
{
	uint32_t id = 0;
	uint8_t  buf[] = {0x9F, DUMMY_BYTE, DUMMY_BYTE, DUMMY_BYTE};
	
	_sFlash_CS_Low();
	_sFlash_SPI_TxRx(buf, buf, sizeof(buf));
	_sFlash_CS_High();

	id = (buf[1] << 16) + (buf[2] << 8) + (buf[3] << 0);
	
	return id;
}

// return 1 : success, 0 : failed
static uint8_t _wait_rdy(uint32_t timeout_ms)
{
	const uint8_t  tx_buf[] = {0x05};
	uint8_t  rx_buf[]       = {DUMMY_BYTE};
	uint32_t tick_ms        = _sFlash_GetTickMs();
	uint32_t sleep_ms       = timeout_ms > 10 ? timeout_ms / 10 : 1;
	uint8_t  is_success     = 0;
	_sFlash_CS_Low();
	_sFlash_SPI_Tx(tx_buf, sizeof(tx_buf));
	do
	{
		_sFlash_SPI_Rx(rx_buf, sizeof(rx_buf));	
		if (rx_buf[0] == 0xFF)
		{
			break;
		}
		if (!(rx_buf[0] & 0x01))
		{
			is_success = 1;
			break;
		}
		_sFlash_Sleep(sleep_ms);		
	} while (_sFlash_GetTickMs() - tick_ms < (timeout_ms + 1000)); // 获取ms的函数(_sFlash_GetTickMs)可能由 秒*1000 实现
	_sFlash_CS_High();
	return is_success;
}

// Write-Enable
static void _WREN(void)
{
	const uint8_t tx_buf[] = {0x06};
	
	_sFlash_CS_Low();
	_sFlash_SPI_Tx(tx_buf, sizeof(tx_buf));
	_sFlash_CS_High();
}

// Write-Disable
static void _WRDI(void)
{
	const uint8_t tx_buf[] = {0x04};
	
	_sFlash_CS_Low();
	_sFlash_SPI_Tx(tx_buf, sizeof(tx_buf));
	_sFlash_CS_High();
}

// Write-Status-Register
// return 1 : success, 0 : failed
static uint8_t _WRSR(uint8_t status)
{
	const uint8_t tx_buf[] = {0x01, status};
	
	_WREN();	
	_sFlash_CS_Low();
	_sFlash_SPI_Tx(tx_buf, sizeof(tx_buf));
	_sFlash_CS_High();
	return _wait_rdy(1);
}


uint8_t sFlash_Init(void)
{	
	_sFlash_WP_Init();
	_sFlash_WP_High();	
	
	_sFlash_SPI_Init();
	_sFlash_CS_Init();
		
	_WRDI(); // 可能在写入数据过程中重启了
	
	uint32_t id = _get_jedec_id();
	if (id != SFLASH_CHIP)
	{
		//ELogERR(ELOG_ID_FLASH, DEF_TRUE, "Flash Get JEDEC_ID Failed. ID = 0x%.8X.", id);
		return 0;
	}
	//ELogTRC(ELOG_ID_FLASH, DEF_TRUE, "Flash Get JEDEC Success.");
	
	uint8_t is_success = _WRSR(0x00);
	
	uint8_t err = SFLASH_ERR_NONE;
	if (is_success)
	{
		err = _sFlash_Lock_Init();
	}
	
	if (is_success && err == SFLASH_ERR_NONE) 
	{
		//ELogTRC(ELOG_ID_FLASH, DEF_TRUE, "Flash Init Success.");
		return SFLASH_ERR_NONE;
	}
	else            
	{
		//ELogERR(ELOG_ID_FLASH, DEF_TRUE, "Flash Init Failed.");
		return SFLASH_ERR_CHIP_TIMEOUT;
	}
}

uint8_t sFlash_EraseChip(void)
{
	const uint8_t tx_buf[] = {ERASE_CMD_CHIP};

	_LOCK;

	_WREN();

	_sFlash_CS_Low();
	_sFlash_SPI_Tx(tx_buf, sizeof(tx_buf));
	_sFlash_CS_High();
	
	_WRDI();
	uint8_t is_success = _wait_rdy(CHIP_ERASE_MS);
	
	_UNLOCK;
	//ELogERR(ELOG_ID_FLASH, !is_success, "sFlash_EraseChip Failed.");
	return is_success ? SFLASH_ERR_NONE : SFLASH_ERR_CHIP_TIMEOUT;
}


static uint8_t _erase(uint32_t addr, uint8_t cmd)
{
	const uint8_t tx_buf[] = {cmd, (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, (addr >> 0) & 0xFF};

	_WREN();
	_sFlash_CS_Low();
	_sFlash_SPI_Tx(tx_buf, sizeof(tx_buf));
	_sFlash_CS_High();
	_WRDI();
	
	return _wait_rdy(BLOCK_ERASE_MS);
}

uint8_t sFlash_EraseSector(uint32_t addr)
{
	if ((addr & (SIZE_4KB - 1)) ||
		(addr >= SFLASH_SIZE))
	{
		//ELogERR(ELOG_ID_FLASH, DEF_TRUE, "sFlash_EraseSector Args Error. addr = 0x%.8X.", addr);
		return SFLASH_ERR_ARGS;
	}

	_LOCK;
	uint8_t is_success = _erase(addr, ERASE_CMD_4KB);
	_UNLOCK;
	//ELogERR(ELOG_ID_FLASH, !is_success, "sFlash_EraseSector Failed.");
	return is_success ? SFLASH_ERR_NONE : SFLASH_ERR_CHIP_TIMEOUT;
}

uint8_t sFlash_Erase(uint32_t addr, uint32_t size)
{
	if ((addr & (SIZE_4KB - 1)) ||
		(size & (SIZE_4KB - 1)) ||
		(size == 0) ||
		(addr + size > SFLASH_SIZE))
	{
		//ELogERR(ELOG_ID_FLASH, DEF_TRUE, "sFlash_Erase Args Error. addr = 0x%.8X, size = 0x%.8X.", addr, size);
		return SFLASH_ERR_ARGS;
	}
	
	uint8_t is_success = 1;
	_LOCK;		
	while (size >= SIZE_4KB)
	{
		if ((size >= SIZE_64KB) && (addr % SIZE_64KB == 0))
		{
			is_success = _erase(addr, ERASE_CMD_64KB);
			if (!is_success) break;
			size -= SIZE_64KB;
			addr += SIZE_64KB;
		}
		else if ((size >= SIZE_32KB) && (addr % SIZE_32KB == 0))
		{
			is_success = _erase(addr, ERASE_CMD_32KB);
			if (!is_success) break;
			size -= SIZE_32KB;
			addr += SIZE_32KB;
		}
		else if ((size >= SIZE_4KB) && (addr % SIZE_4KB == 0))
		{
			is_success = _erase(addr, ERASE_CMD_4KB);
			if (!is_success) break;
			size -= SIZE_4KB;
			addr += SIZE_4KB;
		}
	}
	_UNLOCK;
	//ELogERR(ELOG_ID_FLASH, !is_success, "sFlash_Erase Failed.");
	return is_success ? SFLASH_ERR_NONE : SFLASH_ERR_CHIP_TIMEOUT;
}

static uint8_t _write_byte(uint32_t addr, uint8_t b)
{
	const uint8_t tx_buf[] = {0x02, (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, (addr >> 0) & 0xFF, b};

	_WREN();
	_sFlash_CS_Low();
	_sFlash_SPI_Tx(tx_buf, sizeof(tx_buf));
	_sFlash_CS_High();		
	_WRDI();
	
	return _wait_rdy(1);
}

uint8_t sFlash_WriteByte(uint32_t addr, uint8_t b)
{	
	if (addr >= SFLASH_SIZE)
	{
		//ELogERR(ELOG_ID_FLASH, DEF_TRUE, "sFlash_WriteByte Args Error. addr = 0x%.8X.", addr);
		return SFLASH_ERR_ARGS;
	}
	
	_LOCK;
		
	uint8_t is_success = _write_byte(addr, b);
	
	_UNLOCK;
	//ELogERR(ELOG_ID_FLASH, !is_success, "sFlash_WriteByte Failed.");
	return is_success ? SFLASH_ERR_NONE : SFLASH_ERR_CHIP_TIMEOUT;
}

#ifdef AAI_PROGRAM
uint8_t sFlash_WriteBytes(uint32_t addr, const void* buf, uint32_t len)
{
	uint8_t buf2[] = {0xAD, DUMMY_BYTE, DUMMY_BYTE, DUMMY_BYTE, 0xFF, 0xFF};

	//ELogERR(ELOG_ID_FLASH, addr + len > SFLASH_SIZE, "sFlash_WriteBytes Args Error. addr = 0x%.8X, len = 0x%.8X.", addr, len);
	//ELogERR(ELOG_ID_FLASH, buf == (void*)0,          "sFlash_WriteBytes Args Error. buf = NULL.");
	//ELogERR(ELOG_ID_FLASH, len == 0,                 "sFlash_WriteBytes Args Error. len = 0x%.8X.", len);
	if ((addr + len > SFLASH_SIZE) ||
		(buf == (void*)0) ||
		(len == 0))
	{
		return SFLASH_ERR_ARGS;
	}
	
	uint8_t is_success = 1;
	
	_LOCK;
		
	uint8_t* p_buf = (uint8_t*)buf;
	if ((len == 1) || (addr & 0x1))
	{
		_write_byte(addr, p_buf[0]);
		p_buf++;
		addr++;
		len--;
	}
	if (len == 0)
	{
		_UNLOCK;
		return SFLASH_ERR_NONE;
	}
	if (len == 1)
	{
		_write_byte(addr, p_buf[0]);
		_UNLOCK;
		return SFLASH_ERR_NONE;
	}
	
	// Start AAI Programming
	
	_WREN();
	_sFlash_CS_Low();
	buf2[1] = (addr >> 16) & 0xFF;
	buf2[2] = (addr >> 8) & 0xFF;
	buf2[3] = (addr >> 0) & 0xFF;
	_sFlash_SPI_Tx(buf2, 3);
	while (len >= 2 && is_success)
	{
		buf2[4] = p_buf[0];
		buf2[5] = p_buf[1];
		_sFlash_SPI_Tx(&buf2[3], 3);
		_sFlash_CS_High();
		
		//volatile uint8_t dly = 10;	
		//while (dly--){}
		
		is_success = _wait_rdy(1);
		
		len -= 2;		
		p_buf += 2;
		addr  += 2;
		if (len >= 2)
		{
			buf2[3] = 0xAD;
			
			//dly = 10;
			//while (dly--){}
			
			_sFlash_CS_Low();
		}
	}
	_WRDI();
	if (is_success)
	{
		is_success = _wait_rdy(1);
	}
	
	// End AAI Programming

	if (len == 1 && is_success)
	{
		is_success = _write_byte(addr, p_buf[0]);
	}
	
	_UNLOCK;
	if (is_success) return SFLASH_ERR_NONE;
	else            return SFLASH_ERR_CHIP_TIMEOUT;
}
#endif /* AII_PROGRAM */

#ifdef PAGE_PROGRAM
uint8_t sFlash_WriteBytes(uint32_t addr, const void* buf, uint32_t len)
{
	uint8_t buf2[] = {0x02, DUMMY_BYTE, DUMMY_BYTE, DUMMY_BYTE};
	
	//ELogERR(ELOG_ID_FLASH, addr + len > SFLASH_SIZE, "sFlash_WriteBytes Args Error. addr = 0x%.8X, len = 0x%.8X.", addr, len);
	//ELogERR(ELOG_ID_FLASH, buf == (void*)0,          "sFlash_WriteBytes Args Error. buf = NULL.");
	//ELogERR(ELOG_ID_FLASH, len == 0,                 "sFlash_WriteBytes Args Error. len = 0x%.8X.", len);
	if ((addr + len > SFLASH_SIZE) ||
		(buf == (void*)0) ||
		(len == 0))
	{
		return SFLASH_ERR_ARGS;
	}

	uint8_t* p_buf = (uint8_t*)buf;
	
	uint8_t is_success = 1;

	_LOCK;
		
	do
	{
		_WREN();
		
		buf2[0] = 0x02;
		buf2[1] = (addr >> 16) & 0xFF;
		buf2[2] = (addr >> 8) & 0xFF;
		buf2[3] = (addr >> 0) & 0xFF;
		_sFlash_CS_Low();
		_sFlash_SPI_Tx(buf2, sizeof(buf2));
		uint8_t n = 0;
		do
		{
			buf2[n] = p_buf[0];
			n++;
			addr++;
			p_buf++;
			len--;
		} while ((n < sizeof(buf2)) && (addr & 0xFF) && (len > 0));
		//                           page size 256 bytes
		_sFlash_SPI_Tx(buf2, n);
		_sFlash_CS_High();
		_WRDI();
		is_success = _wait_rdy(1);
	} while (len > 0 && is_success);
	
	_UNLOCK;
	if (is_success) return SFLASH_ERR_NONE;
	else            return SFLASH_ERR_CHIP_TIMEOUT;
}
#endif /* PAGE_PROGRAM */


static void _read_bytes(uint32_t addr, void* buf, uint32_t len)
{
	const uint8_t tx_buf[] = {0x03, (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, (addr >> 0) & 0xFF};

	_sFlash_CS_Low();
	_sFlash_SPI_Tx(tx_buf, sizeof(tx_buf));
	_sFlash_SPI_Rx(buf, len);
	_sFlash_CS_High();	
}

uint8_t sFlash_ReadByte(uint32_t addr, uint8_t* p_byte)
{
	if ((addr >= SFLASH_SIZE) ||
		(p_byte == NULL))
	{
		//ELogERR(ELOG_ID_FLASH, DEF_TRUE, "sFlash_ReadByte Args Error. addr = 0x%.8X.", addr);
		return SFLASH_ERR_ARGS;
	}
	
	_LOCK;

	_read_bytes(addr, p_byte, 1);
	
	_UNLOCK;
	return SFLASH_ERR_NONE;
}

uint8_t sFlash_ReadBytes(uint32_t addr, void* buf, uint32_t len)
{	
	if ((addr + len > SFLASH_SIZE) ||
		(buf == (void*)0) ||
		(len == 0))
	{
		//ELogERR(ELOG_ID_FLASH, DEF_TRUE, "sFlash_ReadBytes Args Error. addr = 0x%.8X, len = 0x%.8X.", addr, len);
		return SFLASH_ERR_ARGS;
	}
	
	_LOCK;

	_read_bytes(addr, buf, len);
	
	_UNLOCK;
	return SFLASH_ERR_NONE;
}

uint8_t sFlash_BlankCheck(uint32_t addr, uint32_t size)
{
	if ((addr + size > SFLASH_SIZE) ||
		(size == 0))
	{
		//ELogERR(ELOG_ID_FLASH, DEF_TRUE, "sFlash_ReadBytes Args Error. addr = 0x%.8X, size = 0x%.8X.", addr, size);
		return SFLASH_ERR_ARGS;
	}

	_LOCK;
	while (size > 0)
	{
		uint8_t n = 4;
		if (size < 4)
			n = size;
		uint32_t buf = UINT32_MAX;
		_read_bytes(addr, &buf, n);
		if (buf != UINT32_MAX)
		{
			_UNLOCK;
			return SFLASH_ERR_NOT_BLANK;
		}
		addr += n;
		size -= n;
	}
	
	_UNLOCK;
	return SFLASH_ERR_NONE;
}
//#ifndef NDEBUG
//
//#include "lib_math.h"
//#define TEST_BUF_SIZE  60
//// 非破坏性测试
//// 若测试失败,则考虑使用第347,359行的延时
//uint8_t sFlash_Test(void)
//{
//	// 搜索连续两块空扇区
//	uint32_t addr_blank = UINT32_MAX;
//	for (uint32_t addr = 0; addr < SFLASH_SIZE; addr += SFLASH_SECTOR_SIZE)
//	{
//		uint8_t err = sFlash_BlankCheck(addr, SFLASH_SECTOR_SIZE);
//		if (err == SFLASH_ERR_NONE)
//		{
//			addr_blank = addr;
//			break;
//		}
//	}
//	if (addr_blank == UINT32_MAX)
//	{
//		return SFLASH_ERR_NO_BLANK_SECTOR;
//	}
//	Math_Init();
//	uint32_t n = Math_Rand() % TEST_BUF_SIZE + 1;
//	uint8_t  w_buf[TEST_BUF_SIZE];
//	for (uint32_t i = 0; i < sizeof(w_buf); i++)
//	{
//		w_buf[i] = Math_Rand();
//	}
//	uint8_t  r_buf[TEST_BUF_SIZE];
//	for (uint32_t i = 0; i < sizeof(w_buf); i++)
//	{
//		r_buf[i] = Math_Rand();
//	}
//	sFlash_Erase(addr_blank, SFLASH_SECTOR_SIZE);	
//	sFlash_WriteBytes(addr_blank, w_buf, n);
//	sFlash_ReadBytes(addr_blank, r_buf, n);
//	for (uint32_t i = 0; i < n; i++)
//	{
//		if (r_buf[i] != w_buf[i])
//		{
//			return SFLASH_ERR_TEST_FAILED;
//		}
//	}
//	return SFLASH_ERR_NONE;
//}
//#endif /* ifndef NDEBUG */
