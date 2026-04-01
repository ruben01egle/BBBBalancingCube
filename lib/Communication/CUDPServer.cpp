#include "CUDPServer.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

#include "CubeConstants.h"
#include "CErrorReporter.h"

CUDPServer::CUDPServer(): mSocket(-1)
{
	memset(&mClientAddr, 0, sizeof(mClientAddr));
	mIntCounter = 0;
}

CUDPServer::~CUDPServer()
{
	if (mSocket >= 0) {
		close(mSocket);
	}
}

bool CUDPServer::init()
{
	mSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (mSocket < 0) {
		REPORT_ERROR_ERRNO("Socket failed to open");
		return false;
	}

	sockaddr_in localAddr{};
	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons(Cube::UDP_PORT);
	localAddr.sin_addr.s_addr = INADDR_ANY;
	if (bind(mSocket, (sockaddr*)&localAddr, sizeof(localAddr)) < 0) {
		REPORT_ERROR_ERRNO("Failed to bind Socket");
		return false;
	}

	mClientAddr.sin_family = AF_INET;
	mClientAddr.sin_port = htons(Cube::UDP_CLIENT_PORT);
	if (inet_pton(AF_INET, Cube::UDP_CLIENT_IP, &mClientAddr.sin_addr) <= 0) {
        REPORT_ERROR_ERRNO("Failed to register client");
        return false; 
    }

	return true;
}

bool CUDPServer::transmitMessage(CContent &pContent)
{
	if (mIntCounter == Cube::UDP_MSG_INT) {
		mIntCounter = 0;
		ssize_t sent = sendto(mSocket, &pContent, sizeof(CContent), 0,
	                      (sockaddr*)&mClientAddr, sizeof(mClientAddr));
		return sent == (ssize_t)sizeof(CContent);
	}
	else {
		mIntCounter++;
	}
	return true;
}
