#include "CHardware.hpp"

#include "CubeConstants.hpp"

using namespace Cube;

CBBBHardware::CBBBHardware():	mSensor1("/dev/spidev1.0"),
								mSensor2("/dev/spidev1.1"),
								mMotor(MOTOR_PWM_MODULE,
                                       MOTOR_PWM_PIN,
                                       MOTOR_ENABLE_GPIO,
									   MOTOR_DIRECTION_GPIO,
                                       MOTOR_PWM_MODULE_CFG,
                                       MOTOR_PWM_CFG,
                                       ADC_STEP_IDX,
                                       ADC_CFG,
                                       MOTOR_TORQUE_CONST)

{
}


bool CBBBHardware::init()
{
	mSensor1.init(0b00011000U);		// setting checked ok. JW 24.4.22
	mSensor2.init(0b00011000U);

	if (mMotor.init() != CMaxonMotor::Status::OKAY) {
        return false;
    }

    return true;
}

bool CBBBHardware::fetchValues(uint16_t& adcValue,
		 CIMUData& sensor1Data,
		 CIMUData& sensor2Data)
{

	if (mMotor.getRawVelocity(adcValue) != CMaxonMotor::Status::OKAY) {
		return false;
	}

	if(!mSensor1.readSensorData(sensor1Data))
		return false;
	if(!mSensor2.readSensorData(sensor2Data))
		return false;
	return true;

}

bool CBBBHardware::enableMotor()
{
	if (mMotor.enable() != CMaxonMotor::Status::OKAY){
		return false;
	}
	return true;
}

bool CBBBHardware::disableMotor()
{
	if (mMotor.disable() != CMaxonMotor::Status::OKAY) {
		return false;
	}
	return true;
}

bool CBBBHardware::setTorque(float torque)
{
	if (mMotor.setTorque(torque) != CMaxonMotor::Status::OKAY) {
		return false;
	}
	return true;
}

