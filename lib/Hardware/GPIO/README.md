# GPIO/

`CGPIOMMAP` is a register-level driver for the AM335x GPIO banks (`GPIO0`-`GPIO3`), used by [CMaxonMotor](../Motors/README.md) for motor enable/direction.

## Usage

```cpp
CInterfaceManager<uint8_t, CGPIOMMAP> mgr;
auto pin = mgr.getInstance(gpioNumber, /*allowMultipleInstances=*/false).value();

pin->init(/*pOutput=*/true);
pin->setHigh();
pin->setLow();
bool level = pin->getCurrentState();
```

Always go through `CInterfaceManager` (see [../README.md](../README.md)) rather than constructing `CGPIOMMAP` directly — the constructor is private. Existing call sites request GPIOs with `allowMultipleInstances=false` (exclusive) — a given physical pin is meant to have exactly one owner.

## Pin numbering

The constructor/`getInstance` argument is a flat GPIO number, not a header pin: `gpio_nr = module*32 + pin`. Look up a header pin's module/pin (or its flat number directly) in the [BeagleBone Black P9/P8 header table](https://itbrainpower.net/a-gsm/images/BeagleboneBlackP9HeaderTable.pdf). Current usage, from [CubeConstants.hpp](../../../include/CubeConstants.hpp):

| Header pin | GPIO nr | Bank/pin | Usage |
|---|---|---|---|
| P8.7 | 66 | GPIO2_2 | Motor enable |
| P8.8 | 67 | GPIO2_3 | Motor direction |

## Behavior notes

- `init(pOutput)` sets direction (via `GPIO_OE`) and must be called once before `setHigh()`/`setLow()`/`getCurrentState()`.
- `setHigh()`/`setLow()` return `Status::CONFIG_ERROR` if the pin was initialized as an input (`pOutput=false`) — they don't silently no-op.
- `getCurrentState()` reads `GPIO_DATAIN` regardless of direction (works for both inputs and to read back an output's actual level).
- `init()` for GPIO module 0 maps a different clock-control block (`CM_WKUP`) than modules 1-3 (`CM_PER`) — this is handled internally, not something callers need to worry about.

## Device tree

P8.7/P8.8 default to plain-GPIO pinmux (`MUX_MODE7`) on the stock BeagleBone Black device tree, so normally **no overlay is required** to use them as GPIO. See [README.md — GPIO Overlay](../README.md#gpio-overlay) if some other cape/overlay re-muxes them.
