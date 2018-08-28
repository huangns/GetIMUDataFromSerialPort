#include "com_class.h"
#include <string>
#include <tchar.h>
#include <ctime>
#include <fstream>
#define PRINT_HEX_DATA
std::ofstream hexData("hexData.txt");
comclass::comclass(int comId, int baud)
	:comId_(comId),baud_(baud),hcom_(INVALID_HANDLE_VALUE),readCount_(83)
{
	startStr_[0]=0xA5;
	startStr_[1]=0x5A;
	startStr_[2]=0x04;
	startStr_[3]=0x01;
	startStr_[4]=0x05;
	startStr_[5]=0xAA;
	startStr_[6]='\0';

	stopStr_[0]=0xA5;
	stopStr_[1]=0x5A;
	stopStr_[2]=0x04;
	stopStr_[3]=0x02;
	stopStr_[4]=0x06;
	stopStr_[5]=0xAA;
	stopStr_[6]='\0';
	//char START_DATA[6]={, 0x5A, 0x04, 0x01,0x05, 0xAA};
	//char STOP_DATA[6]={, , , ,, };
}

bool comclass::openCom()
{
	TCHAR PortName[10] = {'\\','\\','.','\\','C','O','M',0,0,0};//"\\\\.\\COM";//完整的串口名COM+comId
	TCHAR chrTemple[5]={0};
	_itot(comId_,chrTemple,10);
	_tcscat(PortName,chrTemple);
	hcom_=CreateFile(PortName,
		GENERIC_READ|GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,//同步方式
		NULL);
	if((hcom_==0)||(unsigned long)hcom_==0xffffffff)
	//if (hcom_==INVALID_HANDLE_VALUE)
	{
		std::cerr<<"Open COM failed!\n";
		return false;
	}
	return setCom();
}

bool comclass::isOpen()
{
	return (hcom_!=0);
}

bool comclass::closeCom()
{
	//if(hcom_!=INVALID_HANDLE_VALUE)
	if(hcom_)
	{
		std::cout<<"Closing COM...\n";
		return CloseHandle(hcom_);
	}
	return true;
}

//配置串口
bool comclass::setCom()
{
	//设定缓冲区大小
	SetupComm(hcom_,1024,1024);
	//奇偶校验 波特率等配置
	DCB comDCB;
	if (!GetCommState(hcom_,&comDCB))
	{
		std::cerr<<"set COM failed in GetCommState."<<std::endl;
		return false;
	}
	comDCB.BaudRate=baud_;
	comDCB.ByteSize=8;
	comDCB.Parity=NOPARITY;
	comDCB.StopBits=ONESTOPBIT;
	if(!SetCommState(hcom_,&comDCB))
	{
		std::cerr<<"set COM failed in SetCommState."<<std::endl;
		return false;
	}

	//设置读写超时
	COMMTIMEOUTS comTimeOut;
	memset(&comTimeOut, 0, sizeof(comTimeOut));
	comTimeOut.ReadIntervalTimeout=1;
	comTimeOut.ReadTotalTimeoutMultiplier=0;
	comTimeOut.ReadTotalTimeoutConstant=1;
	comTimeOut.WriteTotalTimeoutMultiplier=5;
	comTimeOut.WriteTotalTimeoutConstant=5;

	if(!SetCommTimeouts(hcom_,&comTimeOut))
	{
		std::cerr<<"set COM failed in SetCommTimeouts."<<std::endl;
	}
	//清空缓冲区
	PurgeComm(hcom_,PURGE_TXCLEAR|PURGE_RXCLEAR);

	DWORD dwError;
	COMSTAT cs;
	return (hcom_!=0&&ClearCommError(hcom_,&dwError,&cs));
}
bool comclass::writeComData(char* writeStr, int& writeCount)
{
	if (hcom_==NULL||writeStr==NULL)
	{
		return false;
	}
	DWORD dwBytesWrite=writeCount;
	COMSTAT ComStat;
	DWORD dwErrorFlags;
	BOOL bWriteStat;
	DWORD bytesWritten = 0;
	ClearCommError(hcom_,&dwErrorFlags,&ComStat);
	PurgeComm(hcom_, PURGE_TXABORT| PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
	bWriteStat=WriteFile(hcom_,(unsigned char*)writeStr,dwBytesWrite,&bytesWritten,NULL);
	writeCount=(int)bytesWritten;
	if(!bWriteStat)
	{
		std::cerr<<"write Com failed."<<std::endl;
		return false;
	}
	//ClearCommError(hcom_,&dwErrorFlags,&ComStat);
	//PurgeComm(hcom_, PURGE_TXABORT| PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
	return true;
}

bool comclass::readComData(char* readStr, int& readCount)
{
	if(hcom_==NULL||readStr==NULL)
	{
		return false;
	}
	DWORD wCount;//读取的字节数
	BOOL bReadStat;
	COMSTAT ComStat;
	DWORD dwErrorFlags;

	DWORD bytesRead,bytesReadTotal;
	int   timeOutCounter, bytesLeft;
	bytesRead=0;
	bytesReadTotal=0;
	int timeOutMS=1000;
	timeOutCounter=timeOutMS;
	bytesLeft=readCount;
	int dataLength=readCount;
	bool findfirst=true;
	bool i=0;
	while(timeOutCounter>0)
	{
		bytesRead=0;
		ReadFile(hcom_,(unsigned char*)(readStr+bytesReadTotal),1,&bytesRead,NULL);
		if(bytesRead)
		{
			
		    if(!findfirst)
			{
				int i=0;
				int temp=bytesRead;
				while(i<temp-1&&((unsigned char)readStr[i]!=0xa5||(unsigned char)readStr[i+1]!=0x5a))
				{
					i++;
					bytesRead--;
				}
				if(((unsigned char)readStr[i]!=0xa5&&(unsigned char)readStr[i+1]==0x5a))
				{
					findfirst=true;
					for (int k=0;i!=0&&i<temp;++k,++i)
					{
						readStr[k]=readStr[i];
					}
				}
			}
			dataLength-=bytesRead;
			bytesReadTotal+=bytesRead;
			if(dataLength==0) break;
			timeOutCounter=timeOutMS;
		}else
		{
			timeOutCounter-=1;
		}
	}
	readCount=bytesReadTotal;
	//ClearCommError(hcom_,&dwErrorFlags,&ComStat);
	//PurgeComm(hcom_, PURGE_TXABORT| PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
	/*
	bReadStat=ReadFile(hcom_,(unsigned char*)readStr,readCount,&wCount,NULL);
	readCount=(int)wCount;
	if(!bReadStat)
	{
		std::cerr<<"read Com Data failed."<<std::endl;
		return false;
	}
	*/
	//PurgeComm(hcom_, PURGE_TXABORT| PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
	return TRUE; //同步写串口
}

bool comclass::sendStart()
{
	int writeCount=6;
	bool result = writeComData(startStr_,writeCount);
	//std::cout<<"result start: "<<writeCount<<std::endl;
	return result;
}

bool comclass::sendStop()
{
	int writeCount=6;
	return writeComData(stopStr_,writeCount);
}

bool comclass::getData(char* data, int& dataCount)
{
	return readComData(data,dataCount);
}

bool comclass::getIMUData(double* imudata,double* getdatatime)
{

	*getdatatime=(double)std::clock()/CLOCKS_PER_SEC;
    int chararrayCount=readCount_;
	getData(readStr_,chararrayCount);

#ifdef PRINT_HEX_DATA
	
	for(int j=0;j<chararrayCount;++j)
	{
		hexData<<std::hex<<(int)(unsigned char)readStr_[j]<<" ";
	}
	hexData<<std::endl;
#endif

	double gromutip = 1.0/32.8*3.14159265358979323846/180.0;
	double accmutip=1.0/16384.0*9.81;

	if (chararrayCount!=0)
	{
		//memcpy(tmp,chrBuffer,sizeof(unsigned char)*recevLength);
		bool found = false;
		for(int kk=0;kk<chararrayCount-1&&!found;++kk)
		{
			if((unsigned char)readStr_[kk]==0xA5 && (unsigned char)readStr_[kk+1]==0x5A )
			{
				unsigned char* data=(unsigned char*)readStr_+kk;
				if( (data[2]!=0x14&&data[3]!=0xA1) && 
					(data[2]!=0x16&&data[3]!=0xA2) &&
					(data[2]!=0x13&&data[3]!=0xA6) &&
					(data[2]!=0x0E&&data[3]!=0xA3) || (chararrayCount-kk<13)
					)    continue;
				unsigned int len=data[2];
				unsigned int checksum=0;
				for(int i=0;i<len;++i)
				{
					checksum+=(unsigned int)data[i];
				}
				unsigned int check=checksum%256+1;
				unsigned int check_true=data[len];
				if(check!=check_true)
				{
					continue;
				}

				if(data[3]==0xA2)
				{
					//imudata.getDataTime_=getdatatime;
					imudata[0]=(double)(short)(((unsigned short)(data[4]))<<8|(unsigned short)(data[5]))*accmutip;
					imudata[1]=(double)(short)(((unsigned short)(data[6]))<<8|(unsigned short)(data[7]))*accmutip;
					imudata[2]=(double)(short)(((unsigned short)(data[8]))<<8|(unsigned short)(data[9]))*accmutip;

					imudata[3]=(double)(short)(((unsigned short)(data[10]))<<8|(unsigned short)(data[11]))*gromutip;
					imudata[4]=(double)(short)(((unsigned short)(data[12]))<<8|(unsigned short)(data[13]))*gromutip;
					imudata[5]=(double)(short)(((unsigned short)(data[14]))<<8|(unsigned short)(data[15]))*gromutip;

					//std::cout<<std::hex<<(unsigned int)data[10]<<" "<<(unsigned int)data[11]<<" "<<(unsigned int)data[12]<<" "
					//	<<(unsigned int)data[13]<<" "<<(unsigned int)data[14]<<" "<<(unsigned int)data[15]<<"\n";
					//imudata.DataSave(saveAccData,saveGyroData);
					found=true;
					return true;
				}
			}
		}
	}
}