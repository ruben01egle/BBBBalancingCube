/**
 * @file	CGPIO.h
 * @author	Michael Meindl, Benjamin Spiegler
 * @date	23.03.22
 * @brief	Class to handle the GPIO-Outputs.
 */
#ifndef CGPIO_H
#define CGPIO_H
#include "Global.h"
#include "Assertion.h"
#include <string>

class CGPIO
{
public:
	bool setHigh();
	bool setLow();
public:
	CGPIO(const std::string& gpioNr);
	CGPIO(const CGPIO&) = delete;
	CGPIO& operator=(const CGPIO&) = delete;
	~CGPIO();
private:
	int mValueFD;
};

#endif
