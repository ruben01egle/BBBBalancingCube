#ifndef CADC_H
#define CADC_H
#include "Global.h"

// needed for ADC
constexpr UInt32 cBIT0 	= 1U << 0U;
constexpr UInt32 cBIT1	= 1U << 1U;
constexpr UInt32 cBIT2	= 1U << 2U;
constexpr UInt32 cBIT3	= 1U << 3U;
constexpr UInt32 cBIT4	= 1U << 4U;
constexpr UInt32 cBIT5	= 1U << 5U;
constexpr UInt32 cBIT6	= 1U << 6U;
constexpr UInt32 cBIT7	= 1U << 7U;
constexpr UInt32 cBIT8	= 1U << 8U;
constexpr UInt32 cBIT9	= 1U << 9U;
constexpr UInt32 cBIT10	= 1U << 10U;
constexpr UInt32 cBIT11	= 1U << 11U;
constexpr UInt32 cBIT12	= 1U << 12U;
constexpr UInt32 cBIT13	= 1U << 13U;
constexpr UInt32 cBIT14	= 1U << 14U;
constexpr UInt32 cBIT15	= 1U << 15U;
constexpr UInt32 cBIT16	= 1U << 16U;
constexpr UInt32 cBIT17	= 1U << 17U;
constexpr UInt32 cBIT18	= 1U << 18U;
constexpr UInt32 cBIT19	= 1U << 19U;
constexpr UInt32 cBIT20	= 1U << 20U;
constexpr UInt32 cBIT21	= 1U << 21U;
constexpr UInt32 cBIT22	= 1U << 22U;
constexpr UInt32 cBIT23	= 1U << 23U;
constexpr UInt32 cBIT24	= 1U << 24U;
constexpr UInt32 cBIT25	= 1U << 25U;
constexpr UInt32 cBIT26	= 1U << 26U;
constexpr UInt32 cBIT27	= 1U << 27U;
constexpr UInt32 cBIT28	= 1U << 28U;
constexpr UInt32 cBIT29	= 1U << 29U;
constexpr UInt32 cBIT30	= 1U << 30U;
constexpr UInt32 cBIT31	= 1U << 31U;

class CADCMMap
{
public:
	bool fetchValue(UInt16& value);
private:
	UInt32 readRegister(const UInt32 addrOffset);
	void writeRegister(const UInt32 addrOffset, const UInt32 value);
public:
	CADCMMap();
	CADCMMap(const CADCMMap&) = delete;
	CADCMMap& operator=(const CADCMMap&) = delete;
	~CADCMMap();
private:
	int mMemoryFD;
	UInt8* mAddrPtr;

	static constexpr UInt32 sSTART_ADDR					= 0x44E0D000U;
	static constexpr UInt32 sEND_ADDR					= 0x44E0F000U;
	static constexpr UInt32 sMEM_SIZE					= sEND_ADDR - sSTART_ADDR;

	static constexpr UInt32 sIDMask						= 0b011110000000000000000U;
	static constexpr UInt32 sValueMask					= 0b111111111111U;
	static constexpr UInt32	sChannel1ID					= 0b00000U;
	static constexpr UInt32 sChannel2ID					= 0b00001U;
	static constexpr UInt32	sChannel3ID					= 0b00010U;

	static constexpr UInt32 sREVISION_Offset 			= 0x00U;
	static constexpr UInt32 sSYSCONFIG_Offset			= 0x10U;
	static constexpr UInt32 sIRQSTATUS_RAW_Offset		= 0x24U;
	static constexpr UInt32 sIRQSTATUS_Offset			= 0x28U;
	static constexpr UInt32 sIRQENABLE_SET_Offset		= 0x2CU;
	static constexpr UInt32 sIREQENABLE_CLR_Offset		= 0x30U;
	static constexpr UInt32 sIRQWAKEUP_Offset			= 0x34U;
	static constexpr UInt32 sDMAENABLE_SET_Offset		= 0x38U;
	static constexpr UInt32 sDMAENABLE_CLR_Offset		= 0x3CU;
	static constexpr UInt32 sCTRL_Offset				= 0x40U;
	static constexpr UInt32 sADCSTAT_Offset				= 0x44U;
	static constexpr UInt32 sADCRANGE_Offset			= 0x48U;
	static constexpr UInt32 sADC_CLKDIV_Offset			= 0x4CU;
	static constexpr UInt32 sADC_MISC_Offset			= 0x50U;
	static constexpr UInt32 sSTEPENABLE_Offset			= 0x54U;
	static constexpr UInt32 sIDLECONFIG_Offset			= 0x58U;
	static constexpr UInt32 sTS_CHARGE_STEPCONFIG_Offset= 0x5CU;
	static constexpr UInt32 sTS_CHARGE_DELAY_Offset		= 0x60U;
	static constexpr UInt32 sSTEPCONFIG1_Offset			= 0x64U;
	static constexpr UInt32 sSTEPDELAY1_Offset			= 0x68U;
	static constexpr UInt32 sSTEPCONFIG2_Offset			= 0x6CU;
	static constexpr UInt32 sSTEPDELAY2_Offset			= 0x70U;
	static constexpr UInt32 sSTEPCONFIG3_Offset			= 0x74U;
	static constexpr UInt32 sSTEPDELAY3_Offset			= 0x78U;
	static constexpr UInt32 sSTEPCONFIG4_Offset			= 0x7CU;
	static constexpr UInt32 sSTEPDELAY4_Offset			= 0x80U;
	static constexpr UInt32 sSTEPCONFIG5_Offset			= 0x84U;
	static constexpr UInt32 sSTEPDELAY5_Offset			= 0x88U;
	static constexpr UInt32 sSTEPCOFNIG6_Offset			= 0x8CU;
	static constexpr UInt32 sSTEPDELAY6_Offset			= 0x90U;
	static constexpr UInt32 sSTEPCOFNIG7_Offset			= 0x94U;
	static constexpr UInt32 sSTEPDELAY7_Offset			= 0x98U;
	static constexpr UInt32 sSTEPCOFNIG8_Offset			= 0x9CU;
	static constexpr UInt32 sSTEPDELAY8_Offset			= 0xA0U;
	static constexpr UInt32 sSTEPCONFIG9_Offset			= 0xA4U;
	static constexpr UInt32 sSTEPDELAY9_Offset			= 0xA8U;
	static constexpr UInt32 sSTEPCONFIG10_Offset		= 0xACU;
	static constexpr UInt32 sSTEPDELAY10_Offset			= 0xB0U;
	static constexpr UInt32 sSTEPCOFNIG11_Offset		= 0xB4U;
	static constexpr UInt32 sSTEPDELAY11_Offset			= 0xB8U;
	static constexpr UInt32 sSTEPCONFIG12_Offset		= 0xBCU;
	static constexpr UInt32 sSTEPDELAY12_Offset			= 0xC0U;
	static constexpr UInt32 sSTEPCOFNIG13_Offset		= 0xC4U;
	static constexpr UInt32 sSTEPDELAY13_Offset			= 0xC8U;
	static constexpr UInt32 sSTEPCONFIG14_Offset		= 0xCCU;
	static constexpr UInt32 sSTEPDELAY14_Offset			= 0xD0U;
	static constexpr UInt32 sSTEPCONFIG15_Offset		= 0xD4U;
	static constexpr UInt32 sSTEPDELAY15_Offset			= 0xD8U;
	static constexpr UInt32 sSTEPCOFNIG16_Offset		= 0xDCU;
	static constexpr UInt32 sSTEPDELAY16_Offset			= 0xE0U;
	static constexpr UInt32 sFIFO0COUNT_Offset			= 0xE4U;
	static constexpr UInt32 sFIFO0THRESHOLD_Offset		= 0xE8U;
	static constexpr UInt32 sDMA0REQ_Offset				= 0xECU;
	static constexpr UInt32 sFIFO1COUNT_Offset			= 0xF0U;
	static constexpr UInt32 sFIFO1THRESHOLD_Offset		= 0xF4U;
	static constexpr UInt32 sDMA1REQ_Offset				= 0xF8U;
	static constexpr UInt32 sFIFO0DATA_Offset			= 0x100U;
	static constexpr UInt32 sFIFO1DATA_Offset			= 0x200U;
};

#endif
