/**
 * @author	Michael Meindl
 * @date	5.12.2016
 * @brief	Container class used for communication between the components.
 * 			The comm-component waits for the read semaphore, reads the values
 * 			and transmits them via the TCP/IP-socket.
 * 			The control-component writes the values - without signaling - and gives the read semaphore.
 */
#pragma once

#include <cstdint>
#include "CContent.hpp"
#include "CStateVectorData.hpp"
#include "CIMUDataCalibrated.hpp"
#include "CBinarySemaphore.hpp"

class CContainer
{
public:
	bool getContent(bool waitForever,
					CContent& content);
	void signalReader();
	bool writeData(const int64_t timeUs,
                              const uint16_t adcValue,
                              const float torque,
                              const CIMUData& sensor1,
                              const CIMUData& sensor2,
                              const CIMUDataCalibrated& sensor1Calib,
                              const CIMUDataCalibrated& sensor2Calib,
                              const CStateVectorData& stateData);

	bool writeTime(const int64_t timeUs);
	bool writeADCValue(const uint16_t adcValue);
	bool writeTorqueValue(const float torque);
	bool writeSensor1Data(const CIMUData& sensorData);
	bool writeSensor2Data(const CIMUData& sensorData);
	bool writeSensor1CalibData(const CIMUDataCalibrated& sensorData);
	bool writeSensor2CalibData(const CIMUDataCalibrated& sensorData);
	bool writeStateData(const CStateVectorData& sensorData);
public:
	CContainer();
private:
	CContent mContent;
	CBinarySemaphore mReadSem;
};
