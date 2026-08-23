/**
 * @author	Michael Meindl
 * @date	8.12.2016
 * @brief	Structure to hold all the content of the CContainer.
 */
#pragma once

#include <cstdint>
#include "CStateVectorData.hpp"
#include "CIMUData.hpp"
#include "CIMUDataCalibrated.hpp"

class CContent
{
public:
	int64_t mTimeUs;
	CIMUData mSensor1Data;
	CIMUData mSensor2Data;
	CIMUDataCalibrated mSensor1DataCalib;
	CIMUDataCalibrated mSensor2DataCalib;
	CStateVectorData mStateData;
	float mMotorTorque;
	uint16_t mADCValue;
	uint16_t mPadding;
};

static_assert(sizeof(CContent) == 88, "CContent size mismatch! Check alignment.");
