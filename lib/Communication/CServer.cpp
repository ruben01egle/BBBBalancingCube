#include <CServer.h>

#include <sys/socket.h>
#include <unistd.h>
#include <strings.h>
#include <cerrno>

#include "CubeConstants.hpp"
#include "CErrorReporter.hpp"


bool CServer::transmitMessage(CContent& content)
{
	bool success = false;
	int32_t retVal = -1;
	size_t writtenByte = 0;
	uint8_t* buffer = reinterpret_cast<uint8_t*>(&content);
	do
	{
		retVal = send(mConnectedSocketFD, (buffer+writtenByte), (sizeof(content) - writtenByte), MSG_NOSIGNAL);
		if((retVal < 0) && (errno == EPIPE))
		{
			REPORT_ERROR("Connection was terminated");
			success = false;
			break;
		}
		else if(retVal < 0)
		{
			REPORT_ERROR_ERRNO("Failed to send the message");
		}
		success = true;
		writtenByte += retVal;
	}while(writtenByte < sizeof(content));
	return success;
}

bool CServer::init()
{
	mSocketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (mSocketFD < 0) {
		REPORT_ERROR_ERRNO("Failed to open socket");
	}

	int enable = 1;
	setsockopt(mSocketFD, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

	//Bind the socket to an address
	struct sockaddr_in server_addr;
	bzero(reinterpret_cast<int8_t*>(&server_addr), sizeof(server_addr));
	server_addr.sin_family		= AF_INET;
	server_addr.sin_port		= htons(Cube::TCP_PORT);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	int32_t retVal = bind(mSocketFD,
						reinterpret_cast<struct sockaddr*>(&server_addr),
						sizeof(server_addr));
	if (retVal < 0) {
		REPORT_ERROR_ERRNO("Failed to bind the socket");
		return false;
	}

	retVal = listen(mSocketFD, 1);
	if (retVal < 0) {
		REPORT_ERROR_ERRNO("Failed to listen()");
		return false;
	}
	return true;
}

bool CServer::waitForClient(size_t pTimeout)
{
	struct timeval timeout;      
	timeout.tv_sec = pTimeout;
	timeout.tv_usec = 0;
	setsockopt(mSocketFD, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	mConnectedSocketFD = accept(mSocketFD,
					   	   	    reinterpret_cast<struct sockaddr*>(&mClientAddr),
								&mClientLen);
	if (mConnectedSocketFD < 0) {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
            REPORT_ERROR_ERRNO("Failed to accept the client connection");
        }
		return false;
	}
	return true;
}

CServer::CServer() : mSocketFD(-1),
					 mConnectedSocketFD(-1),
					 mClientLen(0U)
{

}

CServer::~CServer()
{
	int32_t retVal = shutdown(mConnectedSocketFD, SHUT_RDWR);
	if (retVal < 0) {
		REPORT_ERROR_ERRNO("Failed to shutdown socket");
	}

	retVal = close(mConnectedSocketFD);
	if (retVal < 0) {
		REPORT_ERROR_ERRNO("Failed to close connected socket");
	}

	retVal = close(mSocketFD);
	if (retVal < 0) {
		REPORT_ERROR_ERRNO("Failed to close socket");
	}
}
