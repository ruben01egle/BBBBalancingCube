# PWM/

`CPWMMMAP` drives the AM335x eHRPWM modules (EPWM0-2). It's a hybrid driver: it `mmap`s the module's registers directly for the time-critical duty-cycle write (`setDutyCycle`), but goes through the kernel's sysfs `pwm` framework for enabling the channel — see "Heads up" below for why both are needed. `CPWMModuleConfig` is the plain per-module config struct passed to it. Used by [CMaxonMotor](../Motors/README.md) for the motor's speed/torque set-point.

## Usage

```cpp
CInterfaceManager<uint8_t, CPWMMMAP> mgr;
auto pwm = mgr.getInstance(module /* 0, 1, or 2 */, /*allowMultipleInstances=*/true).value();

CPWMModuleConfig cfg{/*frequencyHz=*/50, /*activeHigh=*/true};
pwm->init(pin /* 0 = A, 1 = B */, cfg);

pwm->setDutyCycle(pin, 42.0 /* percent */);
```

Always go through `CInterfaceManager` (see [../README.md](../README.md)) rather than constructing `CPWMMMAP` directly — the constructor is private. Get it with `allowMultipleInstances=true` since both pins (A/B) of one module share the same time-base and must go through the same `CPWMMMAP` object.

## Module/pin addressing

- `module`: 0, 1, or 2 → EPWM0 (`0x48300000`), EPWM1 (`0x48302000`), EPWM2 (`0x48304000`).
- `pin` (per call, not per instance): 0 = output A, 1 = output B.

Current usage: module 2, pin 1 → EHRPWM2B → **P8.13** (50 Hz, active-high). See [README.md — PWM](../README.md#pwm) for the pin/overlay.

## `CPWMModuleConfig` fields

| Field | Meaning |
|---|---|
| `mFrequency` | Carrier frequency in Hz. Internally mapped to one of 4 fixed prescalers (1/2/4/32) chosen so the 16-bit period register doesn't overflow — supported range is **48 Hz and up** (below that, `init()`/`setFrequency()` returns `INVALID_FREQUENCY`). There's no explicit upper-bound check; very high frequencies just leave fewer ticks of duty-cycle resolution. |
| `mActiveHigh` | Output polarity. |

## Heads up

- **Only one pin's config actually takes effect for the whole module — and it's whichever `init()` call happens to run last.** `setHighLowActive()` (which programs `AQCTLA` *and* `AQCTLB`, i.e. both A and B) runs unconditionally on **every** `init()` call, regardless of which pin was requested — it isn't scoped to the pin argument. So if you ever drive both pins of one module from different consumers, they must agree on the same `mActiveHigh`, or whichever calls `init()` last silently overwrites the other pin's polarity.
- **Frequency is likewise only applied on the first `init()` call for a module** (`setFrequency()` runs inside the `!moduleAlreadyInitialized` branch only). A second pin's `init()` on an already-initialized module keeps the first pin's frequency — a different `mFrequency` passed on the second call is silently ignored, not an error.
- Since this project only ever drives module 2 / pin 1 (P8.13), neither of the above currently bites — but be aware of it before wiring up P8.19 (EHRPWM2A) or any other module's second pin.
- **Two-stage enable, both required:** enabling `CM_PER`'s EPWMSS clock gate only ungates *register* access to the module — it does **not** ungate the internal time-base counter clock. Without also exporting/enabling the channel through sysfs (`/sys/class/pwm/pwmchipN/pwmM/enable`), `TBCNT` never advances and a raw `CMPA`/`CMPB` register write never loads from its shadow register into the active one, i.e. `setDutyCycle()` would silently have no effect. `init()` does both steps already; don't try to skip the sysfs export as an "optimization".
- **sysfs `pwmchipN` index ≠ module number.** The kernel assigns `pwmchipN` by probe order, not by physical module. `resolvePwmChipIndex()` matches on the module's physical base address (via the `/sys/class/pwm/pwmchipN` symlink target) instead of assuming `pwmchip2` is EPWM2. If the relevant overlay isn't loaded, this resolves to nothing and `init()` fails with `DRIVER_ERROR` — see [README.md — PWM Overlay](../README.md#pwm-overlay).
- Period is written before `duty_cycle`/`enable` deliberately — the kernel PWM core rejects enabling a channel whose period is still zero.

## `test.sh`

A standalone debug helper (not part of the app) that live-prints `TBPRD`/`CMPA`/`CMPB` and the resulting duty cycle for **EPWM2** (`0x48304000`, hardcoded) by polling `/dev/mem` directly with a `sudo python3` one-liner. Useful for confirming the motor's actual PWM output independent of what the C++ side thinks it programmed. Run with `./test.sh`, Ctrl+C to stop; edit the hardcoded offset/address if you need to watch a different module.
