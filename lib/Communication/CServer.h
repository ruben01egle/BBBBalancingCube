#ifndef CSERVER_HPP
#define CSERVER_HPP

#include <cstdint>
#include "CContent.h"
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
	bool mConnected;
};

#endif
