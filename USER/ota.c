#include "ota.h"


iapfun jump2app;


//��ת��Ӧ�ó����
//appxaddr:�û�������ʼ��ַ.
void iap_load_app(u32 appxaddr)
{
	jump2app = (iapfun) * (vu32*)(appxaddr + 4);	//�û��������ڶ�����Ϊ����ʼ��ַ(��λ��ַ)
	MSR_MSP(*(vu32*)appxaddr);						//��ʼ��APP��ջָ��(�û��������ĵ�һ�������ڴ��ջ����ַ)
	jump2app();										//��ת��APP.
}

//��ȡָ����ַ�İ���(16λ����)
//faddr:����ַ(�˵�ַ����Ϊ2�ı���!!)
//����ֵ:��Ӧ����.
u16 STMFLASH_ReadHalfWord(u32 faddr)
{
	return *(vu16*)faddr;
}

//���Ƴ�����벢��ת��Ӧ�ó����ִַ��
u8 CopyJumpToAppAddress(void)
{
	u8 ret = 0;
	u32 i = 0;
	u16 page_num = 0;
	u32 frameware_size = 0;
	u16 temp = 0;

	ret = ReadFrameWareState();

	while(ret)	//��ȡ�̼�״̬�ɹ�
	{
		frameware_size = FrameWareState.total_size - FrameWareState.total_bags * 2;	//�̼���С(��ȥCRC16�ĳ���)

		switch(FrameWareState.state)
		{
			case FIRMWARE_DOWNLOADED:		//�̼��Ѿ��������
				FrameWareState.state = FIRMWARE_ERASEING;	//׼������Ӧ����FLASH
			break;

			case FIRMWARE_ERASEING:			//���ڲ���FLASH
				page_num = (FIRMWARE_BUCKUP_FLASH_BASE_ADD - FIRMWARE_RUN_FLASH_BASE_ADD) / 2048;	//�õ�Ӧ��������������

				FLASH_Unlock();						//����

				for(i = 0; i < page_num; i ++)
				{
					FLASH_ErasePage(i * 2048 + FIRMWARE_RUN_FLASH_BASE_ADD);	//����FLASH
				}

				FLASH_Lock();						//����

				FrameWareState.state = FIRMWARE_ERASE_SUCCESS;	//����FLASH���
			break;

			case FIRMWARE_ERASE_SUCCESS:		//FLASH�����ɹ�
				FrameWareState.state = FIRMWARE_UPDATING;		//׼�����ƹ̼�
			break;

			case FIRMWARE_UPDATING:			//��������(���ƹ̼�)
				FLASH_Unlock();						//����

				for(i = 0; i < frameware_size; i += 2)
				{
					temp = STMFLASH_ReadHalfWord(FIRMWARE_BUCKUP_FLASH_BASE_ADD + i);
					FLASH_ProgramHalfWord(FIRMWARE_RUN_FLASH_BASE_ADD + i,temp);
				}

				FLASH_Unlock();						//����

				FrameWareState.state = FIRMWARE_UPDATE_SUCCESS;		//�̼��������
			break;

			case FIRMWARE_UPDATE_SUCCESS:		//�������(�������)
				WriteFrameWareStateToEeprom();	//���¹̼�����״̬

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
//				WriteFrameWareStateToEeprom();	//���¹̼�����״̬

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

	WriteFrameWareStateToEeprom();	//���¹̼�����״̬

	iap_load_app(FIRMWARE_RUN_FLASH_BASE_ADD);

	return ret;
}


















