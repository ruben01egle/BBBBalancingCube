# Hardware/

Low-level, register/mmap-based drivers for the physical peripherals of the BeagleBone Black (BBB), plus the sensor/actuator classes built on top of them.

```
Hardware/
├── CHardware.hpp/.cpp        # CBBBHardware — entry point, aggregates everything below
├── CInterfaceManager.hpp     # generic singleton/registry used by every driver below
├── ADC/                      # CADCMMAP — AM335x ADC_TSC register driver
├── GPIO/                     # CGPIOMMAP — AM335x GPIO register driver
├── PWM/                      # CPWMMMAP — AM335x EPWM register + sysfs driver
├── SPI/                      # CSPIModuleMMAP — AM335x McSPI register driver
├── IMU/                      # CMPU9250 — MPU-9250 IMU driver (built on SPI/)
├── Motors/                   # CMaxonMotor — motor driver (built on PWM/ + GPIO/ + ADC/)
├── BBB_IMAGE_README.md       # BBB OS image / kernel setup
└── util/                     # overlay sources & uEnv.txt referenced further down in this file
```

For flashing/setting up the BBB image itself, see [BBB_IMAGE_README.md](BBB_IMAGE_README.md). Each subfolder above has its own `README.md` covering the class(es) it contains, their configuration structs, and known gotchas — read those before wiring up a new pin/peripheral. Pin assignments and device tree overlay setup are covered further down in this file.

## Entry point: `CBBBHardware`

[CHardware.hpp](CHardware.hpp) / [CHardware.cpp](CHardware.cpp) define `CBBBHardware`, the single object the rest of the application talks to. It owns two `CMPU9250` sensors and one `CMaxonMotor`, wires them up from the pin/module constants in [`CubeConstants.hpp`](../../include/CubeConstants.hpp), and exposes:

| Function | Description |
|----------|-------------|
| `init()` | Initializes both IMUs and the motor. Returns `false` (and aborts) if any of them fails. |
| `fetchValues(adcValue, sensor1Data, sensor2Data)` | Reads the motor's raw ADC value and both IMUs' raw accel/gyro data in one call. |
| `enableMotor()` / `disableMotor()` | Arms/disarms the motor driver (see [Motors/README.md](Motors/README.md) for what this does electrically). |
| `setTorque(torque)` | Commands a torque set-point. |

See [lib/README.md](../README.md) for how this fits into the wider `lib/` framework.

## The `CInterfaceManager` pattern — read this first

Every driver in `ADC/`, `GPIO/`, `PWM/`, and `SPI/` has a **private constructor** and declares `CInterfaceManager<uint8_t, TheClass>` as a `friend`. You never construct these directly — you obtain them through:

```cpp
CInterfaceManager<uint8_t, CGPIOMMAP> mgr;
auto result = mgr.getInstance(pinNumber, /*allowMultipleInstances=*/false);
if (result.has_value()) {
    std::shared_ptr<CGPIOMMAP> pin = result.value();
}
```

`getInstance` is keyed by an arbitrary `uint8_t` identifier (a pin number, module index, ADC step index, ...) and returns a `shared_ptr` shared by every caller that asks for the same id — this is what lets two `CMPU9250` instances share one physical `CSPIModuleMMAP`, or two PWM pins on the same module share one `CPWMMMAP`.

> ⚠️ The `allowMultipleInstances` flag is **sticky to whichever call creates the entry first**

Existing call sites for reference:
- `CMPU9250` → `getInstance(spiModule, true)` (both IMUs share SPI1)
- `CMaxonMotor` → GPIOs with `false` (exclusive), ADC step and PWM module with `true` (shared)

## Hardware Management on the BeagleBone Black

This project uses direct access to hardware peripherals on the BBB. To make these peripherals available to Linux, the Device Tree needs to be configured accordingly via U-Boot overlays.

### Pin Configuration

#### SPI (Sensors)

| PIN     | USAGE                       |
|---------|------------------------------|
| P.9.31  | SPI1_SCLK (shared)           |
| P.9.30  | SPI1_MOSI (shared)           |
| P.9.29  | SPI1_MISO (shared)           |
| P.9.20  | SPI1_CS0 (alt, default is P9.28) |
| P.9.19  | SPI1_CS1                     |

#### GPIO

| PIN     | GPIO NR | USAGE                        |
|---------|---------|-------------------------------|
| P8.7    | gpio66 (GPIO2_2, mode 7)  | Motor enable (`MOTOR_ENABLE_GPIO`, [CubeConstants.hpp](../../include/CubeConstants.hpp)) |
| P8.8    | gpio67 (GPIO2_3, mode 7)  | Motor direction (`MOTOR_DIRECTION_GPIO`, [CubeConstants.hpp](../../include/CubeConstants.hpp)) |

`gpio_nr = module*32 + pin` (see [GPIO/README.md](GPIO/README.md)). Both pins are plain digital I/O muxed with `MUX_MODE7`, driven low at idle per the source `.bbio`.

#### ADC

| PIN     | CHANNEL | USAGE                        |
|---------|---------|-------------------------------|
| P9.39   | AIN0    | Motor current/velocity sense (`ADC_STEP_IDX`/`ADC_CFG`, [CubeConstants.hpp](../../include/CubeConstants.hpp)) |

Only one ADC channel is used by this project. AIN0 is fed into ADC step 1 (steps are numbered 1-16, see [ADC/README.md](ADC/README.md)).

#### PWM

| PIN     | SIGNAL       | USAGE                        |
|---------|--------------|-------------------------------|
| P8.13   | EHRPWM2B (mode 4) | Motor speed/torque set-point (`MOTOR_PWM_MODULE`=2, `MOTOR_PWM_PIN`=1, [CubeConstants.hpp](../../include/CubeConstants.hpp)) |
| P8.19   | EHRPWM2A (mode 4) | Enabled by the same overlay/module, currently unused |

### BBB Device Tree Setup

U-Boot overlays are Device-Tree-Overlays that modify hardware configuration at boot without requiring kernel recompilation. They enable features like GPIOs, UARTs, I2C, and SPI dynamically.

#### Boot Order

1. U-Boot loads the base board `.dtb` (e.g. `am335x-boneblack.dtb`). This already enables several peripherals by default (UART, MMC, USB, HDMI, and I2C2 for cape EEPROM detection on P9.19/P9.20).
2. U-Boot then reads `/boot/uEnv.txt` and merges in whatever `.dtbo` overlays are listed there.
3. Overlays only ever *modify* the tree from step 1 — if a pin is already claimed by an enabled node in the base tree (e.g. I2C2 on P9.19/P9.20), your overlay must explicitly disable that node, otherwise the merge fails with a `pinctrl-single: pin already requested` error at boot.

#### Using U-Boot Overlays

Edit `/boot/uEnv.txt` (project copy: [util/uEnv.txt](util/uEnv.txt)). Depending on the image, overlays are loaded either via the multi-slot syntax:

```
uboot_overlay_addr0=/lib/firmware/<overlay0>.dtbo
uboot_overlay_addr1=/lib/firmware/<overlay1>.dtbo
...
uboot_overlay_addr7=/lib/firmware/<overlay7>.dtbo
```

or, on newer images, via the single custom-cape slot:

```
dtb_overlay=/lib/firmware/<overlay>.dtbo
```

Check which style your `uEnv.txt` already uses (look for a `###Custom Cape` section) before adding a new line — mixing both isn't necessary.

Also make sure `enable_uboot_overlays=1` is uncommented, and disable unneeded default virtual capes (audio/video/wireless) if not required, to avoid pin conflicts:

```
disable_uboot_overlay_video=1
disable_uboot_overlay_audio=1
disable_uboot_overlay_wireless=1
```

Reboot to apply changes.

#### Writing/Editing a Custom Overlay

Write overlays using `/plugin/;` syntax with symbolic node labels (`&spi1`, `&i2c2`, `&am33xx_pinmux`, ...) rather than hand-editing a decompiled overlay's raw phandles. With symbolic labels, `dtc` auto-generates the `__fixups__`/`__symbols__`/`__local_fixups__` blocks at compile time — new fragments can be added freely without manually maintaining those tables.

Pin mux entries use the 3-cell format `<offset conf mux>`:

- `offset` — pin control register offset (see e.g. the [BeagleBone Black P9 header table](https://itbrainpower.net/a-gsm/images/BeagleboneBlackP9HeaderTable.pdf))
- `conf` — pull/input flags (e.g. `0x30` = `INPUT_PULLUP`, `0x10` = `OUTPUT_PULLUP`)
- `mux` — mode number (0–7) selecting which peripheral function is routed to that pin

Compile/decompile with:

```bash
dtc -I dtb -O dts -o output.dts input.dtbo   # decompile to inspect
dtc -@ -O dtb -o output.dtbo input.dts       # compile (symbols/fixups auto-generated)
```

Copy the resulting `.dtbo` to `/lib/firmware/` and reference it from `uEnv.txt` as described above, then reboot and verify with `dmesg | grep -i -E "spi|pinctrl-single"`.

Always start from the stock overlay shipped with the kernel as the reference/base rather than an old copy — found under `/boot/dtbs/$(uname -r)/overlays/` (e.g. `/boot/dtbs/$(uname -r)/overlays/BB-SPIDEV1-00A0.dtbo`). Decompile that with `dtc -I dtb -O dts` and diff against your custom version to catch upstream changes after a kernel update.

### SPI1 Overlay (Custom CS Pins)

```
SCLK:          P9_31
MISO/D0:       P9_29
MOSI/D1:       P9_30
CS-Channel0:   P9_20  (alt — default would be P9_28)
CS-Channel1:   P9_19
```

P9.19/P9.20 default to I2C2 (used for cape EEPROM detection), so the overlay must explicitly disable `&i2c2` to free those pins for SPI1 — otherwise pin mux application fails with `pin already requested by ...i2c`. Disabling I2C2 removes `/dev/i2c-2`; the cape EEPROM is only read once by U-Boot at early boot, so this is safe for normal operation but worth remembering if any other I2C2 peripheral is ever added.

CS0 was moved off the stock overlay's default (P9.28, offset `0x19c`, mode 3) onto P9.20 (offset `0x17c`, mode 4) so P9.28 stays free for other use, and CS1 was added on P9.19 (offset `0x178`, mode 4) since the stock overlay only defines a single CS.

Overlay source: [util/BB-SPIDEV1-00A0-CSMOD.dts](util/BB-SPIDEV1-00A0-CSMOD.dts)

Build and install:

```bash
dtc -@ -O dtb -o BB-SPIDEV1-00A0-CSMOD.dtbo BB-SPIDEV1-00A0-CSMOD.dts
sudo cp BB-SPIDEV1-00A0-CSMOD.dtbo /lib/firmware/
```

Reference it from [util/uEnv.txt](util/uEnv.txt) (`dtb_overlay=` line, see the "Using U-Boot Overlays" section above), then copy that file to `/boot/uEnv.txt` on the board.

Reboot, then verify:

```bash
dmesg | grep -i -E "spi|pinctrl-single"
ls /dev/spidev1.*
```

Expected: no `pinctrl-single`/`omap2_mcspi` errors, and both `/dev/spidev1.0` and `/dev/spidev1.1` present.

> ⚠️ **Important:** If using multiple SPI devices with non-standard CS polarity (active high), call the channel configuration function for **all** devices before starting communication. CS is idle-high by default when unconfigured, which can cause bus collisions if devices expect opposite polarity before they've been configured.

### GPIO Overlay

P8.7/P8.8 (used for motor enable/direction) just need to be muxed as plain GPIO (`MUX_MODE7`), no peripheral controller node is involved:

```
0x090 0x27 0x07   /* P8_7  gpmc_advn_ale.gpio2_2, MUX_MODE7 | INPUT_PULLDOWN (0x27) or OUTPUT (0x07) */
0x094 0x27 0x07   /* P8_8  gpmc_oen_ren.gpio2_3,  same */
```

On the stock `am335x-boneblack.dtb`, pins that aren't claimed by another onboard peripheral (like these two) are already left in `MUX_MODE7`/GPIO by default — so in practice **no dedicated overlay is required** to drive P8.7/P8.8 as GPIO with this project's default image. Only add a fragment like the one above if some other overlay/cape you load later re-muxes these two pins for something else, and you need to explicitly reclaim them.

### ADC Overlay

The `ADC_TSC` peripheral (`0x44E0D000`, register block backing [CADCMMAP](ADC/README.md)) needs its module clock/pm_runtime state enabled before use. Unlike the GPIO/PWM/SPI classes in this codebase — which each poke `CM_PER`/`CM_WKUP` clock-control registers themselves during `init()` — **`CADCMMAP` does not enable the ADC module clock**. It only resets/configures registers, assuming the module is already powered.

That means an overlay (or base image) that sets the `tscadc`/`adc` node's `status = "okay"` must be loaded first — e.g. the standard `BB-ADC-00A0` overlay, or `am335x-adc` already brought up by the base board file on stock BeagleBoard.org images (check with `cat /sys/bus/iio/devices/iio:device0/name` → should print `TI-am335x-adc`, or `dmesg | grep -i tscadc`). If that device never shows up, `CADCMMAP::init()` will still "succeed" (no clock-gating check happens) but reads will hang or silently return stale/zero data, since the underlying module clock is never actually running. If AIN0 stops updating, check this first.

No AIN pin needs its own pinmux fragment — the ADC pins are analog-only and always routed to the ADC_TSC block; there's no `MUX_MODE` selection for them.

### PWM Overlay

P8.13 needs muxing to its PWM alternate function plus the `epwmss2`/`ehrpwm2` controller nodes enabled:

```
0x048 0x0C 0x04   /* P8_13 gpmc_ad6.ehrpwm2B, MODE4, OUTPUT (0x0C) */
```
with `status = "okay"` on `&epwmss2` / `&ehrpwm2` (P8.19/EHRPWM2A gets the analogous fragment at pinctrl offset `0x020`).

This project doesn't ship a custom `.dts` for this one (unlike the SPI1 CS remap) — `uEnv.txt` ([util/uEnv.txt](util/uEnv.txt)) instead references the stock overlay that ships with the kernel/firmware package directly:

```
uboot_overlay_addr0=/lib/firmware/BB-EHRPWM2-P8_13-P8_19.dtbo
```

Verify after boot with:

```bash
ls /sys/class/pwm/           # one pwmchipN per enabled EPWM module
cat /sys/class/pwm/pwmchip*/npwm
dmesg | grep -i -E "ehrpwm|pwmss"
```

> ⚠️ **Heads up:** the sysfs `pwmchipN` index is assigned by the kernel in probe order, **not** by physical module number — [CPWMMMAP](PWM/README.md) resolves the right chip by matching the module's physical base address (`0x483040xx` for EPWM2) rather than assuming `pwmchip2` is EPWM2. If this overlay isn't loaded (or fails to bind), `resolvePwmChipIndex()` finds nothing and `init()` fails with `DRIVER_ERROR` — check `dmesg` for `pinctrl-single: pin already requested` first (usually a conflicting cape/overlay claiming the same pin).
