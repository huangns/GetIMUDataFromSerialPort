#include "imudata_class.h"

IMUMeasurement::IMUMeasurement():imuDataReady_(false),isReadyToGetData_(true),getDataTime_(0.0),imutime(0.0),mytime(0.0)
{
	IMU_a_m_[0]=0;
	IMU_a_m_[1]=0;
	IMU_a_m_[2]=0;

	IMU_w_m_[0]=0;
	IMU_w_m_[1]=0;
	IMU_w_m_[2]=0;
};

void IMUMeasurement::DataSave(std::ofstream& saveAccData, std::ofstream& saveGyroData)
{
	saveAccData<<getDataTime_<<"\t"<<IMU_a_m_[0]<<"\t"<<IMU_a_m_[1]<<"\t"<<IMU_a_m_[2]<<"\n";
	saveGyroData<<getDataTime_<<"\t"<<IMU_w_m_[0]<<"\t"<<IMU_w_m_[1]<<"\t"<<IMU_w_m_[2]<<"\n";
	//saveAccData<<getDataTime_<<"\t"<<IMU_a_m_.x()<<"\t"<<IMU_a_m_.y()<<"\t"<<IMU_a_m_.z()<<std::endl;
	//saveGyroData<<getDataTime_<<"\t"<<IMU_w_m_.x()<<"\t"<<IMU_w_m_.y()<<"\t"<<IMU_w_m_.z()<<std::endl;
}