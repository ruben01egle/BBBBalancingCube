# Changelog: `main` → `rt_kernel`

Summary of the conceptual and behavioral changes on `rt_kernel` (12 commits since `main`, from "working spi" through "more cleanup and renaming, untested"). This branch is a from-scratch rewrite of the hardware layer plus a project-wide error-handling hardening pass. **Status: untested on hardware** (per the last commit message) — treat as a WIP integration branch, not a validated release.

## 1. Hardware layer: full rewrite

The old flat, ad-hoc hardware classes are gone:

- `CBBBHardware`, `CGPIO`, `CPWM`, `CADCMMap`, `CMPU9250` (old), `CMotor` → replaced by a peripheral-organized driver stack under `lib/Hardware/`:
  - `ADC/CADCMMAP`, `GPIO/CGPIOMMAP`, `PWM/CPWMMMAP`, `SPI/CSPIModuleMMAP` — register/mmap-level drivers, one per AM335x peripheral.
  - `IMU/CMPU9250` (rewritten) — built on top of `SPI/`.
  - `Motors/CMaxonMotor` (rewritten) — composes `PWM/` + `GPIO/` + `ADC/`.
  - `CHardware` — new single entry point (replaces `CBBBHardware`), owns two IMUs + one motor, exposes `init()`, `fetchValues()`, `enableMotor()`/`disableMotor()`, `setTorque()`.
- Every driver/module folder now ships its own `README.md` (ADC, GPIO, PWM, SPI, IMU, Motors) plus a top-level `lib/Hardware/README.md` covering pin assignments, device-tree overlay setup, and a new `BBB_IMAGE_README.md` for BBB OS/kernel image setup.

**New shared-ownership pattern:** `CInterfaceManager<IdType, DriverType>` — a generic keyed singleton registry. `ADC`/`GPIO`/`PWM`/`SPI` drivers now have **private constructors** and are only obtainable via `CInterfaceManager::getInstance(id, allowMultipleInstances)`, keyed by pin/module/step number. This is what lets two `CMPU9250` instances safely share one physical SPI module, or two motor GPIOs stay exclusive. Note: the `allowMultipleInstances` flag is sticky to whichever caller creates the entry first.

**Behavioral changes, not just reshuffling:**
- All hardware `init()`/read/write paths now return real status codes instead of silently failing — a bad WHO_AM_I check, a failed clock/mux setup, a timed-out SPI transfer, etc. all surface as a typed `Status` rather than producing zeroed or stale data silently.
- IMU reads (`CMPU9250::readImu`) now **retry up to 3 times** on a failed SPI burst before giving up, logging each attempt — recovers from transient SPI hiccups instead of failing the whole control loop on one bad transaction.
- New device-tree overlay for remapped SPI1 chip-selects (`lib/Hardware/util/BB-SPIDEV1-00A0-CSMOD.dts`) so both IMUs can share SPI1 on distinct CS lines (P9.20/P9.19), documented in `lib/Hardware/README.md`.

### 1a. The bigger point: sysfs/kernel-driver access → direct register mmap

The core architectural shift, peripheral by peripheral (old code read via `git show main:...`, new code read from the current tree):

**GPIO** (`CGPIOMMAP`)
- Old `CGPIO`: pure sysfs. Constructor wrote the pin number to `/sys/class/gpio/export`, then `open()`'d `/sys/class/gpio/gpioNN/direction` and `.../value` and kept the value fd open; every `setHigh()`/`setLow()` was a `write()` syscall of a single ASCII `"0"`/`"1"` byte through the kernel gpiolib.
- New `CGPIOMMAP`: `mmap()`s the GPIO bank's physical register block directly (`GPIO0`=`0x44E07000`, `GPIO1`=`0x4804C000`, `GPIO2`=`0x481AC000`, `GPIO3`=`0x481AE000`) and toggles pins with direct `GPIO_OE`/`GPIO_DATAOUT` register writes, reading level back from `GPIO_DATAIN`. It also has to enable the bank's clock gate itself first — module 0 hangs off `CM_WKUP` (`0x44E00400`) while modules 1-3 hang off `CM_PER` (`0x44E00000`), a distinction the old sysfs path never had to care about (the kernel driver handled it).
- Net effect: motor enable/direction toggles are now a couple of memory writes instead of a `write()` syscall through gpiolib — removes syscall/scheduling jitter from the hot path, at the cost of the driver now being responsible for clock-gating correctness itself.

**PWM** (`CPWMMMAP`)
- Old `CPWM`: 100% sysfs, and the constructor took **raw, manually-discovered index strings** — `CMotor` was constructed with literal `"7"`/`"1"` as the pwmchip/pwm numbers (`CBBBHardware.cpp`: `mMotor{"7", "1", "66", "67", ...}`). Those sysfs indices are assigned by the kernel in **probe order**, not by physical module — so `"7"` only meant EPWM2 on the specific image/boot the author tested against; a kernel update, a different overlay load order, or another cape could silently renumber `pwmchipN` and point the motor driver at the wrong (or a nonexistent) PWM chip with no error. The period was also a **hardcoded 2,000,000 ns** (500 Hz) written once via sysfs — never configurable. Every `setDutyCycle()` call was an `open()`+`write()`+(implicit close via kept fd) of an ASCII number to `.../duty_cycle`.
- New `CPWMMMAP`: resolves the module by its **physical base address** instead of trusting sysfs numbering — `resolvePwmChipIndex()` walks `/sys/class/pwm/pwmchipN` symlinks and matches whichever one's target encodes EPWM0/1/2's real address (`0x48300000`/`0x48302000`/`0x48304000`), so it can't be pointed at the wrong chip by boot-order changes. The frequency is now a config value (`CPWMModuleConfig::mFrequency`) mapped to one of 4 hardware prescalers — **the motor's actual carrier frequency changed from ~500 Hz (old, hardcoded) to 50 Hz (new, `CubeConstants::MOTOR_PWM_MODULE_CFG`)**, a real electrical behavior change for the motor driver stage, not just a refactor. `setDutyCycle()` itself is now a direct write to the `CMPA`/`CMPB` compare register via `mmap`, removing the per-tick sysfs round-trip from the control loop.
- It's a **deliberate hybrid**, not a pure mmap win: the sysfs `enable` file still has to be exported/written once during `init()`, because ungating the EPWMSS clock via `CM_PER` only unblocks *register access* — it does not start the internal `TBCNT` time-base counter, so a pure-register duty-cycle write would silently do nothing without the one-time sysfs enable. This is called out explicitly in `PWM/README.md` so nobody "optimizes" that step away later.

**ADC** (`CADCMMAP`)
- Old `CADCMMap` was already mmap-based (`0x44E0D000`, same as new), but hardcoded to fire **3 ADC steps at once** (steps 1-3, all pre-configured identically) and busy-loop until 3 words landed in FIFO1 — then it kept only the *first* word and threw the other two away. Effectively: manual, undocumented 3x oversampling via redundant steps, discarding 2/3 of the conversions, with no way to pick a different channel/averaging without touching the class.
- New `CADCMMAP` is instantiated per **ADC step index (1-16)** via `CInterfaceManager`, with a `CADCConfig` (channel, `ONESHOT`/`CONTINUOUS` mode, FIFO select, hardware averaging `NO_AVG`..`AVG_16_SAM`, sample/open delay) — hardware-level oversampling replaces the old "fire 3 steps, drop 2" trick, and the channel/step mapping is explicit and configurable instead of implicit in the constructor body. Still no read timeout in `ONESHOT` mode (documented as a known gap in `ADC/README.md`), and the module clock still isn't gated by this class in either version — that always came from the device tree/base image.

**SPI / IMU** (`CSPIModuleMMAP`, `CMPU9250`)
- Old `CMPU9250` didn't own an SPI driver at all — it opened the Linux kernel's **spidev character device directly** (`/dev/spidev2.0` / `/dev/spidev2.1`, i.e. **SPI bus 2**), configured mode/speed via `ioctl(SPI_IOC_WR_MODE/...)`, and did each register burst-read as a spidev `ioctl` transfer with the register address re-sent as a padding byte ahead of every data byte in the tx buffer.
- New `CSPIModuleMMAP` bypasses spidev/the kernel SPI driver entirely and talks to the **McSPI0/1 register block** directly (`0x48030000`/`0x481A0000`, plus the module's `CM_PER` clock-gate registers at `0x44E00000`), with its own from-scratch clock-divider, CS-timing, and word-length configuration, and a **software transfer timeout** derived from `wordLength`/`sclk_Frequency_Hz` (20x headroom, floored at 1 ms) instead of relying on the kernel driver's own timeout handling.
- The bus assignment itself changed, not just the access method: IMUs moved from **SPI2** (stock spidev nodes, no custom overlay needed) to **SPI1** with a **new custom device-tree overlay** (`util/BB-SPIDEV1-00A0-CSMOD.dts`) that remaps CS0 off its stock pin (P9.28 → P9.20) and adds a second CS1 (P9.19) so both IMUs can share one physical McSPI module on distinct chip-selects — this requires disabling the base image's I2C2 node (which defaults onto P9.19/P9.20 for cape EEPROM detection), documented with the exact failure mode (`pinctrl-single: pin already requested`) in `lib/Hardware/README.md`.
- Read framing also changed: the old code addressed every one of the 14 payload bytes individually in the tx buffer (address+0x80 repeated per byte); the new `burstRead()` sends a single start address with the `READ` bit and lets the MPU-9250 auto-increment through all 14 registers in one contiguous SPI transaction — fewer bytes on the wire, and the retry-on-failure logic (see below) wraps this single transaction instead of a chain of ioctls.

**Motor** (`CMaxonMotor`)
- Old `CMotor` was a thin, non-defensive wrapper: fixed 10-90% duty-cycle mapping over a `sMaxCurrent`/`sMaxTorque` hardcoded at construction, `void`-returning `enableMotor()`/`setTorque()` (no way to detect a failure), and no coupling to the ADC read at all (that lived separately in `CBBBHardware::fetchValues`, keyed to nothing in particular).
- New `CMaxonMotor` composes its own GPIO(x2)/PWM/ADC instances (acquired through `CInterfaceManager`, GPIOs exclusive, PWM/ADC shared), exposes a typed `Status` from every call, takes an explicit `PWMCfg` (duty-cycle range ↔ target-current range, linearly interpolated) instead of a hardcoded 10-90%/`sMaxTorque` split, and has real safety-adjacent behavior: `enable()` forces torque to 0 and waits 200 ms before arming the driver stage (lets direction/torque settle first), and the destructor **unconditionally** zeroes torque and drives the enable pin low before releasing the PWM handle — a `CMaxonMotor` going out of scope always leaves the motor de-energized, which wasn't guaranteed before.
- One old debugging artifact worth knowing is gone: `CBBBHardware::fetchValues` had a commented-out hardcoded fallback (`adcValue = 2035; // to do`) that used to stand in for a real ADC read during bring-up — the new `CHardware::fetchValues` has no such escape hatch; an ADC failure now propagates as `false` all the way to `CControlComp`.

## 2. Error handling: no more silent failure / hard exit

- `Assertion.h/.cpp` and `Global.h` (old `UInt8`/`Int32`-style typedefs, `sAssertion()` which printed to `stderr` and called `exit(-1)` on failure) are **deleted outright**. `sAssertion` used to be called from inside nearly every hardware constructor — e.g. every `open()`/`write()` in old `CGPIO`/`CPWM`'s sysfs export dance, every `ioctl()` in old `CMPU9250`'s spidev setup — so a transient sysfs hiccup or a device not yet enumerated at boot (a real race: the app could start before udev finished creating `/sys/class/gpio/exportNN`) killed the **entire process** on the spot, with no chance to retry or degrade. None of the new mmap-based drivers call `exit()` anywhere; every failure path returns a `Status`/`bool` instead.
- `ErrorReporter` renamed to `CErrorReporter` (to match project naming convention) and is now used pervasively across hardware, threading, communication, and control code via `REPORT_ERROR`/`REPORT_ERROR_ERRNO`.
- `CThread::start()` now **returns `bool`** instead of calling `exit(-1)` when `pthread_create()` fails — callers (`main.cpp`) check the result and report the error instead of the whole process dying immediately. `join()` now guards against joining a thread that never started.
- Fixed a bug in the real-time priority calculation in `CThread::start()` (division order was wrong) and clamped the computed priority into `[min, max]` of `SCHED_RR`.
- `CControlComp::run()` now checks `mHardware.fetchValues()` and `mHardware.setTorque()` return values every loop iteration — a failure logs an error and cleanly stops the control loop instead of continuing to run with garbage/stale sensor data or silently dropping torque commands.
- `CServer::init()` now actually returns `false` when `socket()` fails (previously it logged and fell through as if nothing happened); `transmitMessage()` now aborts the send loop on the first error instead of reporting success regardless.

Net effect: a hardware or thread-start fault now degrades to a **reported, controlled stop** instead of either an unconditional `exit()` or (worse) silently continuing with bad data — worth keeping in mind when triaging a "the cube stopped" report vs. a crash.

## 3. Calibration

- Per-cube calibration constants moved into `CubeConstants.hpp` as a static table `ALL_CUBES[14]` (one entry per physical cube unit, indexed by a `CUBE` constant) — IMU accel/gyro scale+offset and ADC scale+offset are now baked in per-device rather than a single global default.
- `CCalibration` renamed from `.h`→`.hpp`, minor cleanup only in this branch's committed history.
- *(Not yet committed, currently sitting as uncommitted WIP in the working tree: a new `CCalibration`/`CCalibrationData`/`CIMUDataCalibrated` runtime calibration path feeding into `CControlComp`, plus matching GUI plotting changes in `gui/plot/imu_calib_plot.py`. Not part of `rt_kernel`'s committed history yet — flagging so it isn't mistaken for finished/tested work.)*

## 4. Naming/build/deploy cleanup

- Broad `.h` → `.hpp` rename across headers, switched from `#ifndef` include guards to `#pragma once` throughout.
- App binary name is now a **fixed** `balancing_cube_app` (makefile, `.vscode/launch.json`) instead of being derived from the checkout's directory basename — avoids the binary name changing if someone clones the repo under a different folder name.
- `manage_cube.sh` / `README.md` updated for a new BBB image: SSH login user changed from `root` to `debian`, home dir path updated, and the remote debugger launch now goes through `sudo gdbserver` (since the login user is no longer root).
- Removed obsolete `START.sh`/`STOP.sh` (raw `/sys/class/gpio/gpio66` sysfs toggle scripts) — superseded by the new `GPIO/CGPIOMMAP` driver.

## What this means for onboarding / instructions

- Anyone with local docs/notes referencing `CBBBHardware`, `CGPIO`, `CPWM`, `CADCMMap`, `CMotor`, or the old `.h` header names needs to update them to the new `Hardware/<Peripheral>/` layout and `.hpp` names.
- SSH/deploy instructions referencing `root@` login need to switch to `debian@` (+ `sudo` for anything privileged like `gdbserver`).
- Anything driving GPIO66/67 via raw sysfs (`START.sh`/`STOP.sh` style) should go through `CGPIOMMAP`/`CHardware` instead — the sysfs scripts are gone.
- **Any bring-up instructions that reference `/dev/spidev2.x` for the IMUs are now wrong** — the IMUs live on **SPI1**, not SPI2, and require flashing the new `BB-SPIDEV1-00A0-CSMOD.dtbo` overlay (built from `util/BB-SPIDEV1-00A0-CSMOD.dts`) via `uEnv.txt`, which also disables I2C2 on P9.19/P9.20. A board still set up for the old SPI2 wiring will need this overlay applied before the new code can talk to the IMUs at all.
- **Any tuning notes/setpoints derived from the old ~500 Hz motor PWM carrier no longer apply** — the new driver runs the motor PWM at 50 Hz (`CubeConstants::MOTOR_PWM_MODULE_CFG`). If the motor driver stage's behavior (audible whine, current ripple, response) was tuned around the old frequency, re-validate it against 50 Hz.
- Anything that used to hardcode a `pwmchipN` sysfs index (as the old `CMotor` constructor did with literal `"7"`/`"1"`) should be considered fragile/wrong going forward — the new driver resolves the chip by physical address specifically because that sysfs numbering isn't stable across boots/images.
- Before wiring up any new pin/peripheral, read the relevant `lib/Hardware/<X>/README.md` first — each documents the config struct fields and known gotchas (e.g. SPI CS idle-polarity ordering, ADC module-clock prerequisites, PWM chip-index resolution by physical address).
- This branch is explicitly flagged **untested** as of the last two commits — don't treat it as a drop-in replacement for `main` without a hardware validation pass.
