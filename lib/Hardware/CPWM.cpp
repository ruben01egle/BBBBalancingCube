/**
 * @file	CPWM.cpp
 * @author	Michael Meindl, Benjamin Spiegler
 * @date	23.03.22
 * @brief	Method definitions for CPWM.
 */
#include "CPWM.h"
#include "Assertion.h"
#include <cstdlib>
#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <cstring>
#include <string>
#include <sstream>

using namespace std;

CPWM::CPWM(const string& chipNr,
		   const string& pwmNr) : mDutyCycleFD(-1)
{
	string chip_path       = "/sys/class/pwm/pwmchip" + chipNr;
	string export_path 	   = chip_path + "/export";
	string pwm_path 	   = chip_path + "/pwm" + pwmNr;

	int ret = access(pwm_path.c_str(), F_OK);
	if(ret < 0)
	{
		int export_fd = open(export_path.c_str(), O_WRONLY);
		sAssertion(export_fd >= 0, "(CPWM::CPWM) Failed to open export", true);

		ret = write(export_fd, pwmNr.c_str(), pwmNr.size());
		sAssertion(ret == static_cast<int>(pwmNr.size()), "(CPWM::CPWM) Failed to export " + pwmNr, true);

		ret = close(export_fd);
		sAssertion(ret >= 0, "(CPWM::CPWM) Failed to close export", true);
	}
	//Disable the PWM-Generator
	string enable_path = pwm_path + "/enable";
	int enable_fd      = open(enable_path.c_str(), O_WRONLY);
	sAssertion(enable_fd >= 0, "(CPWM::CPWM) Failed to open " + enable_path, true);

	//ret = write(enable_fd, "0", strlen("0"));
	//sAssertion(ret == strlen("0"), "(CPWM::CPWM) Failed to write to enable", true);

	//Configure the Period
	string period_path = pwm_path + "/period";
	int period_fd      = open(period_path.c_str(), O_WRONLY);
	sAssertion(period_fd >= 0, "(CPWM::CPWM) Failed to open "  + period_path, true);

	ret = write(period_fd, "2000000", strlen("2000000"));
	sAssertion(ret == static_cast<int>(strlen("2000000")), "(CPWM::CPWM) Failed to write to period", true);

	ret = close(period_fd);
	sAssertion(ret >= 0, "(CPWM::CPWM) Failed to close " + period_path, true);

	//Set the Duty-Cycle to 0
	string dc_path = pwm_path + "/duty_cycle";
	int dc_fd	   = open(dc_path.c_str(), O_WRONLY);
	sAssertion(dc_fd >= 0, "(CPWM::CPWM) Failed to open " + dc_path, true);

	ret = write(dc_fd, "0", strlen("0"));
	sAssertion(ret == strlen("0"), "(CPWM::CPWM) Failled to write to duty_cycle", true);

	mDutyCycleFD = dc_fd;

	//Enable the PWM-Generator
	ret = write(enable_fd, "1", strlen("1"));
	sAssertion(ret == strlen("1"), "(CPWM::CPWM) Failed to write to enable", true);

	ret = close(enable_fd);
	sAssertion(ret >= 0, "(CPWM::CPWM) Failed to close " + enable_path, true);
}
bool CPWM::setDutyCycle(float duty_cycle)
{
	ostringstream oss;
	oss << static_cast<int>(duty_cycle*sPeriod/100.0F);
	string dcStr = oss.str();

	int ret = write(mDutyCycleFD, dcStr.c_str(), dcStr.size());
	return ret == static_cast<int>(dcStr.size());
}

CPWM::~CPWM() {
	close(mDutyCycleFD);
}
