#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <string>

#include "CContainer.hpp"
#include "CCommComp.hpp"
#include "CControlComp.hpp"
#include "CThread.hpp"
#include "CErrorReporter.hpp"

using namespace std;

CContainer myContainer;
atomic<bool> runvar{true};

int main(){
	cout << "main running" << endl;

	CCommComp Comm;
	CThread CommThread(&Comm, CThread::PRIORITY_ABOVE_NORM);
	if (!CommThread.start()) {
		REPORT_ERROR("main: failed to start CommThread");
	}

	for (uint8_t i = 0; i < 3; ++i) {
		cout << "Starting ControlComp in " << int(3-i) << endl;
		this_thread::sleep_for(chrono::seconds(1));
	}

	CControlComp Control;
	CThread ControlThread(&Control, CThread::EPriority::PRIORITY_REALTIME);
	if (!ControlThread.start()) {
		REPORT_ERROR("main: failed to start ControlThread");
	}

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