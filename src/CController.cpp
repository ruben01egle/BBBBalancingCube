#include "CController.h"

#include <algorithm>

CController::CController(std::array<float, 3> pK, float pMaxTM): mK(pK), mMaxTM(pMaxTM)
{
    mTM = 0;
}

float CController::update(CStateVectorData pStateData)
{
    mTM = std::clamp(mK[0]*pStateData.mPhi_C
                    + mK[1]*pStateData.mDotPhi
                    + mK[2]*pStateData.mDotPsi, -mMaxTM, mMaxTM);
    return mTM;
}
