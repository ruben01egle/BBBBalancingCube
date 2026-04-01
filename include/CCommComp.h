#ifndef CCOMMCOMP_H
#define CCOMMCOMP_H

#include "IRunnable.h"
#include "CTCPServer.h"
#include "CUDPServer.h"

class CCommComp : public IRunnable
{
public:
	CCommComp();
	~CCommComp();
	void init() override;
	void run() override;

private:
	CContent mData;
	CTCPServer mTCPServer;
	CUDPServer mUDPServer;
	bool mTCPClientConnected;
};

#endif
