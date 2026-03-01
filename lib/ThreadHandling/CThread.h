/**
 * @file	CThread.h
 * @author	Michael Meindl
 * @date	28.9.2016
 * @brief	Class definition for a thread-carrier.
 */
#ifndef CTHREAD_H
#define CTHREAD_H
#include "IRunnable.h"
#include <pthread.h>

class CThread
{
public:
	  enum EPriority
	     {
	        PRIORITY_IDLE 			= 4,
	        PRIORITY_ABOVE_IDLE 	= 5,
	        PRIORITY_BELOW_LOW 		= 6,
	        PRIORITY_LOW 			= 7,
	        PRIORITY_ABOVE_LOW 		= 8 ,
			PRIORITY_BELOW_NORM 	= 9,
	        PRIORITY_NORM 			= 10,
	        PRIORITY_ABOVE_NORM 	= 11,
	        PRIORITY_BELOW_HIGH 	= 12,
	        PRIORITY_HIGH 			= 13,
	        PRIORITY_ABOVE_HIGH 	= 14,
	        PRIORITY_BELOW_CRITICAL = 15,
	        PRIORITY_CRITICAL 		= 20,
	        PRIORITY_ABOVE_CRITICAL = 25,
	        PRIORITY_BELOW_REALTIME = 50,
	        PRIORITY_REALTIME 		= 63
	     };
public:
	void start();
	void join();
private:
	friend void* threadProc(void*);
	void run();
public:
	CThread(IRunnable* runnablePtr,
			EPriority prio);
private:
	pthread_t mThreadID;
	IRunnable* mRunablePtr;
	EPriority mPrioBase;
};

#endif
