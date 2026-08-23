#pragma once

#include <cstdint>
#include "IRunnable.hpp"
#include "CLoopTimer.hpp"
#include "CHardware.hpp"
#include "CStateVectorData.hpp"
#include "CIMUData.hpp"
#include "CIMUDataCalibrated.hpp"
#include "CCalibration.hpp"
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
    CIMUDataCalibrated mImu1CalibData;
    CIMUDataCalibrated mImu2CalibData;
    uint16_t mADCVal;
    CHardware mHardware;
    CCalibration mCalibration;
    CStateEstimation mStateEstimation;
    CController mController;
};
