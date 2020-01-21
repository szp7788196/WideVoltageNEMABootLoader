#include "ota.h"


iapfun jump2app;


//跳转到应用程序段
//appxaddr:用户代码起始地址.
void iap_load_app(u32 appxaddr)
{
	jump2app = (iapfun) * (vu32*)(appxaddr + 4);	//用户代码区第二个字为程序开始地址(复位地址)
	MSR_MSP(*(vu32*)appxaddr);						//初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
	jump2app();										//跳转到APP.
}

//读取指定地址的半字(16位数据)
//faddr:读地址(此地址必须为2的倍数!!)
//返回值:对应数据.
u16 STMFLASH_ReadHalfWord(u32 faddr)
{
	return *(vu16*)faddr;
}

//复制程序代码并跳转到应用程序地址执行
u8 CopyJumpToAppAddress(void)
{
	u8 ret = 0;
	u32 i = 0;
	u16 page_num = 0;
	u32 frameware_size = 0;
	u16 temp = 0;

	ret = ReadFrameWareState();

	while(ret)	//读取固件状态成功
	{
		frameware_size = FrameWareState.total_size - FrameWareState.total_bags * 2;	//固件大小(减去CRC16的长度)

		switch(FrameWareState.state)
		{
			case FIRMWARE_DOWNLOADED:		//固件已经下载完成
				FrameWareState.state = FIRMWARE_ERASEING;	//准备擦除应用区FLASH
			break;

			case FIRMWARE_ERASEING:			//正在擦除FLASH
				page_num = (FIRMWARE_BUCKUP_FLASH_BASE_ADD - FIRMWARE_RUN_FLASH_BASE_ADD) / 2048;	//得到应用区的扇区总数

				FLASH_Unlock();						//解锁

				for(i = 0; i < page_num; i ++)
				{
					FLASH_ErasePage(i * 2048 + FIRMWARE_RUN_FLASH_BASE_ADD);	//擦除FLASH
				}

				FLASH_Lock();						//上锁

				FrameWareState.state = FIRMWARE_ERASE_SUCCESS;	//擦除FLASH完成
			break;

			case FIRMWARE_ERASE_SUCCESS:		//FLASH擦除成功
				FrameWareState.state = FIRMWARE_UPDATING;		//准备复制固件
			break;

			case FIRMWARE_UPDATING:			//正在升级(复制固件)
				FLASH_Unlock();						//解锁

				for(i = 0; i < frameware_size; i += 2)
				{
					temp = STMFLASH_ReadHalfWord(FIRMWARE_BUCKUP_FLASH_BASE_ADD + i);
					FLASH_ProgramHalfWord(FIRMWARE_RUN_FLASH_BASE_ADD + i,temp);
				}

				FLASH_Unlock();						//解锁

				FrameWareState.state = FIRMWARE_UPDATE_SUCCESS;		//固件复制完成
			break;

			case FIRMWARE_UPDATE_SUCCESS:		//升级完成(复制完成)
				WriteFrameWareStateToEeprom();	//更新固件升级状态

				iap_load_app(FIRMWARE_RUN_FLASH_BASE_ADD);
			break;

			case FIRMWARE_DOWNLOADING:
				iap_load_app(FIRMWARE_RUN_FLASH_BASE_ADD);
			break;

			default:
//				FrameWareState.state 			= FIRMWARE_FREE;
//				FrameWareState.total_bags 		= 0;
//				FrameWareState.current_bag_cnt 	= 0;
//				FrameWareState.bag_size 		= 0;
//				FrameWareState.last_bag_size 	= 0;
//				FrameWareState.total_size 		= 0;
//
//				WriteFrameWareStateToEeprom();	//更新固件升级状态

				iap_load_app(FIRMWARE_RUN_FLASH_BASE_ADD);
			break;
		}
	}

	FrameWareState.state 			= FIRMWARE_FREE;
	FrameWareState.total_bags 		= 0;
	FrameWareState.current_bag_cnt 	= 0;
	FrameWareState.bag_size 		= 0;
	FrameWareState.last_bag_size 	= 0;
	FrameWareState.total_size 		= 0;

	WriteFrameWareStateToEeprom();	//更新固件升级状态

	iap_load_app(FIRMWARE_RUN_FLASH_BASE_ADD);

	return ret;
}


















