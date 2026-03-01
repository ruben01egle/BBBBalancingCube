#include "CControlComp.h"

#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include <algorithm>
#include <atomic>

#include "CubeConstants.h"
#include "CContainer.h"

using namespace Cube;
using namespace std;

extern CContainer myContainer;
extern atomic<bool> runvar;

CControlComp::CControlComp():
            mTimer(T_A),
            mStateEstimation(ALPHA, T_A, CUBE_CONFIG),
            mController(K, MAX_TM)
{
    mInitSuccesfull = false;
}

void CControlComp::init()
{
    cout << "ControlComp init" << endl;
    if (!mHardware.enableMotor()){
        return;
    }
    int64_t currentMicros;
    int64_t lastPrintMicros = 0;
    int64_t sensorInitTime = 3;
    mTimer.start();
    do{
        if (!runvar) {
            return;
        }
        currentMicros = mTimer.getCurrentMicros();
        mHardware.fetchValues(mADCVal, mImu1Data, mImu2Data);
        mStateEstimation.estimateState(mADCVal, mImu1Data, mImu2Data, mStateData);
        if (currentMicros - lastPrintMicros >= 1'000'000) {
            int remainingSeconds = sensorInitTime - (currentMicros / 1'000'000);
            if (remainingSeconds < 0) remainingSeconds = 0;
            std::cout << "Sensor initialising, remaining seconds: " << remainingSeconds << std::endl;
            lastPrintMicros = currentMicros;
        }
        mTimer.sleepUntilNext();

    } while(currentMicros < sensorInitTime*1'000'000);

    mInitSuccesfull = true;
}

void CControlComp::run()
{
    cout << "ControlComp run" << endl;
    if (!mInitSuccesfull) {
        mHardware.disableMotor();
        cerr << "ControlComp init failed - exiting run" << endl;
        return;
    }

    int64_t currentMicros = 0;

    mTimer.start();
    while (runvar.load())
    {
        currentMicros = mTimer.getCurrentMicros();
        mHardware.fetchValues(mADCVal, mImu1Data, mImu2Data);
        mStateEstimation.estimateState(mADCVal, mImu1Data, mImu2Data, mStateData);

        if (abs(mStateData.mPhi_C) > 0.25){
            cout << "Cube out of range" << endl;
            runvar.store(false);
        }

        float TM = mController.update(mStateData);
        
        mHardware.setTorque(TM);

        myContainer.writeData(
            currentMicros,
            mADCVal,
            TM,
            mImu1Data,
            mImu2Data,
            mStateData
        );
        myContainer.signalReader();

        mTimer.sleepUntilNext();
    }
    mHardware.setTorque(0);
    mHardware.disableMotor();
    cout << "Control end" << endl;
}
