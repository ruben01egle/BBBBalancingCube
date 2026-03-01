#include <unistd.h>
#include <iostream>
#include <atomic>
using namespace std;

#include "CCommComp.h"
#include "CContainer.h"

extern CContainer myContainer;
extern atomic<bool> runvar;

CCommComp::CCommComp(){
	mClientConnected = false;
};

CCommComp::~CCommComp(){

};

void CCommComp::init()
{
	cout << "Comm init " << endl;
	mServer.init();
	cout << "Comm: wait for Client connected" << endl;
	size_t timeout = 1;
	size_t timeoutCounter = 0;
	size_t timeoutCounterMax = 30;

	while (runvar.load()) {
		if (timeoutCounter > timeoutCounterMax) {
			cout << "No client connected, end CommThread" << endl;
			break;
		}
		if (mServer.waitForClient(timeout)){
			mClientConnected = true;
			cout << "Client connected" << endl;
			break;
		}
		else{
			timeoutCounter++;
		}
	}
	
};

void CCommComp::run()
{
	if (!mClientConnected){
		cout << "Comm end" << endl;
		return;
	}
	
	cout << " CCommThread running " << endl;
	while(runvar)
	{
		myContainer.getContent(true, mData);
		if (!runvar) break;
		if (!mServer.transmitMessage(mData)){
			cout << "CCommThread: Client disconncted or error while transmitting message" << endl;
			break;
		}
	}
	cout<<"Comm End"<<endl;
};
