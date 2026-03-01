/**
 * @author	Michael Meindl
 * @date	5.12.2016
 * @brief	Container class used for communication between the components.
 * 			The comm-component waits for the read semaphore, reads the values
 * 			and transmits them via the TCP/IP-socket.
 * 			The control-component writes the values - without signaling - and gives the read semaphore.
 */
#ifndef CCONTAINER_H
#define CCONTAINER_H
#include <cstdint>
#include "CContent.h"
#include "CStateVectorData.h"
#include "CBinarySemaphore.h"

class CContainer
{
public:
	bool getContent(bool waitForever,
					CContent& content);
	void signalReader();
	bool writeTime(const int64_t timeUs);
	bool writeADCValue(const uint16_t adcValue);
	bool writeTorqueValue(const float torque);
	bool writeSensor1Data(const CIMUData& sensorData);
	bool writeSensor2Data(const CIMUData& sensorData);
	bool writeStateData(const CStateVectorData& sensorData);
public:
	CContainer();
private:
	CContent mContent;
	CBinarySemaphore mReadSem;
};

#endif
