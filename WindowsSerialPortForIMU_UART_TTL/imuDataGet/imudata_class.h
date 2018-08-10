#ifndef IMUDATA_CLASS_H
#define IMUDATA_CLASS_H
#include <fstream>
#include <iostream>
class IMUMeasurement//数据是角度制的，根据需要判断是否需要转成弧度制
{
public:
	double IMU_w_m_[3];
	double IMU_a_m_[3];
	double getDataTime_;
	double imutime;
	double mytime;
	bool imuDataReady_;
	bool isReadyToGetData_;
	IMUMeasurement();
	void DataPrint();
	void DataSave(std::ofstream& saveAccData, std::ofstream& saveGyroData);
};


#endif
