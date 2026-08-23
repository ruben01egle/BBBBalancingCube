/*
 * CGPIOMMAP.cpp
 *
 *  Created on: Jun 12, 2023
 *      Author: Knut
 */

#include "CGPIOMMAP.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <stdio.h>
#include <cstdlib>

#include "CErrorReporter.hpp"

std::mutex CGPIOMMAP::mOEMutex;

CGPIOMMAP::CGPIOMMAP(uint8_t pGPIONumber) :
					mapPtr				(NULL),
					ADDR_START_CM_PER	(0x44E00000U),
					ADDR_START_CM_WKUP	(0x44E00400U),
					ADDR_START_GPIOMOD	{0x44E07000U, 0x4804C000U, 0x481AC000U, 0x481AE000U},
					MAP_SIZE_CM_PER		(0x400U),
					MAP_SIZE_CM_WKUP	(0x100U),
					MAP_SIZE_GPIOMOD	(0x1000U),
					OFFS_CM_GPIO		{0x8U, 0xACU, 0xB0U, 0xB4U},
					OFFS_SYSCONFIG		(0x10U),
					OFFS_OE				(0x134U),
					OFFS_DATA_IN		(0x138U),
					OFFS_CLEAR_DOUT		(0x190U),
					OFFS_SET_DOUT		(0x194U)
{
	mGPIOModuleIndex = pGPIONumber/32;
	mGPIOPinIndex = pGPIONumber % 32;
	mOutput = false;
}

CGPIOMMAP::~CGPIOMMAP() {
	if (mapPtr != nullptr && mapPtr != MAP_FAILED) {
		if(munmap(mapPtr, MAP_SIZE_GPIOMOD) == -1) {
			REPORT_ERROR_ERRNO("Unable to delete GPIO module mapping");
		}
	}
}

CGPIOMMAP::Status CGPIOMMAP::init( bool pOutput)
{
	if (mGPIOModuleIndex >= 4) {
		REPORT_ERROR("GPIO module index out of range");
		return Status::CONFIG_ERROR;
	}

	mOutput = pOutput;
	// mmap to required memory space for CM_PER
	int mMemoryFD = open("/dev/mem", O_RDWR | O_SYNC);
	if (mMemoryFD < 0) {
		REPORT_ERROR_ERRNO("unable to open /dev/mem");
		return Status::MMAP_ERROR;
	}
	if(mGPIOModuleIndex != 0) {
		// mmap registers for required clock module
		mapPtr = reinterpret_cast<uint8_t*>(mmap(0,					// start address for new mapping
				MAP_SIZE_CM_PER,									// mapped length
				PROT_READ | PROT_WRITE,								// permit read and write operations
				MAP_SHARED,											// share changes with other processes
				mMemoryFD,											// mem filedescriptor
				ADDR_START_CM_PER));								// start address in mem file

		if(mapPtr == MAP_FAILED) {								// check for successful mapping
			REPORT_ERROR_ERRNO("Unable to mmap clock module peripheral registers");
			close(mMemoryFD);
			return Status::MMAP_ERROR;
		}

		// enable device
		*reinterpret_cast<volatile uint32_t*>(mapPtr+OFFS_CM_GPIO[mGPIOModuleIndex]) |= 0x2;

		// wait until device is enabled, abort if this takes longer than 1 second.
		int counter = 0;
		while((*reinterpret_cast<volatile uint32_t*>(mapPtr+OFFS_CM_GPIO[mGPIOModuleIndex]) & 0x00030000) != 0x0) {
			usleep(1);

			counter++;
			if(counter > 1E6) {
				REPORT_ERROR("Can not enable gpio module");
				munmap(mapPtr, MAP_SIZE_CM_PER);
				close(mMemoryFD);
				return Status::MMAP_ERROR;
			}
		}

		// delete mapping and check if operation is successful
		if(munmap(mapPtr, MAP_SIZE_CM_PER) == -1) {
			REPORT_ERROR_ERRNO("Unable to delete clock module peripheral register mapping");
			close(mMemoryFD);
			return Status::MMAP_ERROR;
		}
	} else {	// mGPIOModulIndex == 0
		// mmap registers for required clock module (WKUP)
		mapPtr = reinterpret_cast<uint8_t*>(mmap(0,					// start address for new mapping
				MAP_SIZE_CM_WKUP,									// mapped length
				PROT_READ | PROT_WRITE,								// permit read and write operations
				MAP_SHARED,											// share changes with other processes
				mMemoryFD,											// mem filedescriptor
				ADDR_START_CM_WKUP));								// start address in mem file

		if(mapPtr == MAP_FAILED) {								// check for successful mapping
			REPORT_ERROR_ERRNO("Unable to mmap clock module peripheral registers (WKUP)");
			close(mMemoryFD);
			return Status::MMAP_ERROR;
		}

		// enable device
		*reinterpret_cast<volatile uint32_t*>(mapPtr+OFFS_CM_GPIO[mGPIOModuleIndex]) |= 0x2;

		// wait until device is enabled, abort if this takes longer than 1 second.
		int counter = 0;
		while((*reinterpret_cast<volatile uint32_t*>(mapPtr+OFFS_CM_GPIO[mGPIOModuleIndex]) & 0x00030000) != 0x0) {
			usleep(1);

			counter++;
			if(counter > 1E6) {
				REPORT_ERROR_ERRNO("Can not enable gpio module");
				munmap(mapPtr, MAP_SIZE_CM_WKUP);
				close(mMemoryFD);
				return Status::MMAP_ERROR;
			}
		}

		// delete mapping and check if operation is successful
		if(munmap(mapPtr, MAP_SIZE_CM_WKUP) == -1) {
			REPORT_ERROR_ERRNO("Unable to delete clock module peripheral register (WKUP) mapping");
			close(mMemoryFD);
			return Status::MMAP_ERROR;
		}
	}

	// mmap registers for GPIO-Module
	mapPtr = reinterpret_cast<uint8_t*>(mmap(0,					// start address for new mapping
			MAP_SIZE_GPIOMOD,									// mapped length
			PROT_READ | PROT_WRITE,								// permit read and write operations
			MAP_SHARED,											// share changes with other processes
			mMemoryFD,											// mem filedescriptor
			ADDR_START_GPIOMOD[mGPIOModuleIndex]));				// start address in mem file

	close(mMemoryFD);

	if(mapPtr == MAP_FAILED) {								// check for successful mapping
		REPORT_ERROR_ERRNO("unable to mmap GPIO-Module");
		return Status::MMAP_ERROR;
	}

	// set no-idle in GPIO_SYSCONFIG
	*reinterpret_cast<volatile uint32_t*>(mapPtr + OFFS_SYSCONFIG) |= 0x8;

	// configure GPIO output abilities
	{
		std::lock_guard<std::mutex> lock(mOEMutex);
		if (mOutput) {
			*reinterpret_cast<volatile uint32_t*>(mapPtr + OFFS_OE) &= ~(1U << mGPIOPinIndex);
		}
		else {
			*reinterpret_cast<volatile uint32_t*>(mapPtr + OFFS_OE) |= (1U << mGPIOPinIndex);
		}
	}

	return Status::OKAY;
}

CGPIOMMAP::Status CGPIOMMAP::setHigh() {
	if (!mOutput) {
		return Status::CONFIG_ERROR;
	}
	*reinterpret_cast<volatile uint32_t*>(mapPtr + OFFS_SET_DOUT) = (0x1 << mGPIOPinIndex);
	return Status::OKAY;
}

CGPIOMMAP::Status CGPIOMMAP::setLow() {
	if (!mOutput) {
		return Status::CONFIG_ERROR;
	}
	*reinterpret_cast<volatile uint32_t*>(mapPtr + OFFS_CLEAR_DOUT) = (0x1 << mGPIOPinIndex);
	return Status::OKAY;
}

bool CGPIOMMAP::getCurrentState() {
	return ((*reinterpret_cast<volatile uint32_t*>(mapPtr + OFFS_DATA_IN) >> mGPIOPinIndex) & 0x1);
}
