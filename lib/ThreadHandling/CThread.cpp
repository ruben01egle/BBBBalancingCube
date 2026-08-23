/**
 * @file	CTread.cpp
 * @author	Michael Meindl
 * @date	28.9.2016
 * @brief	Method definitions for CThread.
 */
#include "CThread.hpp"
#include <cstdint>
#include <unistd.h>

#include "CErrorReporter.hpp"

using namespace std;

void* threadProc(void* thisPtr)
{
	CThread* thisThreadPtr = reinterpret_cast<CThread*>(thisPtr);
	thisThreadPtr->run();
	return NULL;
}
CThread::CThread(IRunnable* runnablePtr,
				 EPriority prio) : mThreadID(-1),
								   mRunablePtr(runnablePtr),
								   mPrioBase(prio)
{

}
bool CThread::start()
{
	struct sched_param threadparam;
	int max = sched_get_priority_max(SCHED_RR);
	int min = sched_get_priority_min(SCHED_RR);
	int32_t realPrio = min + mPrioBase * (max - min) / (PRIORITY_REALTIME + 1);
	if (realPrio < min) realPrio = min;
	if (realPrio > max) realPrio = max;

	//Configure the scheduling policy as Round-Robin
	pthread_attr_t attribute;
	pthread_attr_init(&attribute);
	pthread_attr_setinheritsched(&attribute, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&attribute, SCHED_RR);
	//Set the priority of the new thread
	pthread_attr_getschedparam(&attribute, &threadparam);
	threadparam.sched_priority = realPrio;
	pthread_attr_setschedparam(&attribute, &threadparam);

	int ret = pthread_create(&mThreadID, &attribute, threadProc, this);

	//Cleanup
	pthread_attr_destroy(&attribute);

	if(ret != 0)
	{
		REPORT_ERROR("pthread_create() failed! ret: ", ret);
		return false;
	}
	mStarted = true;
	return true;
}
void CThread::run()
{
	mRunablePtr->init();
	mRunablePtr->run();
}
void CThread::join()
{
	if (mStarted) {
		pthread_join(mThreadID, NULL);
	}
}

