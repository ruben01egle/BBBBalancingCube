# ADC/

`CADCMMAP` is a register-level driver for the AM335x's `ADC_TSC` peripheral (`0x44E0D000`), used by [CMaxonMotor](../Motors/README.md) for current/velocity sensing. `CADCConfig` is the plain config struct passed to it.

## Usage

```cpp
CInterfaceManager<uint8_t, CADCMMAP> mgr;
auto adc = mgr.getInstance(stepIdx, /*allowMultipleInstances=*/true).value();

CADCConfig cfg{
    .mode = CADCConfig::Mode::ONESHOT,
    .channel = CADCConfig::Channel::AIN0,
    .fifo = CADCConfig::FIFOSel::FIFO1,
    .averaging = CADCConfig::Averaging::AVG_4_SAM,
    .sampleDelay = 10,
    .openDelay = 20,
};
adc->init(cfg);

uint16_t value;
adc->readADC(value);
```

Always go through `CInterfaceManager` (see [../README.md](../README.md)) rather than constructing `CADCMMAP` directly — the constructor is private.

## `CADCConfig` fields

| Field | Meaning |
|---|---|
| `mode` | `ONESHOT` — trigger a single conversion per `readADC()` call, block until it's ready. `CONTINUOUS` — the step free-runs once enabled; `readADC()` just drains whatever the fifo has queued and returns the newest value (discarding older ones). |
| `channel` | Physical analog input pin, `AIN0`-`AIN6`. Only `AIN0` (P9.39) is wired up in this project — see [README.md](../README.md#adc). |
| `fifo` | Which of the two hardware FIFOs (`FIFO0`/`FIFO1`) this step's results land in. Steps sharing a fifo interleave results there; `readFifo()` demuxes by the step-index tag the hardware stamps on each fifo word. |
| `averaging` | Hardware oversampling: `NO_AVG` through `AVG_16_SAM`. |
| `sampleDelay` / `openDelay` | Step timing (`STEPDELAY` register): sample-and-hold and charge/open delay, in ADC clock cycles. |

## Step index

The constructor argument (and the id you pass to `CInterfaceManager`) is an **ADC step number**, 1-16 — not the analog channel. The AM335x ADC_TSC has 16 independently-configurable sequencer "steps", each of which picks its own channel/mode/averaging; a channel can be sampled by more than one step. `init()` rejects anything outside `1..16`. This project only uses step 1 (`ADC_STEP_IDX` in [CubeConstants.hpp](../../../include/CubeConstants.hpp)) mapped to `AIN0`.

## Heads up

- **The module clock is not enabled by this class.** Unlike [GPIO](../GPIO/README.md), [PWM](../PWM/README.md), and [SPI](../SPI/README.md) in this codebase, `CADCMMAP::init()` only resets/configures registers — it never pokes a `CM_PER`/`CM_WKUP` clock-gate. The `ADC_TSC` module must already be powered via the device tree (an overlay like `BB-ADC-00A0`, or already brought up by the base image). If that's missing, `init()` still reports `OKAY`, but reads will hang or return stale/zero data. See [README.md — ADC Overlay](../README.md#adc-overlay).
- In `ONESHOT` mode, `readADC()` busy-loops until a value shows up in the queue for this step — there's no timeout. If the module clock isn't running (see above) or the step is misconfigured, this call hangs forever.
- `readADC()` only ever returns the **latest** queued value for a step and discards the rest — fine for a control-loop sample-on-demand pattern, not suitable if you need every sample.
