#ifndef CTCPSERVER_HPP
#define CTCPSERVER_HPP

#include <cstdint>
#include "CContent.h"
#include <netinet/in.h>

class CTCPServer
{
public:
	CTCPServer();
	~CTCPServer();
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
