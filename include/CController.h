#ifndef CCONTROLLER_H
#define CCONTROLLER_H

#include <array>

#include "CStateVectorData.h"

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

#endif