#ifndef IMUDATA_CLASS_H
#define IMUDATA_CLASS_H
#include <fstream>
#include <iostream>
class IMUMeasurement//�����ǽǶ��Ƶģ�������Ҫ�ж��Ƿ���Ҫת�ɻ�����
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
