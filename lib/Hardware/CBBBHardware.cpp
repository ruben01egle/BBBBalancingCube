/**
 * @author	Michael Meindl, Benjamin Spiegler
 * @date	23.03.22
 * @brief	Method definitions for CBBBHardware.
 */


#include <time.h>
#include <sys/time.h>
#include <stdio.h>

#include "CBBBHardware.h"
#include <unistd.h>


CBBBHardware::CBBBHardware() : mSensor1("/dev/spidev1.0"), mSensor2("/dev/spidev1.1"), mMotor{"0", "1", "546", "547", 2.0f, 0.0369f, 1.0f}

{
	mSensor1.init(0b00011000U);		// setting checked ok. JW 24.4.22
	mSensor2.init(0b00011000U);

}
bool CBBBHardware::fetchValues(uint16_t& adcValue,
		 CIMUData& sensor1Data,
		 CIMUData& sensor2Data)
{
	 if(!mADC.fetchValue(adcValue)) //Original: !mADC.fetchValue(adcValue)
		return false;
//	adcValue = 2035;									// to do
	if(!mSensor1.readSensorData(sensor1Data))
		return false;
	if(!mSensor2.readSensorData(sensor2Data))
		return false;
	return true;

}
bool CBBBHardware::enableMotor()
{
	mMotor.enableMotor();
	return true;
}
bool CBBBHardware::disableMotor()
{
	mMotor.disableMotor();
	return true;
}
bool CBBBHardware::setTorque(float torque)
{
	mMotor.setTorque(torque);
	return true;
}
