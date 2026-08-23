#pragma once

#include <array>

#include "CStateVectorData.hpp"

class CController
{
public:
    CController(std::array<float, 3> pK, float pMaxTM);
    float update(CStateVectorData pStateData);
private:
    std::array<float, 3> mK;
    float mMaxTM;
    float mTM;
};
