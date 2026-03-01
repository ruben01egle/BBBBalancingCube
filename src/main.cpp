#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <string>

#include "CContainer.h"
#include "CCommComp.h"
#include "CControlComp.h"
#include "CThread.h"

using namespace std;

CContainer myContainer;
atomic<bool> runvar{true};

int main(){
	cout << "main running" << endl;

	CCommComp Comm;
	CThread CommThread(&Comm, CThread::PRIORITY_ABOVE_NORM);
	CommThread.start();

	for (uint8_t i = 0; i < 3; ++i) {
		cout << "Starting ControlComp in " << int(3-i) << endl;
		this_thread::sleep_for(chrono::seconds(1));
	}

	CControlComp Control;
	CThread ControlThread(&Control, CThread::EPriority::PRIORITY_REALTIME);
	ControlThread.start();

	cout << "Type a random character to kill program:" << endl;
	
	char in;
    cin >> in;
	runvar.store(false);

	ControlThread.join();
	myContainer.signalReader();
	CommThread.join();

	cout << "main end" << endl;
	return 0;
}