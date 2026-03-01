#ifndef ASSERTION_H
#define ASSERTION_H
#include <string>

void sAssertion(bool assert,
		        const std::string& errorMessage,
				bool exitOnError);

#endif
