#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "CADCMMap.h"
#include "Global.h"
#include "Assertion.h"

CADCMMap::CADCMMap() : mMemoryFD(-1), mAddrPtr(nullptr)
{
	mMemoryFD = open("/dev/mem", O_RDWR);
	sAssertion(mMemoryFD >= 0, "(CSPI::CSPI) Failed to open /dev/mem", true);
	mAddrPtr = reinterpret_cast<UInt8*>(mmap(0, sMEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mMemoryFD, sSTART_ADDR));

	this->writeRegister(sCTRL_Offset, 0U);

	this->writeRegister(sSTEPCONFIG1_Offset, cBIT26);
	this->writeRegister(sSTEPCONFIG2_Offset, cBIT26);
	this->writeRegister(sSTEPCONFIG3_Offset, cBIT26);

	this->writeRegister(sCTRL_Offset, cBIT2 | cBIT1 |cBIT0);

}
CADCMMap::~CADCMMap()
{
	close(mMemoryFD);
}
void CADCMMap::writeRegister(const UInt32 addrOffset, const UInt32 value)
{
	*reinterpret_cast<UInt32*>(mAddrPtr + addrOffset) = value;
}
UInt32 CADCMMap::readRegister(const UInt32 addrOffset)
{
	return *reinterpret_cast<UInt32*>(mAddrPtr + addrOffset);
}
bool CADCMMap::fetchValue(UInt16& data)
{
	this->writeRegister(sSTEPENABLE_Offset, cBIT3 | cBIT2 | cBIT1);
	bool finished = false;
	do
	{
		UInt32 fifo1Count = this->readRegister(sFIFO1COUNT_Offset);
		if(fifo1Count >= 3U)
		{
			finished = true;
			for(UInt32 i = 0U; i < 3U; i++)
			{
				UInt16 value = this->readRegister(sFIFO1DATA_Offset);
				value 		 = ( value & sValueMask);
				switch(i)
				{
				case 0:
					data = value;
					break;
				case 1:

					break;
				case 2:

					break;
				default:
					break;
				}
			}
		}
	}while( (finished == false) );
	return finished;

}
