#ifndef __SFLASH_PORT_H__
#define __SFLASH_PORT_H__


// 需要多任务操作 Flash ?
// !!! 当配置为 0 时,禁止多个任务使用此模块 !!!
#define SFLASH_MULTI_TASK         0          /* 0: Disable  1: Enable */   


// 获取操作权限时,等待的最大毫秒数(仅多任务操作时有效) 
#define SFLASH_PEND_MS            50000      



// --- 选择芯片 ---
// 支持的芯片列表:
//   SST25VF080B
//   SST25VF016B
//   W25Q32FV
//   W25Q64FV
//   W25Q128FV
#define SFLASH_CHIP               SST25VF080B


#endif /* __SFLASH_PORT_H__ */
