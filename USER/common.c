#include "common.h"
#include "24cxx.h"
#include "m53xx.h"





/*************************固件升级状态***************************/
FrameWareState_S FrameWareState;


//在str1中查找str2，失败返回0xFF,成功返回str2首个元素在str1中的位置
u16 MyStrstr(u8 *str1, u8 *str2, u16 str1_len, u16 str2_len)
{
	u16 len = str1_len;
	if(str1_len == 0 || str2_len == 0)
	{
		return 0xFFFF;
	}
	else
	{
		while(str1_len >= str2_len)
		{
			str1_len --;
			if (!memcmp(str1, str2, str2_len))
			{
				return len - str1_len - 1;
			}
			str1 ++;
		}
		return 0xFFFF;
	}
}


//32位CRC校验
u32 CRC32( const u8 *buf, u32 size)
{
     uint32_t i, crc;
     crc = 0xFFFFFFFF;
     for (i = 0; i < size; i++)
      crc = crc32tab[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);
     return crc^0xFFFFFFFF;
}

/*****************************************************
函数：u16 CRC16(u8 *puchMsgg,u8 usDataLen)
功能：CRC校验用函数
参数：puchMsgg是要进行CRC校验的消息，usDataLen是消息中字节数 mode:0 低字节在前 1:高字节在前
返回：计算出来的CRC校验码。
*****************************************************/
u16 CRC16(u8 *puchMsgg,u16 usDataLen,u8 mode)
{
	u16 ret = 0;
    u8 uchCRCHi = 0xFF ; 											//高CRC字节初始化
    u8 uchCRCLo = 0xFF ; 											//低CRC 字节初始化
    u8 uIndex ; 													//CRC循环中的索引

    while (usDataLen--) 											//传输消息缓冲区
    {
		uIndex = uchCRCLo ^ *puchMsgg++; 							//计算CRC
		uchCRCLo = uchCRCHi ^ auchCRCHi[uIndex];
		uchCRCHi = auchCRCLo[uIndex];
    }

	if(mode == 0)
	{
		ret = (((u16)uchCRCHi) << 8) + (u16)uchCRCLo;
	}
	else
	{
		ret = (((u16)uchCRCLo) << 8) + (u16)uchCRCHi;
	}

    return ret;
}

u16 GetCRC16(u8 *data,u16 len)
{
	u16 ax,lsb;
	int i,j;

	ax = 0xFFFF;

	for(i = 0; i < len; i ++)
	{
		ax ^= data[i];

		for(j = 0; j < 8; j ++)
		{
			lsb = ax & 0x0001;
			ax = ax >> 1;

			if(lsb != 0)
				ax ^= 0xA001;
		}
	}

	return ax;
}

//计算校验和
u8 CalCheckSum(u8 *buf, u16 len)
{
	u8 sum = 0;
	u16 i = 0;

	for(i = 0; i < len; i ++)
	{
		sum += *(buf + i);
	}

	return sum;
}



//从EEPROM中读取数据(带CRC16校验码)len包括CRC16校验码
u8 ReadDataFromEepromToMemory(u8 *buf,u16 s_add, u16 len)
{
	u16 i = 0;
	u16 j = 0;
	u16 ReadCrcCode;
	u16 CalCrcCode = 0;

	for(i = s_add,j = 0; i < s_add + len; i ++, j++)
	{
		*(buf + j) = AT24CXX_ReadOneByte(i);
	}

	ReadCrcCode = (u16)(*(buf + len - 2));
	ReadCrcCode = ReadCrcCode << 8;
	ReadCrcCode = ReadCrcCode | (u16)(*(buf + len - 1));

	CalCrcCode = CRC16(buf,len - 2,1);

	if(ReadCrcCode == CalCrcCode)
	{
		return 1;
	}

	return 0;
}

//向EEPROM中写入数据(带CRC16校验码)len不包括CRC16校验码
void WriteDataFromMemoryToEeprom(u8 *inbuf,u16 s_add, u16 len)
{
	u16 i = 0;
	u16 j = 0;
	u16 CalCrcCode = 0;

	CalCrcCode = CRC16(inbuf,len,1);

	for(i = s_add ,j = 0; i < s_add + len; i ++, j ++)			//写入原始数据
	{
		AT24CXX_WriteOneByte(i,*(inbuf + j));
	}

	AT24CXX_WriteOneByte(s_add + len + 0,(u8)(CalCrcCode >> 8));		//写入CRC
	AT24CXX_WriteOneByte(s_add + len + 1,(u8)(CalCrcCode & 0x00FF));
}


//将固件升级状态写入到EEPROM
void WriteFrameWareStateToEeprom(void)
{
	u8 temp_buf[20];
	
	temp_buf[0]  = FrameWareState.state;
	temp_buf[1]  = (u8)(FrameWareState.total_bags >> 8);
	temp_buf[2]  = (u8)FrameWareState.total_bags;
	temp_buf[3]  = (u8)(FrameWareState.current_bag_cnt >> 8);
	temp_buf[4]  = (u8)FrameWareState.current_bag_cnt;
	temp_buf[5]  = (u8)(FrameWareState.bag_size >> 8);
	temp_buf[6]  = (u8)FrameWareState.bag_size;
	temp_buf[7]  = (u8)(FrameWareState.last_bag_size >> 8);
	temp_buf[8]  = (u8)FrameWareState.last_bag_size;
	temp_buf[9]  = (u8)(FrameWareState.total_size >> 24);
	temp_buf[10] = (u8)(FrameWareState.total_size >> 16);
	temp_buf[11] = (u8)(FrameWareState.total_size >> 8);
	temp_buf[12] = (u8)FrameWareState.total_size;
	
	WriteDataFromMemoryToEeprom(temp_buf,E_FW_UPDATE_STATE_ADD,E_FW_UPDATE_STATE_LEN - 2);
}

//读取固件设计状态
u8 ReadFrameWareState(void)
{
	u8 ret = 0;
	u8 buf[E_FW_UPDATE_STATE_LEN];

	memset(buf,0,E_FW_UPDATE_STATE_LEN);

	ret = ReadDataFromEepromToMemory(buf,E_FW_UPDATE_STATE_ADD,E_FW_UPDATE_STATE_LEN);

	if(ret == 1)
	{
		FrameWareState.state 			= *(buf + 0);
		FrameWareState.total_bags 		= ((((u16)(*(buf + 1))) << 8) & 0xFF00) + 
		                                  (((u16)(*(buf + 2))) & 0x00FF);
		FrameWareState.current_bag_cnt 	= ((((u16)(*(buf + 3))) << 8) & 0xFF00) + 
		                                  (((u16)(*(buf + 4))) & 0x00FF);
		FrameWareState.bag_size 		= ((((u16)(*(buf + 5))) << 8) & 0xFF00) + 
		                                  (((u16)(*(buf + 6))) & 0x00FF);
		FrameWareState.last_bag_size 	= ((((u16)(*(buf + 7))) << 8) & 0xFF00) + 
		                                  (((u16)(*(buf + 8))) & 0x00FF);

		FrameWareState.total_size 		= ((((u32)(*(buf + 9))) << 24) & 0xFF000000) +
								          ((((u32)(*(buf + 10))) << 16) & 0x00FF0000) +
								          ((((u32)(*(buf + 11))) << 8) & 0x0000FF00) +
								          ((((u32)(*(buf + 12))) << 0) & 0x000000FF);
		
		ret = 1;
	}
	else
	{
		FrameWareState.state 			= FIRMWARE_FREE;
		FrameWareState.total_bags 		= 0;
		FrameWareState.current_bag_cnt 	= 0;
		FrameWareState.bag_size 		= 0;
		FrameWareState.last_bag_size 	= 0;

		FrameWareState.total_size 		= 0;
		
		WriteFrameWareStateToEeprom();			//将默认值写入EEPROM
	}

	return ret;
}












