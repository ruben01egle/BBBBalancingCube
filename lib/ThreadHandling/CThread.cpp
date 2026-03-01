/**
 * @file	CTread.cpp
 * @author	Michael Meindl
 * @date	28.9.2016
 * @brief	Method definitions for CThread.
 */
#include "CThread.h"
#include <cstdint>
#include <unistd.h>
#include <cstdlib>

#include "CErrorReporter.h"

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
void CThread::start()
{
	struct sched_param threadparam;
	int max = sched_get_priority_max(SCHED_RR);
	int min = sched_get_priority_min(SCHED_RR);
	int32_t realPrio = min + (max - min)/(PRIORITY_REALTIME + 1)*mPrioBase;

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

	//Temrminate the application in case the creation of the thread failed
	if(ret != 0)
	{
		REPORT_ERROR("pthread_create() failed! ret: ", ret, " exiting...");
		exit(-1);
	}
	//Cleanup
	pthread_attr_destroy(&attribute);
}
void CThread::run()
{
	mRunablePtr->init();
	mRunablePtr->run();
}
void CThread::join()
{
	pthread_join(mThreadID, NULL);
}

