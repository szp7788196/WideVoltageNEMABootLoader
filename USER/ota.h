#ifndef __OTA_H
#define __OTA_H

#include "sys.h"
#include "common.h"

typedef  void (*iapfun)(void);				//����һ���������͵Ĳ���.  




u16 STMFLASH_ReadHalfWord(u32 faddr);
u8 CopyJumpToAppAddress(void);































#endif
