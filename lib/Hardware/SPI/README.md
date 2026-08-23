# SPI/

`CSPIModuleMMAP` is a register-level driver for the AM335x McSPI peripherals (McSPI0/McSPI1), used by [CMPU9250](../IMU/README.md) to talk to the IMUs. `CSPIChannelConfig` is the plain per-channel config struct passed to it.

Each McSPI module has **2 hardware channels** (chip selects). Only one instance should ever be created per module — even when using both channels — since simultaneous transmission on both channels isn't possible; share the module instance instead (see below).

## Usage

```cpp
CInterfaceManager<uint8_t, CSPIModuleMMAP> mgr;
auto spi = mgr.getInstance(moduleIdx /* 0 or 1 */, /*allowMultipleInstances=*/true).value();

spi->initModule();   // idempotent, safe to call once per module regardless of channel count

CSPIChannelConfig cfg{
    .sclk_Frequency_Hz = 1000000,
    .sclkHighActive = false,
    .samplingOnEvenEdge = true,
    .csHighActive = false,
    .csMaintainActive = true,
    .csTiming = CSPIChannelConfig::CS_15,
    .startBitSelection = CSPIChannelConfig::NO_STARTBIT,
    .wordLength = 8,
};
spi->configChannel(channel /* 0 or 1 */, cfg);   // once per channel — see below

uint32_t tx[2] = {...}, rx[2];
spi->dataExchangeTxRx(channel, tx, rx, /*numWords=*/2);
```

Always go through `CInterfaceManager` (see [../README.md](../README.md)) rather than constructing `CSPIModuleMMAP` directly — the constructor is private. Get it with `allowMultipleInstances=true` if more than one consumer will share a module's two channels (both IMUs do, on module 1).

## Module/channel wiring in this project

Only **SPI1** is wired up (SPI0's pins aren't broken out by the current overlay). See [README.md — SPI1 Overlay](../README.md#spi1-overlay-custom-cs-pins) for the pin/overlay details:

| Signal | Header pin |
|---|---|
| SCLK | P9.31 |
| MISO (D0) | P9.29 |
| MOSI (D1) | P9.30 |
| CS0 | P9.20 (moved off the stock default P9.28) |
| CS1 | P9.19 |

Both IMUs use SPI module 1, on channels 0 and 1 respectively (`IMU1_SPI_CHANNEL`/`IMU2_SPI_CHANNEL` in [CubeConstants.hpp](../../../include/CubeConstants.hpp)).

## `CSPIChannelConfig` fields

| Field | Meaning |
|---|---|
| `sclk_Frequency_Hz` | 11.719 Hz to 48 MHz. |
| `sclkHighActive` / `samplingOnEvenEdge` | Together select SPI mode 0-3 (see field doc-comments in [CSPIChannelConfig.hpp](CSPIChannelConfig.hpp)). |
| `csHighActive` | CS polarity. |
| `csMaintainActive` | Keep CS asserted between words in the same `dataExchangeTxRx()` call — needed for burst reads/writes that must appear as one continuous transaction to the device (e.g. the IMU's address+data burst reads). |
| `csTiming` | Delay between CS assertion and the first clock edge (0.5-3.5 clock cycles). |
| `startBitSelection` | Optional extra bit before the word, with configurable polarity. |
| `wordLength` | 4-32 bits. |

`dataExchangeTxRx` always takes/returns `uint32_t` arrays regardless of `wordLength` — mask down to the configured width yourself (e.g. `rx[i] & 0xFF` for an 8-bit word).

## Heads up

- **`configChannel()` can only be called once per channel per module instance** — a second call returns `CHANNEL_NOT_AVAILABLE` (it checks `mChannelConfig[channel].has_value()`). There's no way to reconfigure a channel short of destroying/recreating the module object.
- **CS idle state:** before `configChannel()` is called for a channel, its CS defaults to idle-high. If you're mixing SPI devices with non-standard (active-high) CS polarity, configure **all** channels before starting any communication — an unconfigured channel's default idle-high CS can otherwise collide with a device expecting the opposite idle polarity.
- `dataExchangeTxRx()` timeouts are derived from `wordLength`/`sclk_Frequency_Hz` (with a 20x headroom multiplier, floored at 1ms) — an `sclk_Frequency_Hz` that doesn't match reality won't break correctness, but will change how long a genuinely stuck transfer takes to report `TRANSFER_TIMEOUT`.
