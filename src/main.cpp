#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <string>
#include <cstdlib>

#include "CContainer.hpp"
#include "CCommComp.hpp"
#include "CControlComp.hpp"
#include "CThread.hpp"
#include "CErrorReporter.hpp"
#include "CCubeConfigLoader.hpp"
#include "CCalibrationData.hpp"

using namespace std;

CContainer myContainer;
atomic<bool> runvar{true};

int main(int argc, char** argv){
	cout << "main running" << endl;

	string configPath;
	if (argc > 1) {
		configPath = argv[1];
	} else if (const char* envPath = getenv("CUBE_CONFIG_PATH")) {
		configPath = envPath;
	} else {
		configPath = CCubeConfigLoader::defaultConfigPath();
	}

	CCalibrationData calibration;
	int cubeId = -1;
	if (!CCubeConfigLoader::loadForThisHost(configPath, calibration, cubeId)) {
		cerr << "Failed to load cube calibration from '" << configPath << "'. Refusing to start." << endl;
		return 1;
	}
	cout << "Loaded calibration for cube " << cubeId << " from '" << configPath << "'" << endl;

	CCommComp Comm;
	CThread CommThread(&Comm, CThread::PRIORITY_ABOVE_NORM);
	if (!CommThread.start()) {
		REPORT_ERROR("main: failed to start CommThread");
	}

	for (uint8_t i = 0; i < 3; ++i) {
		cout << "Starting ControlComp in " << int(3-i) << endl;
		this_thread::sleep_for(chrono::seconds(1));
	}

	CControlComp Control(calibration);
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