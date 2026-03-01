#ifndef CCOMMCOMP_H
#define CCOMMCOMP_H

#include "IRunnable.h"
#include "CServer.h"

class CCommComp : public IRunnable
{
public:
	CCommComp();
	~CCommComp();
	void init() override;
	void run() override;

private:
	CContent mData;
	CServer mServer;
	bool mClientConnected;
};

#endif
