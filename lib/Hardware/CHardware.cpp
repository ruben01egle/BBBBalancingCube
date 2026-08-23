#include "CHardware.hpp"

#include "CubeConstants.hpp"

using namespace Cube;

CHardware::CHardware():	mSensor1(IMU_SPI_MODULE, IMU1_SPI_CHANNEL, IMU_SPI_CHANNEL_CFG),
								mSensor2(IMU_SPI_MODULE, IMU2_SPI_CHANNEL, IMU_SPI_CHANNEL_CFG),
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


bool CHardware::init()
{
	if (mSensor1.initImu(IMU_SETUP) != CMPU9250::Status::OKAY) {
		return false;
	}
	if (mSensor2.initImu(IMU_SETUP) != CMPU9250::Status::OKAY) {
		return false;
	}

	if (mMotor.init() != CMaxonMotor::Status::OKAY) {
        return false;
    }

    return true;
}

bool CHardware::fetchValues(uint16_t& adcValue,
		 CIMUData& sensor1Data,
		 CIMUData& sensor2Data)
{

	if (mMotor.getRawVelocity(adcValue) != CMaxonMotor::Status::OKAY) {
		return false;
	}

	CMPU9250::rawData raw1;
	if (mSensor1.readImu(raw1) != CMPU9250::Status::OKAY) {
		return false;
	}
	sensor1Data.mA_x = raw1.xAccel;
	sensor1Data.mA_y = raw1.yAccel;
	sensor1Data.mA_z = raw1.zAccel;
	sensor1Data.mW_x = raw1.xGyro;
	sensor1Data.mW_y = raw1.yGyro;
	sensor1Data.mW_z = raw1.zGyro;

	CMPU9250::rawData raw2;
	if (mSensor2.readImu(raw2) != CMPU9250::Status::OKAY) {
		return false;
	}
	sensor2Data.mA_x = raw2.xAccel;
	sensor2Data.mA_y = raw2.yAccel;
	sensor2Data.mA_z = raw2.zAccel;
	sensor2Data.mW_x = raw2.xGyro;
	sensor2Data.mW_y = raw2.yGyro;
	sensor2Data.mW_z = raw2.zGyro;

	return true;

}

bool CHardware::enableMotor()
{
	if (mMotor.enable() != CMaxonMotor::Status::OKAY){
		return false;
	}
	return true;
}

bool CHardware::disableMotor()
{
	if (mMotor.disable() != CMaxonMotor::Status::OKAY) {
		return false;
	}
	return true;
}

bool CHardware::setTorque(float torque)
{
	if (mMotor.setTorque(torque) != CMaxonMotor::Status::OKAY) {
		return false;
	}
	return true;
}

