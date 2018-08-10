
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include "Com.h"
#include <iostream>
#define chip_Add 0x32
#define FuncR 0x03

#define		TOTAL_PORT_NUM		65
#define		START_PORT_NUM		0

#define		iBufferSize 250
#define     UARTBufferLength 2000
#undef  SYNCHRONOUS_MODE
static HANDLE        openHandle=NULL;
static HANDLE		 hComDev[TOTAL_PORT_NUM]         ={NULL};
static unsigned long long ulComMask = 0;
static HANDLE		 hCOMThread[TOTAL_PORT_NUM]      ={NULL};
static OVERLAPPED	 stcWriteStatus[TOTAL_PORT_NUM]  = {0};
static OVERLAPPED	 stcReadStatus[TOTAL_PORT_NUM]   = {0};

#ifdef SYNCHRONOUS_MODE
static HANDLE		 hReceiveEvent[TOTAL_PORT_NUM]   ={NULL};
#endif


unsigned long ulComNo=3;
static volatile char chrUARTBuffers[TOTAL_PORT_NUM][UARTBufferLength]={0};
static volatile unsigned long ulUARTBufferStart[TOTAL_PORT_NUM]={0}, ulUARTBufferEnd[UARTBufferLength]={0};
unsigned short CollectUARTData(const unsigned long ulCOMNo,char chrUARTBufferOutput[])
{
	unsigned long ulLength=0;
	unsigned long ulEnd ;
	unsigned long ulStart ;

#ifdef SYNCHRONOUS_MODE
	WaitForSingleObject(hReceiveEvent[ulIndexCorrect],INFINITE);
	ResetEvent(hReceiveEvent[ulIndexCorrect]);
#endif
	ulEnd = ulUARTBufferEnd[ulCOMNo];
	ulStart = ulUARTBufferStart[ulCOMNo];
	if (ulEnd == ulStart)
		return(0);
	if (ulEnd > ulStart)
	{
		memcpy((void*)chrUARTBufferOutput,(void*)(chrUARTBuffers[ulCOMNo]+ulStart),ulEnd-ulStart);
		ulLength = ulEnd-ulStart;
	}
	else
	{
		memcpy((void*)chrUARTBufferOutput,(void*)(chrUARTBuffers[ulCOMNo]+ulStart),UARTBufferLength-ulStart);
		if ( ulEnd != 0 )
		{
			memcpy((void*)(chrUARTBufferOutput+(UARTBufferLength-ulStart)),(void*)chrUARTBuffers[ulCOMNo],ulEnd);
		}
		ulLength = UARTBufferLength+ulEnd-ulStart;
	}
	ulUARTBufferStart[ulCOMNo] = ulEnd;
	return (unsigned short) ulLength;
}

signed char SendUARTMessageLength(unsigned char ulChannelNo, unsigned char chrSendBuffer[],const unsigned short usLen)
{
	DWORD iR;
	DWORD dwRes;
	DCB dcb;
	char chrDataToSend[1000] = {0};
	memcpy(chrDataToSend,chrSendBuffer,usLen);
	memcpy(&chrDataToSend[usLen],chrSendBuffer,usLen);

	GetCommState(hComDev[ulChannelNo] ,&dcb);
	dcb.fDtrControl = 0;//DTR = 1;发送
	SetCommState(hComDev[ulChannelNo] ,&dcb);

	if ( WriteFile(hComDev[ulChannelNo],chrSendBuffer,usLen,&iR,&(stcWriteStatus[ulChannelNo])) || GetLastError() != ERROR_IO_PENDING  ) 
		return -1;
	dwRes = WaitForSingleObject(stcWriteStatus[ulChannelNo].hEvent,1000);
	Sleep(10);
	dcb.fDtrControl = 1;//DTR = 0;接收
	SetCommState(hComDev[ulChannelNo] ,&dcb);
	Sleep(10);

	if(dwRes != WAIT_OBJECT_0 || ! GetOverlappedResult(hComDev[ulChannelNo], &stcWriteStatus[ulChannelNo], &iR, FALSE))
		return 0;
	return 0;
}

DWORD WINAPI ReceiveCOMData(PVOID pParam)
{
	unsigned long uLen;
	unsigned long ulLen1;
	unsigned long ulLen2;
	DWORD	dwRes;
	COMSTAT Comstat;
	DWORD dwErrorFlags;
	char chrBuffer[iBufferSize]={0};
	unsigned long ulUARTBufferEndTemp=ulUARTBufferEnd[0];

	unsigned long ulComNumber = 0;
	memcpy(&ulComNumber,pParam,4);


	while (1)
	{
		if ( ! ReadFile(hComDev[ulComNumber],chrBuffer,iBufferSize-1,&uLen,&(stcReadStatus[ulComNumber])) )
		{
			dwRes = GetLastError() ;
			if ( dwRes != ERROR_IO_PENDING)
			{
				ClearCommError(hComDev[ulComNumber],&dwErrorFlags,&Comstat);
				continue;
			}

			WaitForSingleObject(stcReadStatus[ulComNumber].hEvent,INFINITE);
			if ( !GetOverlappedResult(hComDev[ulComNumber], &(stcReadStatus[ulComNumber]), &uLen, FALSE))
				continue;
			if(uLen <= 0)
				continue;
			if ( (ulUARTBufferEndTemp + uLen) > UARTBufferLength )
			{
				ulLen1 = UARTBufferLength - ulUARTBufferEndTemp;
				ulLen2 = uLen - ulLen1;
				if (ulLen1 > 0)
				{
					memcpy((void *)&chrUARTBuffers[ulComNumber][ulUARTBufferEnd[ulComNumber]],(void *)chrBuffer,ulLen1);
				}
				if (ulLen2 > 0)
				{
					memcpy((void *)&chrUARTBuffers[ulComNumber][0],(void *)(chrBuffer+ulLen1),ulLen2);
				}
				ulUARTBufferEndTemp = ulLen2;
			}
			else
			{
				memcpy((void *)&chrUARTBuffers[ulComNumber][ulUARTBufferEnd[ulComNumber]],(void *)chrBuffer,uLen);
				ulUARTBufferEndTemp+=uLen;	
			}

			if (  ulUARTBufferEndTemp == ulUARTBufferStart[ulComNumber])
			{
				printf("Error!");
			}
			else
			{
				ulUARTBufferEnd[ulComNumber] = ulUARTBufferEndTemp;
			}

#ifdef SYNCHRONOUS_MODE
			SetEvent(hReceiveEvent[ucComNumber]);
#endif
			continue;
		}

		if(uLen <= 0)
			continue;
		if ( (ulUARTBufferEndTemp + uLen) > (UARTBufferLength) )
		{
			ulLen1 = UARTBufferLength - ulUARTBufferEndTemp;
			ulLen2 = uLen - ulLen1;
			if (ulLen1 > 0)
			{
				memcpy((void *)&chrUARTBuffers[ulComNumber][ulUARTBufferEnd[ulComNumber]],(void *)chrBuffer,ulLen1);
			}
			if (ulLen2 > 0)
			{
				memcpy((void *)&chrUARTBuffers[ulComNumber][0],(void *)(chrBuffer+ulLen1),ulLen2);
			}
			ulUARTBufferEndTemp = ulLen2;
		}
		else
		{
			memcpy((void *)&chrUARTBuffers[ulComNumber][ulUARTBufferEnd[ulComNumber]],(void *)chrBuffer,uLen);
			ulUARTBufferEndTemp+=uLen;	
		}

		if (  ulUARTBufferEndTemp== ulUARTBufferStart[ulComNumber])
		{
			printf("Error!");
		}
		else
		{
			ulUARTBufferEnd[ulComNumber] = ulUARTBufferEndTemp;
		}	

#ifdef SYNCHRONOUS_MODE
		SetEvent(hReceiveEvent[ucComNumber]);
#endif

	}
	return 0;
}

signed char OpenCOMDevice(const unsigned long ulPortNo,const unsigned long ulBaundrate)
{
	std::cout<<"start opening COM...\n";
	DWORD dwThreadID,dwThreadParam;
	COMSTAT Comstat;
	DWORD dwErrorFlags;
	DWORD dwRes;
	DCB dcb;
	COMMTIMEOUTS comTimeOut;
	TCHAR PortName[10] = {'\\','\\','.','\\','C','O','M',0,0,0};//"\\\\.\\COM";
	TCHAR chrTemple[5]={0};

	if(ulPortNo >= TOTAL_PORT_NUM)
	{
		printf("\nerror: exceed the max com port num\n");
		return -1;
	}


	_itot(ulPortNo+START_PORT_NUM,chrTemple,10);
	_tcscat(PortName,chrTemple);

	if((hComDev[ulPortNo] = CreateFile(PortName,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_FLAG_OVERLAPPED ,NULL))==INVALID_HANDLE_VALUE)
	//if((openHandle = CreateFile(PortName,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_FLAG_OVERLAPPED ,NULL))==INVALID_HANDLE_VALUE)
	{
		std::cout<<"hello world.\n";
		dwRes=GetLastError();
		return -1;
	}
	ulComMask |= 1<<ulPortNo;

	SetupComm(hComDev[ulPortNo] ,iBufferSize,iBufferSize);
	GetCommState(hComDev[ulPortNo] ,&dcb);
	dcb.BaudRate = ulBaundrate;
	dcb.fParity = NOPARITY;
	dcb.ByteSize=8;
	//dcb.fDtrControl = 1;//DTR = 0;接收
	//dcb.fRtsControl = 0;//RTS = 0;接收
	dcb.StopBits=ONESTOPBIT;

	SetCommState(hComDev[ulPortNo] ,&dcb);
	ClearCommError(hComDev[ulPortNo] ,&dwErrorFlags,&Comstat);
	dwRes = GetLastError();

	comTimeOut.ReadIntervalTimeout = 5;				
	comTimeOut.ReadTotalTimeoutMultiplier = 10;		
	comTimeOut.ReadTotalTimeoutConstant = 100;		
	comTimeOut.WriteTotalTimeoutMultiplier = 5;		
	comTimeOut.WriteTotalTimeoutConstant = 5;		
	SetCommTimeouts(hComDev[ulPortNo] ,&comTimeOut);	

	stcWriteStatus[ulPortNo] .hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	stcReadStatus[ulPortNo] .hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	stcReadStatus[ulPortNo].Internal = 0;
	stcReadStatus[ulPortNo].InternalHigh = 0;
	stcReadStatus[ulPortNo].Offset = 0;
	stcReadStatus[ulPortNo].OffsetHigh = 0;
	dwThreadParam = ulPortNo;
	hCOMThread[dwThreadParam] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ReceiveCOMData,&dwThreadParam,0,&dwThreadID);
	SetThreadPriority(hCOMThread[ulPortNo],THREAD_PRIORITY_NORMAL);
	Sleep(200);

	return 0;

} 

signed char SetBaundrate(const unsigned long ulPortNo,const unsigned long ulBaundrate)
{

	DCB dcb;	
	GetCommState(hComDev[ulPortNo] ,&dcb);
	dcb.BaudRate = ulBaundrate;
	SetCommState(hComDev[ulPortNo] ,&dcb);
	return 0;

} 
void CloseCOMDevice()
{
	unsigned char i;
	for(i=0 ; i<sizeof(ulComMask)*8 ; i++)
	{
		if((ulComMask & (1<<i))==0)
			continue;
		ulUARTBufferEnd[i] = 0;ulUARTBufferStart[i]=0;
		TerminateThread(hCOMThread[i],0);
		WaitForSingleObject(hCOMThread[i],10000);
		PurgeComm(hComDev[i],PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
		CloseHandle(stcReadStatus[i].hEvent);
		CloseHandle(stcWriteStatus[i].hEvent);
		CloseHandle(hComDev[i]);
	}
	ulComMask = 0;
}
unsigned char auchCRCHi[256] ={
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
	0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
	0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
	0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
	0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
	0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
	0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
	0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
	0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
	0x40} ;
	unsigned char auchCRCLo[256] ={
		0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
		0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
		0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 
		0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3, 
		0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7, 
		0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 
		0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 
		0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 
		0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2, 
		0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 
		0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 
		0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5, 
		0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91, 
		0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 
		0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 
		0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C, 
		0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80, 
		0x40 };
		unsigned short CRC16(unsigned char *puchMsg,unsigned short usDataLen) //CRC?校验
		{ 
			char uchCRCHi = 0xFF ; /* CRC高位*/
			char uchCRCLo = 0xFF ; /*CRC低位*/
			int uIndex ; /* CRC索引 */
			int i = 0;
			for (;i<usDataLen;i++)
			{
				uIndex = (int)(uchCRCHi ^ puchMsg[i]) ; /* ?? CRC */
				uchCRCHi = (char)(uchCRCLo ^ auchCRCHi[uIndex]) ;
				uchCRCLo = auchCRCLo[uIndex] ;
			}
			return (unsigned short)(((short)uchCRCHi << 8) | (short)uchCRCLo) ;
		}
		void read(unsigned short regId,unsigned short regNum)
		{
			signed readFlag;
			char cIndex=0;
			unsigned char record[20];
			short checkCRC;
			record[cIndex++]=chip_Add;
			record[cIndex++]=FuncR;
			record[cIndex++]=regId<<8;
			record[cIndex++]=regId&0xff;
			record[cIndex++]=regNum<<8;
			record[cIndex++]=regNum&0xff;
			checkCRC=CRC16(record,cIndex);
			record[cIndex++]=checkCRC&0xff; 
			record[cIndex++]=(checkCRC>>8)&0xff;
			readFlag=SendUARTMessageLength(ulComNo,record,8);
			if(readFlag!=0)
			{
				printf("read error!");
			}
		}