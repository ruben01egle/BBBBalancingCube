/**
 * @file CBinarySemaphore.cpp
 * @author Michael Meindl
 * @date 24.9.2016
 * @brief Method definitions for a binary semaphore, which is simulated using a mutex.
 */
#include "CBinarySemaphore.h"
#include "CErrorReporter.hpp"

CBinarySemaphore::CBinarySemaphore() : mCounter(1)
{
}

bool CBinarySemaphore::init(bool pIsFull, bool pIsProcessShared)
{
	int32_t retVal;
	pthread_mutexattr_t mutexAttr;
	retVal = pthread_mutexattr_init(&mutexAttr);
	if (retVal != 0) {
        REPORT_ERROR("Failed to init Mutex-Attribute, errno: ", retVal);
		return false;
    }

	retVal = pthread_mutexattr_settype(&mutexAttr, PTHREAD_MUTEX_ERRORCHECK);
	if (retVal != 0) {
        REPORT_ERROR("Failed to set Mutex-Type, errno: ", retVal);
		return false;
    }

	retVal = pthread_mutex_init(&mMutex, &mutexAttr);
	if (retVal != 0) {
        REPORT_ERROR("Failed to init Mutex, errno: ", retVal);
		return false;
    }

	retVal = pthread_mutexattr_destroy(&mutexAttr);
	if (retVal != 0) {
        REPORT_ERROR("Failed to destroy Mutex-Attribute, errno: ", retVal);
		return false;
    }

	pthread_condattr_t conditionAttr;
	retVal = pthread_condattr_init(&conditionAttr);
	if (retVal != 0) {
        REPORT_ERROR("Failed to init Condition-Attribute, errno: ", retVal);
		return false;
    }

	retVal = pthread_condattr_setpshared(&conditionAttr,
										 pIsProcessShared ? PTHREAD_PROCESS_SHARED : PTHREAD_PROCESS_PRIVATE);
	if (retVal != 0) {
        REPORT_ERROR("Failed to set Condition-Attribute, errno: ", retVal);
		return false;
    }

	pthread_cond_init(&mCondition, &conditionAttr);
	pthread_condattr_destroy(&conditionAttr);


	if(false == pIsFull)
	{
		mCounter = 0;
	}
	return true;
}

CBinarySemaphore::~CBinarySemaphore()
{
	pthread_mutex_destroy(&mMutex);
}

bool CBinarySemaphore::take(bool waitForever)
{
	bool result = true;
	pthread_mutex_lock(&mMutex);
	if(1 == mCounter)
	{
		mCounter = 0;
	}
	else if(false == waitForever)
	{
		result = false;
	}
	else
	{
		while(0 == mCounter)
		{
			pthread_cond_wait(&mCondition, &mMutex);
		}
		mCounter = 0;
	}
	pthread_mutex_unlock(&mMutex);
	return result;
}

void CBinarySemaphore::give()
{
	pthread_mutex_lock(&mMutex);
	mCounter = 1;
	pthread_mutex_unlock(&mMutex);
	pthread_cond_signal(&mCondition);
}
