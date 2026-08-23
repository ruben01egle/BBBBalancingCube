/**
 * @author	Michael Meindl
 * @date	8.12.2016
 * @brief	Structure to hold all the content of the CContainer.
 */
#pragma once

#include <cstdint>
#include "CStateVectorData.hpp"
#include "CIMUData.hpp"

class CContent
{
public:
	int64_t mTimeUs;
	CIMUData mSensor1Data;
	CIMUData mSensor2Data;
	CStateVectorData mStateData;
	float mMotorTorque;
	uint16_t mADCValue;
	uint16_t mPadding;
};
