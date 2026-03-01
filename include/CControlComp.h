#ifndef CCONTROLCOMP_H
#define CCONTROLCOMP_H

#include "IRunnable.h"
#include "CLoopTimer.h"
#include "CBBBHardware.h"
#include "CStateVectorData.h"
#include "CIMUData.h"
#include "CStateEstimation.h"
#include "CController.h"

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
    CBBBHardware mHardware;
    CStateEstimation mStateEstimation;
    CController mController;
};

#endif