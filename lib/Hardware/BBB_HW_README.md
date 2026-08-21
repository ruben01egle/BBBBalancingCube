# Hardware Management in the BeagleBone Black Project

## Overview

This project uses direct access to hardware peripherals on the BeagleBone Black (BBB). To make these peripherals available to Linux, the Device Tree needs to be configured accordingly via U-Boot overlays.

## BeagleBone Black Pin Configuration

### Power

| PIN     | USAGE                        |
|---------|-------------------------------|
| P.9.1   | GNDD                          |
| P.9.2   | GNDD                          |
| P.9.3   | +3.3V powered by BBB          |
| P.9.4   | +3.3V powered by BBB          |
| P.9.5   | +5V power source BBB          |
| P.9.6   | +5V power source BBB          |

### SPI (Sensors)

| PIN     | USAGE                       |
|---------|------------------------------|
| P.9.31  | SPI1_SCLK (shared)           |
| P.9.30  | SPI1_MOSI (shared)           |
| P.9.29  | SPI1_MISO (shared)           |
| P.9.20  | SPI1_CS0 (alt, default is P9.28) |
| P.9.19  | SPI1_CS1                     |

### GPIO

| PIN     | USAGE                       |
|---------|------------------------------|
| TODO    | TODO                          |
| TODO    | TODO                          |

### ADC

| PIN     | USAGE                       |
|---------|------------------------------|
| TODO    | TODO                          |
| TODO    | TODO                          |

### PWM

| PIN     | USAGE                       |
|---------|------------------------------|
| TODO    | TODO                          |
| TODO    | TODO                          |

## BBB Device Tree Setup

U-Boot overlays are Device-Tree-Overlays that modify hardware configuration at boot without requiring kernel recompilation. They enable features like GPIOs, UARTs, I2C, and SPI dynamically.

### Boot Order

1. U-Boot loads the base board `.dtb` (e.g. `am335x-boneblack.dtb`). This already enables several peripherals by default (UART, MMC, USB, HDMI, and I2C2 for cape EEPROM detection on P9.19/P9.20).
2. U-Boot then reads `/boot/uEnv.txt` and merges in whatever `.dtbo` overlays are listed there.
3. Overlays only ever *modify* the tree from step 1 — if a pin is already claimed by an enabled node in the base tree (e.g. I2C2 on P9.19/P9.20), your overlay must explicitly disable that node, otherwise the merge fails with a `pinctrl-single: pin already requested` error at boot.

### Using U-Boot Overlays

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

### Writing/Editing a Custom Overlay

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

## SPI1 Overlay (Custom CS Pins)

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

## GPIO Overlay

TODO — placeholder for custom GPIO overlay/config once pin assignments are finalized.

## ADC Overlay

TODO — placeholder for ADC (`BB-ADC-00A0` or custom) overlay/config once channels are finalized.

## PWM Overlay

TODO — placeholder for PWM chip overlay/config once pin assignments are finalized.