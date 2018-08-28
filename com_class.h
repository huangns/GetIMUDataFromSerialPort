#ifndef COM_CLASS_H
#define COM_CLASS_H
#include <iostream>
#include <string>
#include <Windows.h>
class comclass
{
public:
	//data member
	int comId_;
	int baud_;
	HANDLE hcom_;
	char readStr_[100];
	char startStr_[7];
	char stopStr_[7];
	int readCount_;
	//function member	
	comclass(int comId, int baud);
	bool openCom();
	bool isOpen();
	bool closeCom();
	bool setCom();
	bool writeComData(char* writeStr, int& writeCount);
	bool readComData(char* readStr, int& readCount);
	bool sendStart();
	bool sendStop();
	bool getData(char* data, int& dataCount);
	bool getIMUData(double* imudata,double* getdatatime);

};



#endif
