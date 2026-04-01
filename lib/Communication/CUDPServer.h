#ifndef CUDPSERVER_H
#define CUDPSERVER_H

#include <cstdint>
#include "CContent.h"
#include <netinet/in.h>

class CUDPServer
{
public:
	CUDPServer();
	~CUDPServer();
	bool init();
	
	bool transmitMessage(CContent& pContent);

private:
	int mSocket;
	struct sockaddr_in mClientAddr;
	uint32_t mIntCounter;
};

#endif
