#pragma once

#include <cstdint>
#include "IRunnable.hpp"
#include "CLoopTimer.hpp"
#include "CHardware.hpp"
#include "CStateVectorData.hpp"
#include "CIMUData.hpp"
#include "CIMUDataCalibrated.hpp"
#include "CCalibrationData.hpp"
#include "CCalibration.hpp"
#include "CStateEstimation.hpp"
#include "CController.hpp"

class CControlComp : public IRunnable
{
public:
    explicit CControlComp(const CCalibrationData& pCalibrationData, bool pCalibrateMode = false);
    void init() override;
    void run() override;

private:
    bool mInitSuccesfull;
    bool mCalibrateMode;
    CLoopTimer mTimer;
    CStateVectorData mStateData;
    CIMUData mImu1Data;
    CIMUData mImu2Data;
    CIMUDataCalibrated mImu1CalibData;
    CIMUDataCalibrated mImu2CalibData;
    uint16_t mADCVal;
    CHardware mHardware;
    CCalibrationData mCalibrationData;
    CCalibration mCalibration;
    CStateEstimation mStateEstimation;
    CController mController;
};
