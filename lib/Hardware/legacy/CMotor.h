/**
 * @file	CMotor.h
 * @author	Michael Meindl, Benjamin Spiegler
 * @date	23.03.22
 * @brief	Class to initialize and utilize the motors.
 */
#ifndef CMOTOR_H
#define CMOTOR_H
#include "CPWM.h"
#include "CGPIO.h"

class CMotor
{
public:
	void enableMotor();
	void disableMotor();
	void setTorque(float torque);
public:
	CMotor(const std::string& pwmchipNr,
		   const std::string& pwmNr,
		   const std::string& enablePinNr,
		   const std::string& directionPinNr,
		   const float maxCurrent,
		   const float torqueConstant,
		   const float gearRatio);

	CMotor(const CMotor&) = delete;
	CMotor& operator=(const CMotor&) = delete;
	~CMotor();
private:
	CPWM mTorquePWM;
	CGPIO mEnablePin;
	CGPIO mDirectionPin;
	float sMaxCurrent;	// maxCurrent: 3,21A bei 90% PWM. Max. Cont. eigentlich 2,7A
	float sTorqueConstant;
	float sGearRatio;
	float sMaxTorque{sMaxCurrent * sTorqueConstant * sGearRatio};
};

#endif
