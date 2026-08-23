#include "CCalibration.hpp"

CCalibration::CCalibration(CCalibrationData pData):
            mData(pData)
{
}

void CCalibration::calibrate(const CIMUData& pImu1Data, const CIMUData& pImu2Data, const uint16_t& pADCVal,
                              CIMUDataCalibrated& pImu1Calib, CIMUDataCalibrated& pImu2Calib, float& pDotPsi) const
{
    pImu1Calib.mDDotX  = mData.mImu1AccelScaleX * (static_cast<float>(pImu1Data.mDDotX) - mData.mImu1AccelOffsetX);
    pImu1Calib.mDDotY  = mData.mImu1AccelScaleY * (static_cast<float>(pImu1Data.mDDotY) - mData.mImu1AccelOffsetY);
    pImu1Calib.mDotPhi = mData.mImu1GyroScale * (static_cast<float>(pImu1Data.mDotPhi) - mData.mImu1GyroOffset);

    pImu2Calib.mDDotX  = mData.mImu2AccelScaleX * (static_cast<float>(pImu2Data.mDDotX) - mData.mImu2AccelOffsetX);
    pImu2Calib.mDDotY  = mData.mImu2AccelScaleY * (static_cast<float>(pImu2Data.mDDotY) - mData.mImu2AccelOffsetY);
    pImu2Calib.mDotPhi = mData.mImu2GyroScale * (static_cast<float>(pImu2Data.mDotPhi) - mData.mImu2GyroOffset);

    pDotPsi = mData.mADCScale * (static_cast<float>(pADCVal) - mData.mADCOffset);
}
