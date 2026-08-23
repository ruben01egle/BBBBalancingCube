#pragma once

#include <cstdint>
#include "CMaxonMotor.hpp"
#include "CMPU9250.hpp"
#include "CIMUData.hpp"

class CHardware
{
public:
	bool fetchValues(uint16_t& adcValue,
					 CIMUData& sensor1Data,
					 CIMUData& sensor2Data);
	bool enableMotor();
	bool disableMotor();
	bool setTorque(float torque);

public:
	CHardware();
	bool init();
private:
	CMPU9250 mSensor1, mSensor2;
	CMaxonMotor mMotor;
};
