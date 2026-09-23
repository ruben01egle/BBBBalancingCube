#pragma once

#include <string>

#include "IRunnable.hpp"

// Used instead of CCommComp for --auto-calibrate: collects the raw IMU data from the
// container until runvar becomes false, computes the IMU offsets and scales from it
// (same maths as tools/calib.py) and writes them into this cube's entry of the config file.
class CCalibComp : public IRunnable
{
public:
	explicit CCalibComp(const std::string& pConfigPath);
	void init() override;
	void run() override;

private:
	std::string mConfigPath;
};
