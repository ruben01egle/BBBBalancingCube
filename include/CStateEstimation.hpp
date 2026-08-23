#pragma once

#include <vector>
#include <array>

#include "CStateVectorData.hpp"
#include "CIMUData.hpp"
#include "CCalibration.hpp"

class CStateEstimation 
{
public:
    CStateEstimation(double pAlpha, double pTa, CCalibration pCalibration);

    void estimateState(const uint16_t& pADCVal, const CIMUData& pImu1Data, const CIMUData& pImu2Data, CStateVectorData& pStateData);

private:
    double mAlpha;
    double mTa;
    CCalibration mCalibration;

private:
    static constexpr double g = 9.81; 
};
