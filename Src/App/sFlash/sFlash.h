#ifndef __SFLASH_H__
#define __SFLASH_H__


#include "stdint.h"
#include "sFlash_port.h"



#define SST25VF080B         0x00BF258E 
#define SST25VF016B         0x00BF2541 
#define W25Q32FV            0x00EF4016 
#define W25Q64FV            0x00EF4017 
#define W25Q128FV           0x00EF4018 


#if SFLASH_CHIP == SST25VF080B
	#define SFLASH_SIZE    (1 * 1024 * 1024)   // Flash容量 1MB
	#define CHIP_ERASE_MS   50                 // 50 ms
	#define BLOCK_ERASE_MS  25                 // 25 ms
	#define AAI_PROGRAM
#elif SFLASH_CHIP == SST25VF016B
	#define SFLASH_SIZE    (2 * 1024 * 1024)   // Flash容量 2MB
	#define CHIP_ERASE_MS   50                 // 50 ms
	#define BLOCK_ERASE_MS  25                 // 25 ms
	#define AAI_PROGRAM
#elif SFLASH_CHIP == W25Q32FV
	#define SFLASH_SIZE    (4 * 1024 * 1024)   // Flash容量 4MB
	#define CHIP_ERASE_MS  (50 * 1000)         // 50 sec
	#define BLOCK_ERASE_MS  2000               // 2000 ms
	#define PAGE_PROGRAM
#elif SFLASH_CHIP == W25Q64FV
	#define SFLASH_SIZE    (8 * 1024 * 1024)   // Flash容量 8MB
	#define CHIP_ERASE_MS  (100 * 1000)        // 100 sec
	#define BLOCK_ERASE_MS  2000               // 2000 ms
	#define PAGE_PROGRAM
#elif SFLASH_CHIP == W25Q128FV
	#define SFLASH_SIZE    (16 * 1024 * 1024)  // Flash容量 16MB
	#define CHIP_ERASE_MS  (200 * 1000)        // 200 sec
	#define BLOCK_ERASE_MS  2000               // 2000 ms
	#define PAGE_PROGRAM
#else
	#error "SFLASH_CHIP define error!"
#endif

// Flash的扇区大小(最小擦除单元)
#define SFLASH_SECTOR_SIZE        0x1000UL   /* 4096 */


#define SFLASH_ERR_NONE                 0  // 无错误
#define SFLASH_ERR_LOCK_INIT            1  // 互斥锁初始化失败(sFlash_Init 函数可能产生此错误)
#define SFLASH_ERR_LOCK_TIMEOUT         2  // 进入临界区超时
#define SFLASH_ERR_LOCK                 3  // 进入或退出临界区失败
#define SFLASH_ERR_ARGS                 4  // 参数错误
#define SFLASH_ERR_CHIP_TIMEOUT         5  // 等待芯片BUSY信号消失 超时
#define SFLASH_ERR_JEDEC                6  // JEDEC不匹配(芯片型号有误, sFlash_Init 函数可能产生此错误)
#define SFLASH_ERR_NOT_BLANK            7  // 非空(sFlash_BlankCheck 函数可能产生此错误)
#define SFLASH_ERR_NO_BLANK_SECTOR      8  // 没有空扇区(sFlash_Test 函数可能产生此错误)
#define SFLASH_ERR_TEST_FAILED          9  // 测试失败(sFlash_Test 函数可能产生此错误)


/// <summary>
/// 初始化
/// </summary>
/// <returns>
/// SFLASH_ERR_NONE         : 成功
/// SFLASH_ERR_LOCK_INIT    : 互斥锁初始化失败
/// SFLASH_ERR_CHIP_TIMEOUT : 等待芯片BUSY信号消失 超时
/// SFLASH_ERR_JEDEC        : JEDEC不匹配(芯片型号有误)
/// </returns>
uint8_t sFlash_Init(void);

/// <summary>
/// 擦除整个芯片
/// </summary>
/// <returns>
/// SFLASH_ERR_NONE         : 成功
/// SFLASH_ERR_LOCK_TIMEOUT : 获取操作权限超时(在多任务操作时可能产生)
/// SFLASH_ERR_LOCK         : 获取操作权限失败(在多任务操作时可能产生)
/// SFLASH_ERR_CHIP_TIMEOUT : 等待芯片BUSY信号消失 超时
/// </returns>
uint8_t sFlash_EraseChip(void);

/// <summary>
/// 擦除扇区
/// (addr 必须可以被 SFLASH_SECTOR_SIZE 整除, 否则不执行)
/// </summary>
/// <param name="addr">Flash地址</param>
/// <returns>
/// SFLASH_ERR_NONE         : 成功
/// SFLASH_ERR_LOCK_TIMEOUT : 获取操作权限超时(在多任务操作时可能产生)
/// SFLASH_ERR_LOCK         : 获取操作权限失败(在多任务操作时可能产生)
/// SFLASH_ERR_CHIP_TIMEOUT : 等待芯片BUSY信号消失 超时
/// </returns>
uint8_t sFlash_EraseSector(uint32_t addr);

/// <summary>
/// 擦除指定位置,指定大小的 Flash 区域
/// (addr 及 size 必须可以被 SFLASH_SECTOR_SIZE 整除, 否则不执行)
/// </summary>
/// <param name="addr">开始地址</param>
/// <param name="size">总擦除字节数</param>
/// <returns>
/// SFLASH_ERR_NONE         : 成功
/// SFLASH_ERR_LOCK_TIMEOUT : 获取操作权限超时(在多任务操作时可能产生)
/// SFLASH_ERR_LOCK         : 获取操作权限失败(在多任务操作时可能产生)
/// SFLASH_ERR_CHIP_TIMEOUT : 等待芯片BUSY信号消失 超时
/// </returns>
uint8_t sFlash_Erase(uint32_t addr, uint32_t size);

/// <summary>
/// 在指定地址写入 1 字节的数据
/// </summary>
/// <param name="addr">地址</param>
/// <param name="b">要写入数据</param>
/// <returns>
/// SFLASH_ERR_NONE         : 成功
/// SFLASH_ERR_LOCK_TIMEOUT : 获取操作权限超时(在多任务操作时可能产生)
/// SFLASH_ERR_LOCK         : 获取操作权限失败(在多任务操作时可能产生)
/// SFLASH_ERR_CHIP_TIMEOUT : 等待芯片BUSY信号消失 超时
/// </returns>
uint8_t sFlash_WriteByte(uint32_t addr, uint8_t b);

/// <summary>
/// 从指定地址开始写入多字节的数据
/// </summary>
/// <param name="addr">起始地址</param>
/// <param name="buf">要写入的数据的指针</param>
/// <param name="len">写入字节数</param>
/// <returns>
/// SFLASH_ERR_NONE         : 成功
/// SFLASH_ERR_LOCK_TIMEOUT : 获取操作权限超时(在多任务操作时可能产生)
/// SFLASH_ERR_LOCK         : 获取操作权限失败(在多任务操作时可能产生)
/// SFLASH_ERR_CHIP_TIMEOUT : 等待芯片BUSY信号消失 超时
/// </returns>
uint8_t sFlash_WriteBytes(uint32_t addr, const void* buf, uint32_t len);

/// <summary>
/// 从指定地址读取 1 字节的数据
/// </summary>
/// <param name="addr">起始地址</param>
/// <param name="p_byte">存放数据的字节指针</param>
/// <returns>
/// SFLASH_ERR_NONE         : 成功
/// SFLASH_ERR_LOCK_TIMEOUT : 获取操作权限超时(在多任务操作时可能产生)
/// SFLASH_ERR_LOCK         : 获取操作权限失败(在多任务操作时可能产生)
/// SFLASH_ERR_CHIP_TIMEOUT : 等待芯片BUSY信号消失 超时
/// </returns>
uint8_t sFlash_ReadByte(uint32_t addr, uint8_t* p_byte);

/// <summary>
/// 从指定地址开始读取多字节的数据
/// (请确保 buf 的可写入字节数大于等于len)
/// </summary>
/// <param name="addr">起始地址</param>
/// <param name="buf">存放读取数据的指针</param>
/// <param name="len">读取字节数</param>
/// <returns>
/// SFLASH_ERR_NONE         : 成功
/// SFLASH_ERR_LOCK_TIMEOUT : 获取操作权限超时(在多任务操作时可能产生)
/// SFLASH_ERR_LOCK         : 获取操作权限失败(在多任务操作时可能产生)
/// SFLASH_ERR_CHIP_TIMEOUT : 等待芯片BUSY信号消失 超时
/// </returns>
uint8_t sFlash_ReadBytes(uint32_t addr, void* buf, uint32_t len);

/// <summary>
/// 检查指定位置,指定大小的 Flash 区域是否为空
/// 当 addr 或 size 的范围不正确时,返回0(非空)
/// </summary>
/// <param name="addr">起始地址</param>
/// <param name="size">需要检查的字节数</param>
/// <param name="p_is_blank">1:空(全为0xFF), 0:非空</param>
/// <returns>
/// SFLASH_ERR_NONE         : 成功
/// SFLASH_ERR_LOCK_TIMEOUT : 获取操作权限超时(在多任务操作时可能产生)
/// SFLASH_ERR_LOCK         : 获取操作权限失败(在多任务操作时可能产生)
/// SFLASH_ERR_CHIP_TIMEOUT : 等待芯片BUSY信号消失 超时
/// SFLASH_ERR_NOT_BLANK    : 非空
/// </returns>
uint8_t sFlash_BlankCheck(uint32_t addr, uint32_t size);

#ifndef NDEBUG
uint8_t sFlash_Test(void);
#endif /* ifndef NDEBUG */

#endif /* __SFLASH_H__ */
