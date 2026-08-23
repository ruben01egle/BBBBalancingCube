#pragma once

#include "IRunnable.hpp"
#include "CContent.hpp"
#include "CServer.hpp"

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
