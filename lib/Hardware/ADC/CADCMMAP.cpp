#include "CADCMMAP.hpp"

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctime>

#include "CErrorReporter.hpp"

namespace {
    // ONESHOT conversions on a healthy step complete within microseconds; this floor is
    // generous headroom so a stalled/misconfigured step still fails fast instead of hanging
    // the calling thread (the motor current/velocity feedback read) forever.
    constexpr long ONESHOT_TIMEOUT_USEC = 100000L;
}

std::array<std::queue<uint16_t>, CADCMMAP::NUM_STEPS> CADCMMAP::mValueQueues;
std::mutex CADCMMAP::mQueueMutex;

CADCMMAP::CADCMMAP(uint8_t pStepNumber)
{
    mStepIdx = pStepNumber;
    mModuleInitialized = false;
    mMapPtr = nullptr;
}

CADCMMAP::~CADCMMAP()
{
    if (mMapPtr != nullptr && mMapPtr != MAP_FAILED) {
        writeRegister(OFFS_CTRL, 0);
        writeRegister(OFFS_STEPENABLE, 0);
        if(munmap(mMapPtr, MAP_SIZE) == -1) {
            REPORT_ERROR_ERRNO("Unable to delete ADC MMAP");
        }
    }
}

CADCMMAP::Status CADCMMAP::init(CADCConfig pADCConfig)
{
    if (mStepIdx < 1 || mStepIdx > 16) {
        REPORT_ERROR_ERRNO("Step Number out of range");
        return Status::CONFIG_ERROR;
    }
    mADCConfig = pADCConfig;

    if (!mModuleInitialized) {
        int MemoryFD = open("/dev/mem", O_RDWR | O_SYNC);
        mMapPtr = reinterpret_cast<uint8_t*>(mmap(0,				// start address for new mapping
                MAP_SIZE,									        // mapped length
                PROT_READ | PROT_WRITE,								// permit read and write operations
                MAP_SHARED,											// share changes with other processes
                MemoryFD,											// mem filedescriptor
                ADDR_START));								        // start address in mem file
        close(MemoryFD);
        if(mMapPtr == ((uint8_t*)-1)) {
            REPORT_ERROR_ERRNO("Unable to mmap ADC");
            return Status::MMAP_ERROR;
        }
        
        writeRegister(OFFS_CTRL, 0);                // reset
        writeRegister(OFFS_STEPENABLE, 0);          // reset
        setBits(OFFS_CTRL, 0x01 << 1U);             // activate step tags in fifo
        mModuleInitialized = true;
    }
    clearBits(OFFS_CTRL, 0x01);                     // stop adc
    if (configStep() != Status::OKAY) {
        return Status::CONFIG_ERROR;
    }
    setBits(OFFS_CTRL, 0x01);                       // start adc

    return Status::OKAY;
}

CADCMMAP::Status CADCMMAP::readADC(uint16_t& pValue)
{
    switch (mADCConfig.mode)
    {
    case CADCConfig::Mode::ONESHOT:
    {
        setBits(OFFS_STEPENABLE, 0x01 << mStepIdx);

        struct timespec deadline;
        clock_gettime(CLOCK_MONOTONIC, &deadline);
        deadline.tv_nsec += ONESHOT_TIMEOUT_USEC * 1000L;
        deadline.tv_sec += deadline.tv_nsec / 1000000000L;
        deadline.tv_nsec %= 1000000000L;

        bool haveValue = false;
        while (!haveValue) {
            readFifo();
            {
                std::lock_guard<std::mutex> lock(mQueueMutex);
                haveValue = !mValueQueues[mStepIdx-1].empty();
            }
            if (haveValue) {
                break;
            }
            struct timespec now;
            clock_gettime(CLOCK_MONOTONIC, &now);
            if (now.tv_sec > deadline.tv_sec ||
                    (now.tv_sec == deadline.tv_sec && now.tv_nsec >= deadline.tv_nsec)) {
                REPORT_ERROR("ADC oneshot read timed out on step ", static_cast<int>(mStepIdx));
                return Status::NO_VALUE;
            }
        }
        break;
    }

    case CADCConfig::Mode::CONTINUOUS:
        readFifo();
        break;
    default:
        return Status::CONFIG_ERROR;
    }

    bool newVal = false;
    {
        std::lock_guard<std::mutex> lock(mQueueMutex);
        // Only returns latest value, all other values are discarted
        while (!mValueQueues[mStepIdx-1].empty()) {
            pValue = mValueQueues[mStepIdx-1].front();
            mValueQueues[mStepIdx-1].pop();
            newVal = true;
        }
    }

    if (!newVal) {
        REPORT_ERROR("No new value available");
        return Status::NO_VALUE;
    }

    return Status::OKAY;
}

void CADCMMAP::readFifo()
{
    uint32_t fifoDataOffset;
    uint32_t fifoCountOffset;
    switch (mADCConfig.fifo)
    {
    case CADCConfig::FIFOSel::FIFO0:
        fifoDataOffset = OFFS_FIFO0DATA;
        fifoCountOffset = OFFS_FIFO0COUNT;
        break;

    case CADCConfig::FIFOSel::FIFO1:
        fifoDataOffset = OFFS_FIFO1DATA;
        fifoCountOffset = OFFS_FIFO1COUNT;
        break;
    
    default:
        REPORT_ERROR("Invalid FIFO");
        return;
    }
    uint32_t regValue;
    uint32_t stepIdx;
    uint16_t data;
    std::lock_guard<std::mutex> lock(mQueueMutex);
    while (readRegister(fifoCountOffset) > 0) {
        regValue = readRegister(fifoDataOffset);
        stepIdx = (regValue & MASK_STEP_IDX) >> 16;
        data = static_cast<uint16_t>(regValue & MASK_DATA);
        if (stepIdx >= NUM_STEPS) {
            REPORT_ERROR("FIFO entry has out-of-range step index ", stepIdx);
            continue;
        }
        mValueQueues[stepIdx].push(data);
    }
}

uint32_t CADCMMAP::readRegister(const uint32_t pAddrOffset)
{
    return *reinterpret_cast<volatile uint32_t*>(mMapPtr + pAddrOffset);
}

void CADCMMAP::writeRegister(const uint32_t pAddrOffset, const uint32_t pValue)
{
    *reinterpret_cast<volatile uint32_t*>(mMapPtr + pAddrOffset) = pValue;
}

void CADCMMAP::setBits(const uint32_t pAddrOffset, const uint32_t pBitMask)
{
    uint32_t regValue = readRegister(pAddrOffset);
    regValue |= pBitMask;
    *reinterpret_cast<volatile uint32_t*>(mMapPtr + pAddrOffset) = regValue;
}

void CADCMMAP::clearBits(const uint32_t pAddrOffset, const uint32_t pBitMask)
{
    uint32_t regValue = readRegister(pAddrOffset);
    regValue &= ~pBitMask;
    *reinterpret_cast<volatile uint32_t*>(mMapPtr + pAddrOffset) = regValue;
}

CADCMMAP::Status CADCMMAP::configStep()
{
    setBits(OFFS_CTRL, 0x01 << 2U);

    uint32_t regValueCfg = STEPCONFIG_SEL_INP_MODE;
    regValueCfg |= static_cast<uint32_t>(mADCConfig.averaging);
    regValueCfg |= static_cast<uint32_t>(mADCConfig.channel);
    regValueCfg |= static_cast<uint32_t>(mADCConfig.mode);
    regValueCfg |= static_cast<uint32_t>(mADCConfig.fifo);
    writeRegister(OFFS_STEPCONFIG[mStepIdx-1], regValueCfg);

    uint32_t regValueDel = 0;
    regValueDel |= static_cast<uint32_t>(mADCConfig.openDelay);
    regValueDel |= static_cast<uint32_t>(mADCConfig.sampleDelay) << 24;
    writeRegister(OFFS_STEPDELAY[mStepIdx-1], regValueDel);

    if (mADCConfig.mode == CADCConfig::Mode::CONTINUOUS) {
        setBits(OFFS_STEPENABLE, 0x01 << mStepIdx);
    }

    clearBits(OFFS_CTRL, 0x01 << 2U);
    return Status::OKAY;
}
