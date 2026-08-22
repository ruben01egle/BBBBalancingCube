#ifndef CBBBHARDWARE_HPP
#define CBBBHARDWARE_HPP

#include <cstdint>
#include "CMaxonMotor.hpp"
#include "CMPU9250.hpp"
#include "CIMUData.h"

class CBBBHardware
{
public:
	bool fetchValues(uint16_t& adcValue,
					 CIMUData& sensor1Data,
					 CIMUData& sensor2Data);
	bool enableMotor();
	bool disableMotor();
	bool setTorque(float torque);

public:
	CBBBHardware();
	bool init();
private:
	CMPU9250 mSensor1, mSensor2;
	CMaxonMotor mMotor;
};

#endif
