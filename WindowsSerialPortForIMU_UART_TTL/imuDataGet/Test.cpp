// UARTTest.cpp : 定义控制台应用程序的入口点。
//


#include "Com.h"

#include "windows.h"
#include "time.h"
#include "stdio.h"
#include "JY901.h"
#include <iostream>
#include <fstream>
#include "imudata_class.h"

unsigned char ucComNo[2]={0,0};
extern unsigned long ulComNo;

std::ofstream saveAccData("saveAccData.txt");
std::ofstream saveGyroData("saveGyroData.txt");

int main()
{
	char chrBuffer[2000];
	unsigned short usLength=0,usCnt=0;
	unsigned long ulBaund=115200,ulComNo=9;
	signed char cResult= 1;	
	//printf("请输入串口号:\r\nCom = ");
	//scanf("%ld",&ulComNo);
	//printf("请输入波特率:(9600、115200或其他)\r\nBaud = ");
	//scanf("%ld",&ulBaund);
	//printf("等待打开串口%d...\r\n",ucComNo);
	//while(cResult!=0)
	//{
		if(cResult = OpenCOMDevice(ulComNo,ulBaund)==0)
		{
			std::cout<<"Open success.\n";
			//CloseCOMDevice();
		}
		else
		{
			std::cout<<"Open failed.\n";
			return 1;
		}

		unsigned char START_DATA[6]={0xA5, 0x5A, 0x04, 0x01,0x05, 0xAA};
		unsigned char STOP_DATA[6]={0xA5, 0x5A, 0x04, 0x02,0x06, 0xAA};
		SendUARTMessageLength(ulComNo,START_DATA,6);
		int i=0;
		double getdatatime=0;
		IMUMeasurement imudata;
		double gromutip = 3.14159265358979323846/180;
		double accmutip=1.0/16384.0*9.81;
		while(i<100)
		{
			//std::cout<<"time 1: "<<(double)clock()/CLOCKS_PER_SEC;
			getdatatime=(double)clock()/CLOCKS_PER_SEC;
			int recevLength=CollectUARTData(ulComNo,chrBuffer);
			if (recevLength!=0)
			{

				//for (int i=0;i<83;++i)
				//{
					//saveAccData<<std::hex<<(unsigned int)(unsigned char)chrBuffer[i]<<" ";
				//}
				
				saveAccData<<std::dec<<"\n";
				imudata.getDataTime_=getdatatime;
				imudata.IMU_a_m_[0]=(double)(((short)chrBuffer[26])<<8|((unsigned short)(unsigned char)chrBuffer[27]))*accmutip;
				imudata.IMU_a_m_[1]=(double)(((short)chrBuffer[28])<<8|((unsigned short)(unsigned char)chrBuffer[29]))*accmutip;
				imudata.IMU_a_m_[2]=(double)(((short)chrBuffer[30])<<8|((unsigned short)(unsigned char)chrBuffer[31]))*accmutip;
		
				//std::cout<<"1: "<<((short)(( ( (unsigned short)chrBuffer[30] )<<8 )|((unsigned short)chrBuffer[31])))<<"\t"<<imudata.IMU_a_m_[2]<<"\t"<<accmutip<<"\t";


				imudata.IMU_w_m_[0]=(double)(((short)chrBuffer[32])<<8|((unsigned short)(unsigned char)chrBuffer[33]))*gromutip;
				imudata.IMU_w_m_[1]=(double)(((short)chrBuffer[34])<<8|((unsigned short)(unsigned char)chrBuffer[35]))*gromutip;
				imudata.IMU_w_m_[2]=(double)(((short)chrBuffer[36])<<8|((unsigned short)(unsigned char)chrBuffer[37]))*gromutip;
				imudata.DataSave(saveAccData,saveGyroData);
				std::cout<<recevLength<<" ";
				std::cout<<(double)clock()/CLOCKS_PER_SEC<<"\n";
				++i;
			}
			//Sleep(10);
		}

		SendUARTMessageLength(ulComNo,STOP_DATA,6);
		CloseCOMDevice();
	//}



	return 0;
}


