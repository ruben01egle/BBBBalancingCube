#pragma once

#include <cstdint>
#include <ctime>

class CLoopTimer
{
public:
    CLoopTimer(double pTa);
    void start();
    void sleepUntilNext();
    int64_t getCurrentMicros();
private:
    struct timespec mStartTime;
    struct timespec mWakeTime;
    int64_t mTaNS;
};
