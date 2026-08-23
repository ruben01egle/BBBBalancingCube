#include "CStateEstimation.hpp"

#include <algorithm>
#include <cmath>

#include "CubeConstants.hpp"
using namespace Cube;

CStateEstimation::CStateEstimation(double pAlpha, double pTa):
            mAlpha(pAlpha),
            mTa(pTa)
{
}

void CStateEstimation::estimateState(const CIMUDataCalibrated &pImu1CalibData, const CIMUDataCalibrated &pImu2CalibData, CStateVectorData &pStateData)
{
    pStateData.mDotPhi = (pImu1CalibData.mDotPhi + pImu2CalibData.mDotPhi)/2;
    pStateData.mPhi_A = atan2(pImu1CalibData.mDDotX - IMU_ALPHA*pImu2CalibData.mDDotX, pImu1CalibData.mDDotY - IMU_ALPHA*pImu2CalibData.mDDotY);
    pStateData.mPhi_G += pStateData.mDotPhi*mTa;
    pStateData.mPhi_C = mAlpha*(pStateData.mPhi_C + mTa*pStateData.mDotPhi) + (1-mAlpha)*pStateData.mPhi_A;
}