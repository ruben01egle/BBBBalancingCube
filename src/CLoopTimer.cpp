#include "CLoopTimer.h"

#include "CErrorReporter.hpp"

CLoopTimer::CLoopTimer(double pTa)
{
    mTaNS = static_cast<double>(pTa*1'000'000'000);
}

void CLoopTimer::start()
{
    clock_gettime(CLOCK_MONOTONIC, &mStartTime);
    mWakeTime = mStartTime;
}

void CLoopTimer::sleepUntilNext()
{
    mWakeTime.tv_nsec += mTaNS;
    if (mWakeTime.tv_nsec >= 1'000'000'000L) {
        mWakeTime.tv_sec += mWakeTime.tv_nsec / 1'000'000'000L;
        mWakeTime.tv_nsec %= 1'000'000'000L;
    }
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    int64_t overrunUs = (now.tv_sec - mWakeTime.tv_sec) * 1'000'000L +
                        (now.tv_nsec - mWakeTime.tv_nsec) / 1'000L;
    if (overrunUs > 0) {
        REPORT_ERROR("Overrun: ", overrunUs, " µs late");
    }
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &mWakeTime, nullptr);
}

int64_t CLoopTimer::getCurrentMicros()
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (now.tv_sec - mStartTime.tv_sec) * 1'000'000L + (now.tv_nsec - mStartTime.tv_nsec) / 1000L;
}