#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <string>
#include <cstdlib>
#include <memory>

#include "CContainer.hpp"
#include "CCommComp.hpp"
#include "CCalibComp.hpp"
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
	bool calibrateMode = false;
	bool autoCalibrateMode = false;
	for (int i = 1; i < argc; ++i) {
		string arg = argv[i];
		if (arg == "--calibrate") {
			calibrateMode = true;
		} else if (arg == "--auto-calibrate") {
			// computes and stores the calibration on the cube itself, implies --calibrate
			calibrateMode = true;
			autoCalibrateMode = true;
		} else if (configPath.empty()) {
			configPath = arg;
		}
	}
	if (configPath.empty()) {
		if (const char* envPath = getenv("CUBE_CONFIG_PATH")) {
			configPath = envPath;
		} else {
			configPath = CCubeConfigLoader::defaultConfigPath();
		}
	}

	CCalibrationData calibration;
	int cubeId = -1;
	if (!CCubeConfigLoader::loadForThisHost(configPath, calibration, cubeId)) {
		cerr << "Failed to load cube calibration from '" << configPath << "'. Refusing to start." << endl;
		return 1;
	}
	cout << "Loaded calibration for cube " << cubeId << " from '" << configPath << "'" << endl;

	// Comm thread streams the data to a client, in auto-calibrate mode the calib thread
	// consumes the data instead
	std::shared_ptr<IRunnable> consumer;

	if (autoCalibrateMode) {
		consumer = std::make_shared<CCalibComp>(configPath);
	}
	else {
		consumer = std::make_shared<CCommComp>();
	}
	CThread ConsumerThread(consumer.get(), CThread::PRIORITY_ABOVE_NORM);

	if (!ConsumerThread.start()) {
		REPORT_ERROR("main: failed to start ConsumerThread");
	}

	if (!autoCalibrateMode) {
		// gives a client time to connect before the data starts
		for (uint8_t i = 0; i < 3; ++i) {
			cout << "Starting ControlComp in " << int(3-i) << endl;
			this_thread::sleep_for(chrono::seconds(1));
		}
	}

	CControlComp Control(calibration, calibrateMode);
	CThread ControlThread(&Control, CThread::EPriority::PRIORITY_REALTIME);
	if (!ControlThread.start()) {
		REPORT_ERROR("main: failed to start ControlThread");
	}

	if (calibrateMode) {
		// Control thread ends runvar on its own after the calibration time
		while (runvar.load()) {
			this_thread::sleep_for(chrono::milliseconds(100));
		}
	} else {
		cout << "Type a random character to kill program:" << endl;

		char in;
		cin >> in;
		runvar.store(false);
	}

	ControlThread.join();
	myContainer.signalReader();
	ConsumerThread.join();

	cout << "main end" << endl;
	return 0;
}