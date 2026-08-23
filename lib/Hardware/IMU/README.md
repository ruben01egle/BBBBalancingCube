# IMU/

`CMPU9250` is a SPI driver for the InvenSense MPU-9250 6-axis IMU (accelerometer + gyroscope; the onboard AK8963 magnetometer is not used — the auxiliary I2C master is left disabled). Built on top of [SPI/](../SPI/README.md); it internally obtains/shares a `CSPIModuleMMAP` instance itself, so callers don't need to set up SPI separately.

## Usage

```cpp
CMPU9250 imu(spiModule, spiChannel, spiChannelCfg);   // public constructor — takes SPI settings, not the SPI object

CMPU9250::CMPU9250Setup setup{
    .gyroScale = CMPU9250::FS_RANGE_2000,
    .accelScale = CMPU9250::FS_RANGE_4g,
    .gyroLP = CMPU9250::G_LP_20HZ,
    .accelLP = CMPU9250::A_LP_20HZ,
};
imu.initImu(setup);

CMPU9250::rawData data;
imu.readImu(data);   // raw int16 counts, not yet scaled to physical units
```

Unlike the `ADC`/`GPIO`/`PWM`/`SPI` register drivers, `CMPU9250` has a **public** constructor — you own the instance directly; only the underlying `CSPIModuleMMAP` is obtained through `CInterfaceManager` internally (in `initImu()`).

## `CMPU9250Setup` fields

| Field | Meaning |
|---|---|
| `gyroScale` | Full-scale range: `FS_RANGE_250/500/1000/2000` (°/s). |
| `accelScale` | Full-scale range: `FS_RANGE_2g/4g/8g/16g`. |
| `gyroLP` | Digital low-pass filter cutoff for the gyro (and, as a side effect, the temperature sensor's LPF), `G_LP_5HZ` through `G_LP_250HZ`, or `G_LP_OFF` to bypass the DLPF entirely (~8.8 kHz raw bandwidth). |
| `accelLP` | Digital low-pass filter cutoff for the accelerometer, `A_LP_5HZ` through `A_LP_460HZ`, or `A_LP_OFF` to bypass (~1.13 kHz raw bandwidth). |

## Output data

`readImu()` returns **raw sensor counts** (`int16_t`), not physical units — converting to g/°/s using the configured full-scale range, and applying per-device offset/scale calibration, happens elsewhere (see `CCalibration`/`CubeConstants.hpp`), not in this class.

`rawData` is a burst read of 14 consecutive registers (`ACCEL_XOUT_H` .. `GYRO_ZOUT_L`) in a single SPI transaction; the 2 temperature bytes in the middle of that range are read but discarded.

## Current usage

Both IMUs sit on SPI module 1, on distinct CS channels (0 and 1) — see [SPI/README.md](../SPI/README.md) and [CubeConstants.hpp](../../../include/CubeConstants.hpp) for the shared `CSPIChannelConfig` (1 MHz, 8-bit words, CS held active for the whole burst).

## Heads up

- `initImu()` does a soft reset (`PWR_MGMT_1` bit 7), waits 100ms, then checks `WHO_AM_I` against the expected `0x71` — if that fails (wrong wiring, dead sensor, SPI misconfigured), it returns `FAILED_TO_CONFIGURE_IMU` without retry.
- It also forces `USER_CTRL.I2C_IF_DIS`, permanently switching the chip to SPI-only mode for as long as it stays powered — don't expect I2C to work afterward without another power cycle.
- `readImu()` retries a failed burst read up to 3 times before giving up and returning `FAILED_TO_READ_SENSOR_DATA` — transient SPI hiccups are logged but self-heal; only a 3rd consecutive failure propagates up.
- `writeReg()` reads back every register write to verify it landed (`check=true` by default) — except the soft-reset write, which self-clears immediately and can't be verified this way (called with `check=false`).
