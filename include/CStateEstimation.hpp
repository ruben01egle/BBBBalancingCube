#pragma once

#include <vector>
#include <array>

#include "CStateVectorData.hpp"
#include "CIMUDataCalibrated.hpp"

class CStateEstimation
{
public:
    CStateEstimation(double pAlpha, double pTa);

    void estimateState(const CIMUDataCalibrated& pImu1CalibData, const CIMUDataCalibrated& pImu2CalibData, CStateVectorData& pStateData);

private:
    double mAlpha;
    double mTa;

private:
    static constexpr double g = 9.81; 
};
