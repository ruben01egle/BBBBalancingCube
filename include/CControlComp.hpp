#pragma once

#include <cstdint>
#include "IRunnable.hpp"
#include "CLoopTimer.hpp"
#include "CHardware.hpp"
#include "CStateVectorData.hpp"
#include "CIMUData.hpp"
#include "CStateEstimation.hpp"
#include "CController.hpp"

class CControlComp : public IRunnable 
{
public:
    CControlComp();
    void init() override;
    void run() override;

private:
    bool mInitSuccesfull;
    CLoopTimer mTimer;
    CStateVectorData mStateData;
    CIMUData mImu1Data;
    CIMUData mImu2Data;
    uint16_t mADCVal;
    CHardware mHardware;
    CStateEstimation mStateEstimation;
    CController mController;
};
