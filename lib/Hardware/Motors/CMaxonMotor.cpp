#include "CMaxonMotor.hpp"

#include <thread>
#include <chrono>
#include <algorithm>

#include "CInterfaceManager.hpp"

CMaxonMotor::CMaxonMotor(uint8_t pPWMModule,
                        uint8_t pPWMPin,
                        uint8_t pEnableGpioNr,
                        uint8_t pDirectionGpioNr,
                        CPWMModuleConfig pPWMModuleCfg,
                        PWMCfg pPWMCfg,
                        uint8_t pADCStepIdx,
                        CADCConfig pADCCfg,
                        double pTorqueConst):
                            mEnableGPIONr(pEnableGpioNr),
                            mDirectionGPIONr(pDirectionGpioNr),
                            mADCStepIdx(pADCStepIdx),
                            mADCCfg(pADCCfg),
                            mPWMModule(pPWMModule),
                            mModuleConfig(pPWMModuleCfg),
                            mPWMPin(pPWMPin),
                            mPWMCfg(pPWMCfg),
                            mTorqueConst(pTorqueConst)
{
}

CMaxonMotor::~CMaxonMotor()
{
    if (mEnablePin != nullptr) {
        setTorque(0);
        mEnablePin->setLow();
    }
    setPWM(0);
}

CMaxonMotor::Status CMaxonMotor::init()
{
    CInterfaceManager<uint8_t, CGPIOMMAP> interfaceManagerGPIO;
    auto enGpio = interfaceManagerGPIO.getInstance(mEnableGPIONr, false);
    if (enGpio.has_value()) {
        mEnablePin = enGpio.value();
    }
    else {
        mEnablePin = nullptr;
        return Status::HARDWARE_ERROR;
    }
    if (mEnablePin->init(true) != CGPIOMMAP::Status::OKAY) {
        return Status::HARDWARE_ERROR;
    }
    mEnablePin->setLow();

    auto dirGpio = interfaceManagerGPIO.getInstance(mDirectionGPIONr, false);
    if (dirGpio.has_value()) {
        mDirectionPin = dirGpio.value();
    }
    else {
        mDirectionPin = nullptr;
        return Status::HARDWARE_ERROR;
    }
    if (mDirectionPin->init(true) != CGPIOMMAP::Status::OKAY) {
        return Status::HARDWARE_ERROR;
    }

    CInterfaceManager<uint8_t, CADCMMAP> interfaceManagerADC;
    auto adc = interfaceManagerADC.getInstance(mADCStepIdx, true);
    if (adc.has_value()) {
        mADC = adc.value();
    }
    else {
        mADC = nullptr;
        return Status::HARDWARE_ERROR;
    }
    if (mADC->init(mADCCfg) != CADCMMAP::Status::OKAY) {
        return Status::HARDWARE_ERROR;
    }

    CInterfaceManager<uint8_t, CPWMMMAP> interfaceManager;
    auto pwmModule = interfaceManager.getInstance(mPWMModule, true);
    if (pwmModule.has_value()) {
        mPWM = pwmModule.value();
    }
    else {
        return Status::PWM_MODULE_NOT_AVAILABLE;
    }
    if (mPWM->init(mPWMPin, mModuleConfig) != CPWMMMAP::Status::OKAY) {
        return Status::PWM_MODULE_NOT_AVAILABLE;
    }

    return Status::OKAY;
}

CMaxonMotor::Status CMaxonMotor::enable()
{
    if (mEnablePin == nullptr) {
        return Status::HARDWARE_ERROR;
    }
    if (setTorque(0) != Status::OKAY) {
        return Status::HARDWARE_ERROR;
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    mEnablePin->setHigh();
    return Status::OKAY;
}

CMaxonMotor::Status CMaxonMotor::disable()
{
    if (mEnablePin == nullptr) {
        return Status::HARDWARE_ERROR;
    }
    if (setTorque(0) != Status::OKAY) {
        return Status::HARDWARE_ERROR;
    }
    mEnablePin->setLow();
    return Status::OKAY;
}

CMaxonMotor:: Status CMaxonMotor::setTorque(double pTorque)
{
    if (pTorque >= 0) {
        mDirectionPin->setLow();
    }
    else {
        mDirectionPin->setHigh();
    }

    double torque = std::abs(pTorque);
    double current = std::clamp(torque/mTorqueConst, mPWMCfg.minTarget, mPWMCfg.maxTarget);
    double dutyCyclePercent = calculateDutyCyclePercent(current);
    return setPWM(dutyCyclePercent);
}

CMaxonMotor::Status CMaxonMotor::getRawVelocity(uint16_t& pRawVelocity)
{
    if (mADC->readADC(pRawVelocity) != CADCMMAP::Status::OKAY) {
        return Status::HARDWARE_ERROR;
    }
    return Status::OKAY;
}

double CMaxonMotor::calculateDutyCyclePercent(double pTarget)
{
    return (mPWMCfg.dutyCyclePercentMax - mPWMCfg.dutyCyclePercentMin) / (mPWMCfg.maxTarget - mPWMCfg.minTarget) * (pTarget - mPWMCfg.minTarget) + mPWMCfg.dutyCyclePercentMin;
}

CMaxonMotor::Status CMaxonMotor::setPWM(double pDutyCyclePercent)
{
    if (mPWM == nullptr) {
        return Status::PWM_MODULE_NOT_AVAILABLE;
    }
    if (mPWM->setDutyCycle(mPWMPin, pDutyCyclePercent) != CPWMMMAP::Status::OKAY) {
        return Status::PWM_MODULE_ERROR;
    }
    return Status::OKAY;
}
