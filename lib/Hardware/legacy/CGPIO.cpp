/**
 * @file	CGPIO.cpp
 * @author	Michael Meindl, Benjamin Spiegler
 * @date	23.03.22
 * @brief	Method definitions for CGPIO.
 */
#include "CGPIO.h"
#include <cstdlib>
#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

CGPIO::CGPIO(const std::string& pinNr) : mValueFD(-1)
{
	string export_path = "/sys/class/gpio/export";
	string pin_path    = "/sys/class/gpio/gpio" + pinNr;
	int ret = access(pin_path.c_str(), F_OK);
	if(ret < 0)
	{
		int export_fd = open(export_path.c_str(), O_WRONLY);
		sAssertion(export_fd >= 0, "(CGPIO::CGPIO) Failed to export " + pinNr, true);
		ret = write(export_fd, pinNr.c_str(), pinNr.size());
		sAssertion(ret == static_cast<int>(pinNr.size()),
				   "(CGPIO::CGPIO) Failed to write " + pinNr + " to export.", true);
		ret = close(export_fd);
		sAssertion(ret >= 0, "(CGPIO::CGPIO): Failed to close export", true);
	}

	int direction_fd = open( (pin_path + "/direction").c_str(), O_WRONLY);
	sAssertion(direction_fd >= 0, "(CGPIO::CGPIO) Failed to open /direction", true);
	ret = write(direction_fd, "out", string("out").size());
	sAssertion(ret == static_cast<int>(string("out").size()),
			   "(CGPIO::CPIO) Failed to write to /direction", true);
	ret = close(direction_fd);
	sAssertion(ret >= 0, "(CGPIO::CGPIO) Failed to close Direction-FD", true);

	mValueFD = open((pin_path + "/value").c_str(), O_WRONLY);
	sAssertion(mValueFD >= 0, ("(CGPIO::CGPIO) Failed to open " + pin_path).c_str(), true);
}
bool CGPIO::setLow()
{
	int ret = write(mValueFD, "0", 1);
	return ret == 1;
}
bool CGPIO::setHigh()
{
	int ret = write(mValueFD, "1", 1);
	return ret == 1;
}

CGPIO::~CGPIO() {
	close(mValueFD);
}
