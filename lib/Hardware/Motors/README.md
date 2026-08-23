# Motors/

`CMaxonMotor` is the torque-controlled motor driver used by [CBBBHardware](../README.md#entry-point-cbbbhardware). It composes three lower-level drivers: one [PWM](../PWM/README.md) output (speed/torque set-point to the motor driver), two [GPIO](../GPIO/README.md)s (enable + direction), and one [ADC](../ADC/README.md) step (current/velocity feedback).

## Usage

```cpp
CMaxonMotor motor(pwmModule, pwmPin, enableGpioNr, directionGpioNr,
                  pwmModuleCfg, pwmCfg, adcStepIdx, adcCfg, torqueConst);
motor.init();

motor.enable();
motor.setTorque(0.05);   // signed Nm
uint16_t raw;
motor.getRawVelocity(raw);
motor.disable();
```

`init()` internally obtains the two GPIOs (exclusive), the ADC step (shared), and the PWM module (shared) through `CInterfaceManager` — see [../README.md](../README.md) for that pattern. If any of them fails to acquire/initialize, `init()` returns `HARDWARE_ERROR`/`PWM_MODULE_NOT_AVAILABLE` without partially retrying.

## Constructor / config fields

| Parameter | Meaning |
|---|---|
| `pPWMModule` / `pPWMPin` | Which EPWM module/output drives the driver's PWM set-point input (see [PWM/README.md](../PWM/README.md)). |
| `pEnableGpioNr` / `pDirectionGpioNr` | Flat GPIO numbers (`module*32+pin`) for the driver's enable and direction inputs. |
| `pPWMModuleCfg` (`CPWMModuleConfig`) | Carrier frequency + polarity for the whole PWM module — see the module-sharing gotchas in [PWM/README.md](../PWM/README.md#heads-up) if this module's other pin is ever used elsewhere. |
| `pPWMCfg` (`PWMCfg`) | Maps a target current onto a duty cycle: `dutyCyclePercentMin/Max` is the output duty-cycle range, `minTarget/maxTarget` is the corresponding current range (A) it's linearly interpolated from. |
| `pADCStepIdx` / `pADCCfg` | Which ADC step (and its config) feeds `getRawVelocity()`. |
| `pTorqueConst` | Motor torque constant (Nm/A) used to convert a torque command into a target current. |

Current values (module 2 / pin 1 → P8.13, 50 Hz active-high; duty cycle 10-90% mapped to 0-2.0 A; torque constant 0.0369 Nm/A) are in [CubeConstants.hpp](../../../include/CubeConstants.hpp).

## Behavior notes

- `setTorque(t)`: sign of `t` sets the direction GPIO (positive = low, negative = high); magnitude is converted to a target current via `pTorqueConst`, clamped to `[minTarget, maxTarget]`, linearly mapped to a duty cycle via `PWMCfg`, then written to PWM.
- `enable()`: forces torque to 0 first, waits 200ms (lets the direction/torque command settle before the driver stage is armed), then sets the enable GPIO high.
- `disable()`: forces torque to 0, then sets the enable GPIO low — no settle delay on the way down.
- `getRawVelocity()` is a bit of a misnomer: despite the name, it just returns whatever the configured ADC step measures — in this project's wiring, that's motor current, not velocity directly (see [ADC/README.md](../ADC/README.md)).
- The destructor unconditionally zeroes torque and drives the enable pin low (if it was ever acquired) before releasing the PWM — a `CMaxonMotor` going out of scope always leaves the motor de-energized.
