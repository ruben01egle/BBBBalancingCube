/**
 * @file	CMotor.cpp
 * @author	Michael Meindl, Benjamin Spiegler
 * @date	23.03.22
 * @brief	Method definitios for CMotor.
 */
#include "CMotor.h"

CMotor::CMotor(const std::string& pwmchipNr,
			   const std::string& pwmNr,
			   const std::string& enablePinNr,
			   const std::string& directionPinNr,
			   const float maxCurrent,
			   const float torqueConstant,
			   const float gearRatio)
				: mTorquePWM(pwmchipNr, pwmNr),
				  mEnablePin(enablePinNr),
				  mDirectionPin(directionPinNr),
				  sMaxCurrent(maxCurrent),
				  sTorqueConstant(torqueConstant),
				  sGearRatio(gearRatio)
{
	mEnablePin.setLow();
	mTorquePWM.setDutyCycle(10.0F);
}
void CMotor::enableMotor()
{
	mEnablePin.setHigh();
}
void CMotor::disableMotor()
{
	mEnablePin.setLow();
	this->setTorque(0.0F);
}
void CMotor::setTorque(float torque)
{
	torque >= 0 ? mDirectionPin.setLow() : mDirectionPin.setHigh();
	torque = torque >= 0 ? torque : -torque;
	torque = torque > sMaxTorque ? sMaxTorque : torque;
	float pwm_value = 10.0F + 80.0F*(torque/sMaxTorque);
	mTorquePWM.setDutyCycle(pwm_value);
}

CMotor::~CMotor()
{
	this->disableMotor();
}
