#ifndef CMAXONMOTOR_HPP
#define CMAXONMOTOR_HPP

#include <vector>
#include <memory>

#include <cstdint>
#include "CPWMMMAP.hpp"
#include "CPWMModuleConfig.hpp"
#include "CGPIOMMAP.hpp"
#include "CADCMMap.h"
#include "CADCConfig.hpp"

class CMaxonMotor
{
public:
    enum class Status : uint8_t {
        OKAY,
        PWM_MODULE_NOT_AVAILABLE,
        PWM_MODULE_ERROR,
        HARDWARE_ERROR
    };
    struct PWMCfg{
        double dutyCyclePercentMin;
        double dutyCyclePercentMax;
        double minTarget;
        double maxTarget;
    };

public:
    CMaxonMotor(uint8_t pPWMModule,
                uint8_t pPWMPin,
                uint8_t pEnableGpioNr,
                uint8_t pDirectionGpioNr,
                CPWMModuleConfig pPWMModuleCfg,
                PWMCfg pPWMCfg,
                uint8_t pADCStepIdx,
                CADCConfig pADCCfg,
                double pTorqueConst);
    ~CMaxonMotor();

    Status init();
    Status enable();
    Status disable();
    Status setTorque(double pTorque);
    Status getRawVelocity(uint16_t& pRawVelocity);

private:
    double calculateDutyCyclePercent(double pTarget);
    Status setPWM(double pDutyCyclePercent);

private:
    uint8_t mEnableGPIONr;
    uint8_t mDirectionGPIONr;
    std::shared_ptr<CGPIOMMAP> mEnablePin;
    std::shared_ptr<CGPIOMMAP> mDirectionPin;

    uint8_t mADCStepIdx;
    CADCMMap mADC;
    CADCConfig mADCCfg;

    std::shared_ptr<CPWMMMAP> mPWM;
    uint8_t mPWMModule;
    CPWMModuleConfig mModuleConfig;
    uint8_t mPWMPin;
    PWMCfg mPWMCfg;

    const double mTorqueConst;
};

#endif