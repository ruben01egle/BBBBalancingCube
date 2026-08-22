#include "CPWMMMAP.hpp"

#include <string>
#include <cstring>
#include <sstream>
#include <dirent.h>
#include <climits>
#include "CErrorReporter.hpp"

using namespace std;

CPWMMMAP::CPWMMMAP(uint8_t pPWMModule) :
                            mapPtr(NULL),
                            mPWMModule(pPWMModule),
                            mModuleConfig{std::nullopt, std::nullopt},
                            mCurrentPrescaler(0.0f),
                            mCurrentPeriod(0U),
                            mCurrentDutyCycle(0U),
                            ADDR_START_CLKCTRL  (0x44E00000U),
                            ADDR_START_CTRLMOD  (0x44E10000U),
                            ADDR_START_PWM      {0x48300000U, 0x48302000U, 0x48304000U},
                            MAP_SIZE_CLKCTRL    (0x400U),
                            MAP_SIZE_CTRLMOD    (0x2000U),
                            MAP_SIZE_PWM        (0x260U),
                            OFFS_EPWMSS_CLKCTRL {0xD4U, 0xCCU, 0xD8U},
                            OFFS_CTRLMOD_PWM    (0x664U),
                            PWM_OFFS            {0x200U},
                            OFFS_CLKCONFIG      (0x8U),
                            OFFS_TBCTL          (0x0U),
                            OFFS_TBSTS          (0x2U),
                            OFFS_TBPHS          (0x6U),
                            OFFS_TBCNT          (0x8U),
                            OFFS_TBPRD          (0xAU),
                            OFFS_CMPCTL         (0xEU),
                            OFFS_CMPA           (0x12U),
                            OFFS_CMPB           (0x14U),
                            OFFS_AQCTLA         (0x16U),
                            OFFS_AQCTLB         (0x18U),
                            OFFS_TZCTL          (0x28U) 
{}

std::optional<std::string> CPWMMMAP::resolvePwmChipIndex(uint8_t pPWMModule) const
{
    // Match on the module's physical base address (e.g. "48304") rather than assuming the
    // sysfs pwmchip index equals the module number -- the kernel numbers chips by probe order.
    ostringstream addrStream;
    addrStream << hex << ADDR_START_PWM[pPWMModule];
    string addrPrefix = addrStream.str().substr(0, 5);

    DIR* dir = opendir("/sys/class/pwm");
    if (!dir) {
        return std::nullopt;
    }

    std::optional<std::string> result;
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        string name = entry->d_name;
        if (name.rfind("pwmchip", 0) != 0) {
            continue;
        }
        string linkPath = "/sys/class/pwm/" + name;
        char buf[PATH_MAX];
        ssize_t len = readlink(linkPath.c_str(), buf, sizeof(buf) - 1);
        if (len < 0) {
            continue;
        }
        buf[len] = '\0';
        if (string(buf).find(addrPrefix) != string::npos) {
            result = name.substr(strlen("pwmchip"));
            break;
        }
    }
    closedir(dir);
    return result;
}

CPWMMMAP::~CPWMMMAP() {
    if(mapPtr && mapPtr != MAP_FAILED) {
        munmap(mapPtr, MAP_SIZE_PWM);
    }
}

CPWMMMAP::Status CPWMMMAP::init(uint8_t pPWMPin, CPWMModuleConfig pModuleConfig)
{
    // The time-base is shared by both pins of a module. Only run the mmap/clock/timebase setup
    // once per module -- redoing it for a second pin would leak the previous mapping and glitch
    // the already-running pin's output.
    bool moduleAlreadyInitialized = (mapPtr != nullptr && mapPtr != MAP_FAILED);

    if (!moduleAlreadyInitialized) {
        int mMemoryFD = open("/dev/mem", O_RDWR | O_SYNC);
        if(mMemoryFD < 0) {
            REPORT_ERROR_ERRNO("unable to open /dev/mem");
            return Status::MMMAP_ERROR;
        }

        // 1. Clock Module mmappos
        uint8_t* clkPtr = reinterpret_cast<uint8_t*>(mmap(0, MAP_SIZE_CLKCTRL, PROT_READ | PROT_WRITE, MAP_SHARED, mMemoryFD, ADDR_START_CLKCTRL));
        if(clkPtr == MAP_FAILED) {
            close(mMemoryFD);
            REPORT_ERROR_ERRNO("unable to mmap clock module");
            return Status::MMMAP_ERROR;
        }

        // Enable device clock (mit volatile!)
        *reinterpret_cast<volatile uint32_t*>(clkPtr + OFFS_EPWMSS_CLKCTRL[mPWMModule]) |= 0x2;

        int counter = 0;
        while((*reinterpret_cast<volatile uint32_t*>(clkPtr + OFFS_EPWMSS_CLKCTRL[mPWMModule]) & 0x00030000) != 0x0) {
            usleep(1);
            if(++counter > 1E6) {
                munmap(clkPtr, MAP_SIZE_CLKCTRL);
                close(mMemoryFD);
                REPORT_ERROR("can not enable EPWMSS clock module!");
                return Status::FAILED_TO_ENABLE_DEVICE;
            }
        }
        munmap(clkPtr, MAP_SIZE_CLKCTRL);

        // 2. Control Module mmap
        uint8_t* ctrlPtr = reinterpret_cast<uint8_t*>(mmap(0, MAP_SIZE_CTRLMOD, PROT_READ | PROT_WRITE, MAP_SHARED, mMemoryFD, ADDR_START_CTRLMOD));
        if(ctrlPtr != MAP_FAILED) {
            *reinterpret_cast<volatile uint32_t*>(ctrlPtr + OFFS_CTRLMOD_PWM) |= (0x1 << mPWMModule);
            munmap(ctrlPtr, MAP_SIZE_CTRLMOD);
        }

        // 3. PWM Base mmap
        mapPtr = reinterpret_cast<uint8_t*>(mmap(0, MAP_SIZE_PWM, PROT_READ | PROT_WRITE, MAP_SHARED, mMemoryFD, ADDR_START_PWM[mPWMModule]));
        close(mMemoryFD);

        if(mapPtr == MAP_FAILED) {
            REPORT_ERROR_ERRNO("unable to mmap PWM");
            return Status::MMMAP_ERROR;
        }

        // Enable internal module clock
        *reinterpret_cast<volatile uint32_t*>(mapPtr + OFFS_CLKCONFIG) |= 0x111;

        // Clear registers directly
        *reinterpret_cast<volatile uint16_t*>(mapPtr + PWM_OFFS + OFFS_TBPHS) = 0x0000;
        *reinterpret_cast<volatile uint16_t*>(mapPtr + PWM_OFFS + OFFS_TBCNT) = 0x0000;

        // Set TBCTL settings
        uint16_t tbctl = *reinterpret_cast<volatile uint16_t*>(mapPtr + PWM_OFFS + OFFS_TBCTL);
        tbctl &= uint16_t(0xDFB0);
        tbctl |= 0xC030;
        *reinterpret_cast<volatile uint16_t*>(mapPtr + PWM_OFFS + OFFS_TBCTL) = tbctl;

        // Configure shadow registers & Trip Zone
        *reinterpret_cast<volatile uint16_t*>(mapPtr + PWM_OFFS + OFFS_CMPCTL) &= uint16_t(0xFFA0);
        *reinterpret_cast<volatile uint16_t*>(mapPtr + PWM_OFFS + OFFS_TZCTL) |= 0xF;

        if (setFrequency(pModuleConfig.mFrequency) != Status::OKAY) {
            REPORT_ERROR("Error setting PWM frequency");
            return Status::INVALID_FREQUENCY;
        }
    }

    // Hand the channel to the kernel's PWM driver through the real sysfs path. This is what
    // actually resumes pm_runtime and ungates the eHRPWM functional/time-base clock -- the
    // CM_PER OFFS_EPWMSS_CLKCTRL poke above only ungates register (bus) access, not the internal
    // counter clock, so without this TBCNT never advances and no CMPA/CMPB write can ever load
    // from its shadow register into the active one. The sysfs pwmchip index is assigned by the
    // kernel in probe order and is not the same as the physical module number, so it has to be
    // resolved rather than assumed.
    auto chipIndex = resolvePwmChipIndex(mPWMModule);
    if (!chipIndex.has_value()) {
        REPORT_ERROR("Unable to find sysfs pwmchip for PWM module ", std::to_string(mPWMModule));
        return Status::DRIVER_ERROR;
    }

    string pwmNr = to_string(pPWMPin);
    string chip_path = "/sys/class/pwm/pwmchip" + chipIndex.value();
    string export_path = chip_path + "/export";
    string pwm_path = chip_path + "/pwm" + pwmNr;

    if (access(pwm_path.c_str(), F_OK) < 0) {
        int export_fd = open(export_path.c_str(), O_WRONLY);
        if (export_fd < 0) {
            REPORT_ERROR_ERRNO("Failed to open ", export_path);
            return Status::DRIVER_ERROR;
        }
        ssize_t written = write(export_fd, pwmNr.c_str(), pwmNr.size());
        close(export_fd);
        if (written != static_cast<ssize_t>(pwmNr.size())) {
            REPORT_ERROR_ERRNO("Failed to export ", pwmNr, " on ", chip_path);
            return Status::DRIVER_ERROR;
        }
    }

    // Period must be written before duty_cycle/enable -- the kernel PWM core rejects enabling a
    // channel whose period is still zero.
    string period_path = pwm_path + "/period";
    string periodStr = to_string(static_cast<int64_t>(1.0 / pModuleConfig.mFrequency * 1E9));
    int period_fd = open(period_path.c_str(), O_WRONLY);
    if (period_fd < 0 || write(period_fd, periodStr.c_str(), periodStr.size()) != static_cast<ssize_t>(periodStr.size())) {
        REPORT_ERROR_ERRNO("Failed to write ", periodStr, " to ", period_path);
        if (period_fd >= 0) close(period_fd);
        return Status::DRIVER_ERROR;
    }
    close(period_fd);

    string dutycycle_path = pwm_path + "/duty_cycle";
    int dc_fd = open(dutycycle_path.c_str(), O_WRONLY);
    if (dc_fd < 0 || write(dc_fd, "0", 1) != 1) {
        REPORT_ERROR_ERRNO("Failed to write to ", dutycycle_path);
        if (dc_fd >= 0) close(dc_fd);
        return Status::DRIVER_ERROR;
    }
    close(dc_fd);

    string enable_path = pwm_path + "/enable";
    int enable_fd = open(enable_path.c_str(), O_WRONLY);
    if (enable_fd < 0 || write(enable_fd, "1", 1) != 1) {
        REPORT_ERROR_ERRNO("Failed to write to ", enable_path);
        if (enable_fd >= 0) close(enable_fd);
        return Status::DRIVER_ERROR;
    }
    close(enable_fd);

    setHighLowActive(pModuleConfig.mActiveHigh);

    mModuleConfig[pPWMPin] = pModuleConfig;
    return Status::OKAY;
}

CPWMMMAP::Status CPWMMMAP::setDutyCycle(uint8_t pPWMPin, double pDutyCyclePercent)
{
    // Compute integer tick duty cycle
    mCurrentDutyCycle = static_cast<uint32_t>(mCurrentPeriod * (pDutyCyclePercent / 100.0));

    switch(pPWMPin) {
    case 0:
        *reinterpret_cast<volatile uint16_t*>(mapPtr + PWM_OFFS + OFFS_CMPA) = static_cast<uint16_t>(mCurrentDutyCycle);
        break;
    case 1:
        *reinterpret_cast<volatile uint16_t*>(mapPtr + PWM_OFFS + OFFS_CMPB) = static_cast<uint16_t>(mCurrentDutyCycle);
        break;
    default:
        REPORT_ERROR("Invalid PWM-Pin");
        return Status::INVALID_PIN;
    }

    return Status::OKAY;
}

CPWMMMAP::Status CPWMMMAP::setFrequency(uint32_t pFrequencyHz) {
    uint16_t tbctl = *reinterpret_cast<volatile uint16_t*>(mapPtr + PWM_OFFS + OFFS_TBCTL);
    tbctl &= uint16_t(0xE07F);

    if(pFrequencyHz >= 1526) {
        mCurrentPrescaler = 1.0f;
    } else if(pFrequencyHz >= 763) {
        tbctl |= 0x80;
        mCurrentPrescaler = 2.0f;
    } else if(pFrequencyHz >= 382) {
        tbctl |= 0x100;
        mCurrentPrescaler = 4.0f;
    } else if (pFrequencyHz >= 48) {
        tbctl |= 0x1080;
        mCurrentPrescaler = 32.0f;
    } else {
        return Status::INVALID_FREQUENCY;
    }

    *reinterpret_cast<volatile uint16_t*>(mapPtr + PWM_OFFS + OFFS_TBCTL) = tbctl;

    mCurrentPeriod = static_cast<uint32_t>(1.0f / (pFrequencyHz * mCurrentPrescaler * 1E-8f) - 1.0f);

    // DIREKTE ZUWEISUNG für TBPRD
    *reinterpret_cast<volatile uint16_t*>(mapPtr + PWM_OFFS + OFFS_TBPRD) = static_cast<uint16_t>(mCurrentPeriod);

    return Status::OKAY;
}

void CPWMMMAP::setHighLowActive(bool pActiveHigh) {
    if(pActiveHigh) {
        *reinterpret_cast<volatile uint16_t*>(mapPtr + PWM_OFFS + OFFS_AQCTLA) = 0x12;
        *reinterpret_cast<volatile uint16_t*>(mapPtr + PWM_OFFS + OFFS_AQCTLB) = 0x102;
    } else {
        *reinterpret_cast<volatile uint16_t*>(mapPtr + PWM_OFFS + OFFS_AQCTLA) = 0x24;
        *reinterpret_cast<volatile uint16_t*>(mapPtr + PWM_OFFS + OFFS_AQCTLB) = 0x24;
    }
}