/**
 * @file IRunnable.h
 * @author Michael Meindl
 * @date 24.9.2016
 * @brief File containing an interface for implement tasks which are run in a separate thread.
 */
#ifndef IRUNNABLE_H
#define IRUNNABLE_H

class IRunnable
{
public:
	virtual void run() = 0;		/// Pure virtual method to run the task in a thread
	virtual void init() = 0;	/// Pure virtual method to initialize the task
public:
	virtual ~IRunnable() {};
};

#endif
