/*
 * @file CBinarySemaphore.h
 * @author Michael Meindl
 * @date 18.9.2016
 * @brief Class definition for a binary semaphore, which is sumulated using a mutex.
 */
#pragma once

#include <cstdint>
#include <pthread.h>

class CBinarySemaphore
{
public:
	bool init(bool pIsFull, bool pIsProcessShared);
	bool take(bool waitForever);
	void give();
public:
	CBinarySemaphore();
	~CBinarySemaphore();
private:
	pthread_mutex_t mMutex;
	pthread_cond_t mCondition;
	int32_t mCounter;
};
