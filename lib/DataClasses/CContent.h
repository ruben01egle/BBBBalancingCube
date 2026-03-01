/**
 * @author	Michael Meindl
 * @date	8.12.2016
 * @brief	Structure to hold all the content of the CContainer.
 */
#ifndef CCONTENT_H
#define CCONTENT_H
#include "CStateVectorData.h"
#include "CIMUData.h"

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

#endif
