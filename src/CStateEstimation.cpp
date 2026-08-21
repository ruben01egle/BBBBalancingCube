#include "CStateEstimation.h"

#include <algorithm>
#include <cmath>

#include "CubeConstants.hpp"
using namespace Cube;

CStateEstimation::CStateEstimation(double pAlpha, double pTa, CCalibration pCalibration):
            mAlpha(pAlpha),
            mTa(pTa),
            mCalibration(pCalibration)
{
}

void CStateEstimation::estimateState(const uint16_t &pADCVal, const CIMUData &pImu1Data, const CIMUData &pImu2Data, CStateVectorData &pStateData)
{
    float dDotXImu1 = mCalibration.mImu1AccelScaleX*static_cast<float>(pImu1Data.mDDotX) + mCalibration.mImu1AccelOffsetX;
    float dDotYImu1 = mCalibration.mImu1AccelScaleY*static_cast<float>(pImu1Data.mDDotY) + mCalibration.mImu1AccelOffsetY;
    float dotPhiImu1 = mCalibration.mImu1GyroScale*static_cast<float>(pImu1Data.mDotPhi) + mCalibration.mImu1GyroOffset;

    float dDotXImu2 = mCalibration.mImu2AccelScaleX*static_cast<float>(pImu2Data.mDDotX) + mCalibration.mImu2AccelOffsetX;
    float dDotYImu2 = mCalibration.mImu2AccelScaleY*static_cast<float>(pImu2Data.mDDotY) + mCalibration.mImu2AccelOffsetY;
    float dotPhiImu2 = mCalibration.mImu2GyroScale*static_cast<float>(pImu2Data.mDotPhi) + mCalibration.mImu2GyroOffset;

    pStateData.mDotPhi = (dotPhiImu1 + dotPhiImu2)/2;
    pStateData.mPhi_A = -atan2(dDotXImu1 - IMU_ALPHA*dDotXImu2, dDotYImu1 - IMU_ALPHA*dDotYImu2);
    pStateData.mPhi_G += pStateData.mDotPhi*mTa;
    pStateData.mPhi_C = ALPHA*(pStateData.mPhi_C + mTa*pStateData.mDotPhi) + (1-ALPHA)*pStateData.mPhi_A;
    pStateData.mDotPsi = mCalibration.mADCScale*static_cast<float>(pADCVal) + mCalibration.mADCOffset;
}