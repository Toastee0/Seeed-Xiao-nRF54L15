# reset_reason

Prints the nRF54L15 `RESETREAS` register at boot, decoded bit by bit, so an
unexpected reset tells you *why* it happened before you start debugging firmware.

```
*** Reset reason: 0x00000001 ***
***   RESETPIN: reset pin
```

If the `SECTAMPER` bit is set (the tamper controller's glitch detectors or
active shield reset the chip), an extra `!!! Tamper reset` line suggests checking
the supply rail, USB cable/hub and probe wiring first.

`0x00000000` means a power-on reset or brownout (the hardware clears the register on POR).

## How the samples use it

Every sample's `CMakeLists.txt` adds this directory as a Zephyr module before
`find_package(Zephyr)`:

```cmake
list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_LIST_DIR}/../lib/reset_reason)
```

It is on by default for nRF54L builds (`SOC_SERIES_NRF54LX`) and off elsewhere.
Output goes through `printk`, so it appears on whatever console the sample uses.
With a USB CDC console the line is printed before the host opens the port, so
read the value with `xiao_reset_reason_get()` instead if you need it there.

## Options

| Kconfig | Default | Purpose |
|---|---|---|
| `CONFIG_XIAO_RESET_REASON` | `y` on nRF54L | Enable the readout |
| `CONFIG_XIAO_RESET_REASON_CLEAR` | `y` | Clear `RESETREAS` after reading. The register is cumulative until cleared, so leaving it set makes old reasons show up again on later boots. Set `n` if something else reads it later (`lowpower` and `bluetooth/peripheral_mds`/Memfault do). |

To turn it off for a sample, add `CONFIG_XIAO_RESET_REASON=n` to its `prj.conf`.

## API

```c
#include <xiao_reset_reason.h>

uint32_t raw = xiao_reset_reason_get();  /* RESETREAS as captured at boot */
```

Not available in non-secure (`/ns`) builds, where the RESET peripheral belongs
to the secure side.
