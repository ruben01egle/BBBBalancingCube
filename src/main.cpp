#include <iostream>
#include <thread>
#include <chrono>

using namespace std;

#include "CMPU9250.h"
#include "CIMUData.h"

int main(){
	cout << "main running" << endl;

	CMPU9250 imu1("/dev/spidev1.1");
	imu1.init(0b00011000U);
	CIMUData imu1_data;

	for(size_t i=0;i<1000;i++) {
		if(!imu1.readSensorData(imu1_data)) {
			cout << "error reading data" << endl;
			break;
		}
		cout << "DotPhi (Z-Rot): " << imu1_data.mDotPhi 
                 << " | DDotX (Acc X): " << imu1_data.mDDotX 
                 << " | DDotY (Acc Y): " << imu1_data.mDDotY << endl;
		this_thread::sleep_for(chrono::milliseconds(20));
	}

	cout << "main end" << endl;
	return 0;
}