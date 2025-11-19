bluepad32 porting

We are porting bluepad32 to work on the seeed studio xiao nrf54l15, it uses a different ble stack, we are currently working on the GATT setup when an x-box controller connects.

## Current Project State (Nov 11, 2025)

### Structure Complete ✅
- Project located at: `bluepad32_zephyr/`
- Build system configured (CMakeLists.txt, Kconfig, prj.conf)
- Successfully compiles with Nordic nRF Connect SDK v3.2.0-preview2
- Device tree overlay configured for Xiao nRF54L15

### What's Working ✅
- BLE initialization with SoftDevice Controller
- Passive scanning (30ms interval/window, matching Bluepad32 original)
- Xbox controller detection (filters for F4:xx MAC addresses)
- Connection establishment
- Delayed pairing to improve connection stability
- Pairing/bonding (BT_SECURITY_L2)
- Connection callbacks (connected, disconnected)
- Pairing callbacks (complete, failed, cancel)
- **NEW: Full GATT service discovery implementation**

### GATT Client Implementation ✅ (JUST COMPLETED)

Implemented complete GATT client for HID service discovery:

**Features:**
- HID Service (0x1812) discovery
- Characteristic discovery:
  - Report Map (0x2A4B) - HID descriptor
  - Report (0x2A4D) - Input reports
  - HID Information (0x2A4A)
  - Control Point (0x2A4C)
- CCC descriptor discovery
- Report Map reading (up to 512 bytes)
- Report notification subscription
- State machine-based discovery flow

**File:** `src/bluetooth/bt_gatt_client.c`

**Flow:**
1. Discover HID Service → get handle range
2. Discover all characteristics within service
3. Read Report Map (HID descriptor) completely
4. Discover CCC descriptor for Report characteristic
5. Subscribe to Report notifications
6. Start receiving controller input

**Key Improvements vs Original Issue:**
- Added 500ms delay before pairing (fixes "Pairing failed: reason 9")
- Proper connection stabilization before security upgrade
- Cancels pending pairing work on disconnect
- Full GATT discovery with proper error handling

### Next Steps (Phase 2B)

**1. HID Report Parser (Priority)**
- Parse HID Report Descriptor to understand report structure
- Extract Xbox controller button mappings
- Parse analog stick data (X/Y axes)
- Parse trigger data (LT/RT)
- Parse D-pad state

**File to implement:** `src/parser/uni_hid_parser_xboxone.c`

**2. Controller Data Callback**
- Create callback structure for parsed input
- Pass button states to application
- Handle thumbstick deadzone
- Normalize trigger values

**3. Testing**
- Verify complete flow: scan → connect → pair → discover → subscribe → parse input
- Test with actual Xbox controller on hardware
- Validate button mappings
- Check latency/responsiveness

### Hardware Target
- **Board:** Seeed Studio Xiao nRF54L15 (xiao_nrf54l15/nrf54l15/cpuapp)
- **BLE Stack:** Nordic SoftDevice Controller (different from Bluepad32's ESP-IDF Bluedroid)
- **Memory:** 256KB SRAM, using full allocation for cpuapp

### Build Commands
```bash
cd bluepad32_zephyr
west build -b xiao_nrf54l15/nrf54l15/cpuapp
west flash
```

### Module Status

**Implemented:**
- `src/main.c` - Core BLE scanning, connection, pairing ✅
- `src/bluetooth/bt_gatt_client.c` - Complete GATT discovery ✅
- Build configuration ✅
- Board overlay ✅

**Next Priority:**
- `src/parser/uni_hid_parser_xboxone.c` - Xbox HID report parser (NEXT)
- `src/controller/uni_gamepad.c` - Gamepad data structure
- `src/device/uni_hid_device.c` - Device lifecycle management
- `src/platform/uni_platform_zephyr.c` - Platform integration

### Test Strategy
- Focus on ONE controller type first (Xbox)
- Test each phase incrementally on hardware
- Defer documentation until successful hardware test
- No README generation until basic functionality proven

### Known Issues Addressed
- ~~Pairing failed (reason 9)~~ → Fixed with delayed pairing
- ~~Stale connection detection~~ → Proper cleanup in disconnect
- ~~GATT discovery not implemented~~ → Complete implementation added
