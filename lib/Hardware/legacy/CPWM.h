/**
 * @file	CPWM.h
 * @author	Michael Meindl, Benjamin Spiegler
 * @date	23.03.22
 * @brief	Class to initialize and utilize the PWMs.
 */
#ifndef CPWM_H
#define CPWM_H
#include "Global.h"
#include <string>

class CPWM
{
public:
	bool setDutyCycle(float duty_cycle);
public:
	CPWM(const std::string& chipNr,
	     const std::string& pwmNr);
	CPWM(const CPWM&) = delete;
	CPWM& operator=(const CPWM&) = delete;
	~CPWM();
private:
	static constexpr float sPeriod    = 2000000.0F;
	int mDutyCycleFD;
};

#endif
