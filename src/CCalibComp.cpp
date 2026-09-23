#include "CCalibComp.hpp"

#include <atomic>
#include <cmath>
#include <iostream>

#include "CubeConstants.hpp"
#include "CContainer.hpp"
#include "CContent.hpp"
#include "CCalibrationData.hpp"
#include "CCubeConfigLoader.hpp"
#include "CErrorReporter.hpp"

using namespace std;

extern CContainer myContainer;
extern atomic<bool> runvar;

namespace {

// Sanity limits on the raw-count data (cube must be still and in its zero position).
// Tuned for the +-4 g / 2000 dps setup, keep in sync with tools/calib.py.
constexpr size_t MIN_SAMPLES = 300;
constexpr double MAX_ACCEL_STD = 50.0;
constexpr double MAX_GYRO_STD = 10.0;
constexpr double MAX_ACCEL_OFFSET = 2000.0;
constexpr double MAX_GYRO_OFFSET = 500.0;

struct SChannel
{
	double sum = 0.0;
	double sumSq = 0.0;

	void add(double pValue) { sum += pValue; sumSq += pValue * pValue; }
	double mean(size_t pCount) const { return sum / pCount; }
	double stdDev(size_t pCount) const {
		const double m = mean(pCount);
		const double var = (sumSq - pCount * m * m) / (pCount - 1);
		return var > 0.0 ? sqrt(var) : 0.0;
	}
};

struct SImuChannels
{
	SChannel accelX;
	SChannel accelY;
	SChannel gyroZ;
};

bool checkImu(const char* pName, const SImuChannels& pCh, size_t pCount, float pAccelOffsetX,
              float pAccelOffsetY, float pGyroOffset)
{
	bool ok = true;
	const double stds[3] = {pCh.accelX.stdDev(pCount), pCh.accelY.stdDev(pCount), pCh.gyroZ.stdDev(pCount)};
	const double limits[3] = {MAX_ACCEL_STD, MAX_ACCEL_STD, MAX_GYRO_STD};
	const char* names[3] = {"accel X", "accel Y", "gyro Z"};
	for (int i = 0; i < 3; ++i) {
		if (stds[i] > limits[i]) {
			cerr << "Calibration rejected: " << pName << " " << names[i] << " std " << stds[i]
			     << " > " << limits[i] << " (cube moving?)" << endl;
			ok = false;
		}
	}
	if (abs(pAccelOffsetX) > MAX_ACCEL_OFFSET || abs(pAccelOffsetY) > MAX_ACCEL_OFFSET) {
		cerr << "Calibration rejected: " << pName << " accel offset exceeds +-" << MAX_ACCEL_OFFSET
		     << " (cube not in zero position?)" << endl;
		ok = false;
	}
	if (abs(pGyroOffset) > MAX_GYRO_OFFSET) {
		cerr << "Calibration rejected: " << pName << " gyro offset exceeds +-" << MAX_GYRO_OFFSET << endl;
		ok = false;
	}
	return ok;
}

void printImu(const char* pName, const char* pKey, float pOld, float pNew)
{
	cout << "  " << pName << pKey << ": " << pOld << " -> " << pNew << endl;
}

} // namespace

CCalibComp::CCalibComp(const std::string& pConfigPath) :
	mConfigPath(pConfigPath)
{
}

void CCalibComp::init()
{
	cout << "CalibComp: collecting raw IMU data until the control thread ends" << endl;
}

void CCalibComp::run()
{
	CContent data;
	SImuChannels imu1;
	SImuChannels imu2;
	size_t count = 0;

	while (runvar.load()) {
		myContainer.getContent(true, data);
		if (!runvar.load()) break;

		imu1.accelX.add(data.mSensor1Data.mA_x);
		imu1.accelY.add(data.mSensor1Data.mA_y);
		imu1.gyroZ.add(data.mSensor1Data.mW_z);
		imu2.accelX.add(data.mSensor2Data.mA_x);
		imu2.accelY.add(data.mSensor2Data.mA_y);
		imu2.gyroZ.add(data.mSensor2Data.mW_z);
		++count;
	}

	cout << "CalibComp: " << count << " samples collected" << endl;
	if (count < MIN_SAMPLES) {
		cerr << "Calibration rejected: only " << count << " samples (need >= " << MIN_SAMPLES << ")" << endl;
		return;
	}

	// Same maths as tools/calib.py: offsets are the mean raw values, y additionally has the
	// expected 1 g reading removed (cube in zero position).
	CCalibrationData cal{};
	cal.mImu1AccelScaleX  = Cube::ACCEL_SCALE_MS2;
	cal.mImu1AccelScaleY  = Cube::ACCEL_SCALE_MS2;
	cal.mImu1AccelOffsetX = imu1.accelX.mean(count);
	cal.mImu1AccelOffsetY = imu1.accelY.mean(count) - Cube::ACCEL_RAW_1G;
	cal.mImu1GyroScale    = Cube::GYRO_SCALE_RAD_S;
	cal.mImu1GyroOffset   = imu1.gyroZ.mean(count);
	cal.mImu2AccelScaleX  = Cube::ACCEL_SCALE_MS2;
	cal.mImu2AccelScaleY  = Cube::ACCEL_SCALE_MS2;
	cal.mImu2AccelOffsetX = imu2.accelX.mean(count);
	cal.mImu2AccelOffsetY = imu2.accelY.mean(count) - Cube::ACCEL_RAW_1G;
	cal.mImu2GyroScale    = Cube::GYRO_SCALE_RAD_S;
	cal.mImu2GyroOffset   = imu2.gyroZ.mean(count);

	bool ok = checkImu("IMU1", imu1, count, cal.mImu1AccelOffsetX, cal.mImu1AccelOffsetY, cal.mImu1GyroOffset);
	ok = checkImu("IMU2", imu2, count, cal.mImu2AccelOffsetX, cal.mImu2AccelOffsetY, cal.mImu2GyroOffset) && ok;
	if (!ok) {
		cerr << "Config not modified" << endl;
		return;
	}

	CCalibrationData old{};
	if (!CCubeConfigLoader::updateImuCalibrationForThisHost(mConfigPath, cal, &old)) {
		cerr << "Calibration computed but could not be written to '" << mConfigPath << "'" << endl;
		return;
	}

	cout << "Calibration written to '" << mConfigPath << "' (old -> new):" << endl;
	printImu("imu1", "AccelScaleX", old.mImu1AccelScaleX, cal.mImu1AccelScaleX);
	printImu("imu1", "AccelScaleY", old.mImu1AccelScaleY, cal.mImu1AccelScaleY);
	printImu("imu1", "AccelOffsetX", old.mImu1AccelOffsetX, cal.mImu1AccelOffsetX);
	printImu("imu1", "AccelOffsetY", old.mImu1AccelOffsetY, cal.mImu1AccelOffsetY);
	printImu("imu1", "GyroScale", old.mImu1GyroScale, cal.mImu1GyroScale);
	printImu("imu1", "GyroOffset", old.mImu1GyroOffset, cal.mImu1GyroOffset);
	printImu("imu2", "AccelScaleX", old.mImu2AccelScaleX, cal.mImu2AccelScaleX);
	printImu("imu2", "AccelScaleY", old.mImu2AccelScaleY, cal.mImu2AccelScaleY);
	printImu("imu2", "AccelOffsetX", old.mImu2AccelOffsetX, cal.mImu2AccelOffsetX);
	printImu("imu2", "AccelOffsetY", old.mImu2AccelOffsetY, cal.mImu2AccelOffsetY);
	printImu("imu2", "GyroScale", old.mImu2GyroScale, cal.mImu2GyroScale);
	printImu("imu2", "GyroOffset", old.mImu2GyroOffset, cal.mImu2GyroOffset);
}
