#include <unistd.h>
#include <iostream>
#include <atomic>
using namespace std;

#include "CCommComp.h"
#include "CContainer.h"

extern CContainer myContainer;
extern atomic<bool> runvar;

CCommComp::CCommComp(){
	mTCPClientConnected = false;
};

CCommComp::~CCommComp(){

};

void CCommComp::init()
{
	cout << "Comm init " << endl;
	mTCPServer.init();
	mUDPServer.init();
	cout << "Comm: wait for Client connected" << endl;
	size_t timeout = 1;
	size_t timeoutCounter = 0;
	size_t timeoutCounterMax = 30;

	while (runvar.load()) {
		if (timeoutCounter > timeoutCounterMax) {
			cout << "No client connected, end TCPServer" << endl;
			break;
		}
		if (mTCPServer.waitForClient(timeout)){
			mTCPClientConnected = true;
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
	cout << " CCommThread running " << endl;
	while(runvar.load())
	{
		myContainer.getContent(true, mData);
		if (!runvar.load()) break;
		if (mTCPClientConnected){
			if (!mTCPServer.transmitMessage(mData)){
				cout << "CCommThread: Client disconncted or error while transmitting message" << endl;
				mTCPClientConnected = false;
			}
		}
		mUDPServer.transmitMessage(mData);
	}
	cout<<"Comm End"<<endl;
};
