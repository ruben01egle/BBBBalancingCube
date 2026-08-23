#pragma once

#include <cstdint>
#include "CContent.hpp"
#include <netinet/in.h>

class CServer
{
public:
	CServer();
	~CServer();
	bool init();
	
	bool waitForClient(size_t pTimeout);
	bool transmitMessage(CContent& pContent);

private:
	int mSocketFD;
	int mConnectedSocketFD;
	socklen_t mClientLen;
	struct sockaddr_in mClientAddr;
};
