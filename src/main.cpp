#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>

using namespace std;

#include "CHardware.hpp"
#include "CIMUData.h"

int main(){
	cout << "main running" << endl;

	CBBBHardware hw;
    if (!hw.init()) {
        return EXIT_FAILURE;
    }
    cout << "init done" << endl;
	CIMUData imu1_data;
	CIMUData imu2_data;
	uint16_t adc = 0;

    hw.enableMotor();
    this_thread::sleep_for(chrono::milliseconds(500));

    cout << "enter loop" << endl;
	for(size_t i = 0; i < 1000; i++) {
        if(!hw.fetchValues(adc, imu1_data, imu2_data)) {
            cout << "error reading data" << endl;
            break;
        }

        if (i<300) {
            hw.setTorque(0.1);
        }
        else if (i<600) {
            hw.setTorque(-0.1);
        }
        else {
            hw.setTorque(0.0);
        }

        cout << "ADC: " << adc
             << " | IMU1 [DotPhi: " << imu1_data.mDotPhi 
             << ", DDotX: " << imu1_data.mDDotX 
             << ", DDotY: " << imu1_data.mDDotY << "]"
             << " | IMU2 [DotPhi: " << imu2_data.mDotPhi 
             << ", DDotX: " << imu2_data.mDDotX 
             << ", DDotY: " << imu2_data.mDDotY << "]"
             << endl;

        this_thread::sleep_for(chrono::milliseconds(20));
    }

    hw.disableMotor();

	cout << "main end" << endl;
	return 0;
}