/**
 * @file	CContainer.cpp
 * @author	Michael Meindl
 * @date	5.12.2016
 * @brief	Method definitions for the container.
 */
#include "CContainer.hpp"
#include "CIMUData.hpp"
#include "CErrorReporter.hpp"


CContainer::CContainer()
{
	if (!mReadSem.init(false, false)) {
		REPORT_ERROR("CContainer: failed to init read semaphore");
	}
}
bool CContainer::getContent(bool waitForever,
							CContent& content)
{
	if(mReadSem.take(waitForever))
	{
		content = mContent;
		return true;
	}
	return false;
}
void CContainer::signalReader()
{
	mReadSem.give();
}

bool CContainer::writeData(const int64_t timeUs,
                              const uint16_t adcValue,
                              const float torque,
                              const CIMUData& sensor1,
                              const CIMUData& sensor2,
                              const CIMUDataCalibrated& sensor1Calib,
                              const CIMUDataCalibrated& sensor2Calib,
                              const CStateVectorData& stateData)
{
    mContent.mTimeUs          = timeUs;
    mContent.mADCValue        = adcValue;
    mContent.mMotorTorque     = torque;
    mContent.mSensor1Data     = sensor1;
    mContent.mSensor2Data     = sensor2;
    mContent.mSensor1DataCalib = sensor1Calib;
    mContent.mSensor2DataCalib = sensor2Calib;
    mContent.mStateData       = stateData;

    return true;
}

bool CContainer::writeTime(const int64_t timeUs)
{
	mContent.mTimeUs = timeUs;
	return true;
}
bool CContainer::writeADCValue(const uint16_t adcValue)
{
	mContent.mADCValue = adcValue;
	return true;
}
bool CContainer::writeTorqueValue(const float torque)
{
	mContent.mMotorTorque = torque;
	return true;
}
bool CContainer::writeSensor1Data(const CIMUData& sensorData)
{
	mContent.mSensor1Data = sensorData;
	return true;
}
bool CContainer::writeSensor2Data(const CIMUData& sensorData)
{
	mContent.mSensor2Data = sensorData;
	return true;
}
bool CContainer::writeSensor1CalibData(const CIMUDataCalibrated& sensorData)
{
	mContent.mSensor1DataCalib = sensorData;
	return true;
}
bool CContainer::writeSensor2CalibData(const CIMUDataCalibrated& sensorData)
{
	mContent.mSensor2DataCalib = sensorData;
	return true;
}
bool CContainer::writeStateData(const CStateVectorData& stateValue)
{
	mContent.mStateData = stateValue;
	return true;
}
