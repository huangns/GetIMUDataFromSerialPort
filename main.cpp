#include <iostream>
#include <fstream>
#include "com_class.h"
#include <ctime>
std::ofstream dataout("dataout.txt");

int main()
{
	std::cout<<"Hello world.\n";
	comclass cc(9,115200);
	if (cc.openCom())
	{
		std::cout<<"open COM success.\n";
		//cc.closeCom();
	}else
	{
		return -1;
	}
	if (!cc.sendStart())
	{
		std::cerr<<"send start message failed."<<std::endl;
		return -1;
	}
	/*
	char data[100];
	int dataCount=83;
	for(int i=0;i<100;++i)
	{
		if(cc.getData(data,dataCount))
		{
			std::cout<<"dataCount: "<<dataCount<<std::endl;
			dataout<<"dataCount: "<<dataCount<<std::endl;
			for(int j=0;j<dataCount;++j)
			{
				dataout<<std::hex<<(int)(unsigned char)data[j]<<std::dec<<" ";
			}
			dataout<<"\n";

		}else
		{
			std::cout<<"getdata failed."<<std::endl;
		}
		dataCount=83;

	}
	*/
	double imudataAndTime[7]={0};
	for(int i=0;i<100;++i)
	{
		if(cc.getIMUData(imudataAndTime,&imudataAndTime[6]))
		{
			dataout<<imudataAndTime[6]<<" ";
			for(int j=0;j<6;++j)
			{
				dataout<<imudataAndTime[j]<<" ";
			}
			dataout<<std::endl;
		}
	}

	if(!cc.sendStop())
	{
		std::cerr<<"send start message failed."<<std::endl;
		return -1;
	}
	
	if (cc.closeCom())
	{
		std::cout<<"close COM."<<std::endl;
	}
	std::cout<<"OK."<<std::endl;
	//cc.closeCom();
	return 0;
}