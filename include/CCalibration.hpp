#pragma once

#include <cstdint>
#include "CIMUData.hpp"
#include "CIMUDataCalibrated.hpp"
#include "CCalibrationData.hpp"

class CCalibration
{
public:
    CCalibration(CCalibrationData pData);

    void calibrate(const CIMUData& pImu1Data, const CIMUData& pImu2Data, const uint16_t& pADCVal,
                    CIMUDataCalibrated& pImu1Calib, CIMUDataCalibrated& pImu2Calib, float& pDotPsi) const;

private:
    CCalibrationData mData;
};
