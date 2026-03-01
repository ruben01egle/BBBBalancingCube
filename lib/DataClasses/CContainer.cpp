/**
 * @file	CContainer.cpp
 * @author	Michael Meindl
 * @date	5.12.2016
 * @brief	Method definitions for the container.
 */
#include "CContainer.h"
#include "CIMUData.h"


CContainer::CContainer()
{
	mReadSem.init(false, false);
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
                              const CStateVectorData& stateData)
{
    mContent.mTimeUs       = timeUs;
    mContent.mADCValue     = adcValue;
    mContent.mMotorTorque  = torque;
    mContent.mSensor1Data  = sensor1;
    mContent.mSensor2Data  = sensor2;
    mContent.mStateData    = stateData;

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
bool CContainer::writeStateData(const CStateVectorData& stateValue)
{
	mContent.mStateData = stateValue;
	return true;
}
