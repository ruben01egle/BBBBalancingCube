#include "Assertion.h"
#include <iostream>
#include <cstdlib>
#include <errno.h>

extern int errno;

void sAssertion(bool assert, const std::string& errMsg, bool exitOnError)
{
	if(false == assert)
	{
		std::cerr << errMsg << ",errno: " << errno << std::endl;
		if(true == exitOnError)
		{
			exit(-1);
		}
	}
}
