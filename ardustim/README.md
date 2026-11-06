# Ardu-Stim firmware

The original AVR targets remain available. The new `blackpill_f401cc` target is
for a WeAct-style STM32F401CC Black Pill with a 25 MHz crystal, 256 KiB flash and
64 KiB RAM. F411 and F103 targets are not included in this change.

F401 status: builds and host regression checks pass. User bench testing confirms
GUI connection, RPM adjustment, crank and both cam outputs, and trigger switching.
Bench testing also confirms settings survive reset and power cycling, output
signal levels/timing were checked, and GUI controls work after reconnecting USB.
Specific timing benchmarks and the remaining checks are tracked below. See [TODO.md](TODO.md) for completed checks and
the remaining validation plan, including the BMW N20 CAM2 check.

## F401 wiring and upload

| Function | Black Pill pin |
| --- | --- |
| Crank / primary output | PB0 |
| Cam 1 / secondary output | PB1 |
| Cam 2 / tertiary output | PB2 |
| Optional RPM potentiometer wiper | PA1 |
| Potentiometer ends | 3V3 and GND |
| GUI connection | Board USB connector (USB CDC serial) |
| Firmware upload | Board USB connector (built-in DFU bootloader) |

The three outputs are **3.3 V push-pull**. Check the receiving input or existing
buffer circuit before connecting it; use a suitable buffer if 5 V output is
required. The potentiometer input must stay between GND and 3.3 V. PA0 is left
for the board's user button. Common ground is required with the receiving device.

Build from this directory using PlatformIO:

```sh
pio run -e blackpill_f401cc
```

To upload over USB, connect a data-capable USB cable, hold **BOOT0**, press and
release **RESET/NRST**, then release **BOOT0**. The board should appear as an
STM32 DFU bootloader device. Run:

```sh
pio run -e blackpill_f401cc -t upload
```

The upload command uses USB DFU; no ST-Link or separately installed bootloader is
required. DFU is a USB device, not a serial port, so no serial upload port needs
selecting. If access is denied on Linux, check the PlatformIO USB udev rules.
After upload, press RESET with BOOT0 released. Then select
the board's USB serial port in the existing GUI (typically `/dev/ttyACM*` on Linux).
The protocol still uses the existing 18-byte configuration format and 115200
serial setup. The GUI's Nano firmware-upload button does not program this board;
use PlatformIO for F401 uploads. No GUI source has been changed.

Settings saves deliberately pause the outputs low while flash is erased/written,
then restart the wheel at sample zero. A save with identical stored values skips
the flash write and pause. Do not depend on uninterrupted signals during Save.
Flash erase can also delay USB responses. This is settings storage, not a
power-loss-safe transactional store; an interrupted save may require resetting
settings. Reflashing with a full-chip erase clears the stored settings.

## Implementation and debugging map

- `ardustim/f401_backend.cpp`: TIM2, GPIOB, potentiometer polling, and save pause.
  TIM2 uses its full 32-bit counter at the actual timer clock with prescaler 1.
  The reload is buffered so RPM updates cannot shorten the interval in progress;
  a new value takes effect after the next reload boundary. Its IRQ has higher
  priority than USB. PB0-PB2 change together with a masked BSRR write that leaves
  unrelated GPIO pins alone.
  Cycle timestamps use a pending-SysTick-aware snapshot, including when TIM2
  prevents the millisecond interrupt from running at a rollover. The timestamp
  reader restores the caller's interrupt mask and leaves SysTick pending.
- `ardustim/pattern_timing.h`: timer period calculation and GPIO bit mapping.
- `ardustim/ardustim.ino`: shared wheel tables, pattern advancement, sweep and
  compression logic; AVR register setup stays in the AVR branch.
- `ardustim/comms.cpp`: existing serial protocol. Configuration packets are
  published together and wheel changes recalculate timing. Sweep words are read
  in explicit byte order to work across AVR and ARM compilers.
  On F401 and ESP32-C3, wheel changes stop the timer before publishing the wheel,
  drive outputs low, clear pending timer events, and restart at sample zero with
  the new period. The transition intentionally resets phase; ordinary RPM edits
  preserve phase and retain the backend's buffered/scheduled update behavior.
- `ardustim/storage.ino`: AVR EEPROM updates or one buffered STM32 flash write.
  F401 also restores base RPM and persists the dynamic compression setting.
- `platformio.ini`: pins the STM32 platform to 19.7.1 (STM32duino 2.12.0), enables
  USB CDC and reserves flash sector 5 at `0x08020000` for settings. Both the linker
  and size check limit firmware to the first 128 KiB. The emulated EEPROM uses an
  8 KiB buffer inside the reserved 128 KiB sector; the entire sector is erased
  during a changed save. Do not allocate firmware or other data there.

## Verification

```sh
pio run -e blackpill_f401cc -e uno -e nano -e diecimilaatmega328
python3 test/run_host_tests.py
git diff --check
```

Host tests compile the actual serial and storage implementation against small
Arduino/EEPROM mocks. They check timer ranges/rounding, all output-bit states,
binary configuration layout and byte order, invalid wheel/mode rejection,
timing refresh after configuration changes, one-write settings saves, unchanged
save suppression and settings reload. They use the undefined-behavior sanitizer.
They do not simulate the STM32 peripheral hardware or prove interrupt latency.
Additional tests compile both production backends against peripheral mocks to
check restart counter/alarm/reload state, pending-event clearing, save/restart,
and F401 pending-SysTick timestamps and interrupt-mask restoration. Serial tests
cover stop-before-publication for packet, direct and next/previous wheel changes,
and verify that RPM-only edits do not restart the timer.

For the first board test, use a scope or logic analyzer on PB0-PB2:

1. Confirm USB connection and GUI pattern/configuration reads.
2. Select 60-2 at 3000 RPM: the sample interval should be about 166.67 us and
   the crank cycle 20 ms. Check cam patterns against their pattern tables.
3. Test low cranking RPM and dense patterns at high RPM; check timing under
   repeated USB commands. Determine the reliable RPM ceiling on real hardware.
4. Exercise fixed RPM, sweeps, compression and a 3.3 V potentiometer on PA1.
5. Change between long and short patterns and verify output phase and RPM.
6. Save, observe the intentional gap/restart, power cycle, and check retention.
   Repeat an unchanged save and confirm there is no flash-induced gap.

Start in fixed RPM mode if no potentiometer is attached; the initial stored
defaults select potentiometer mode.

## ESP32-C3 Super Mini (experimental)

The `esp32c3_supermini` target supports the common **4 MiB ESP32-C3 Super Mini**.
It uses the DevKitM-1 build definition with explicit Super Mini pins; it does not
use the DevKit's LED or UART connections. ESP32-S3/C6 boards are different targets.

| Function | Super Mini pin |
| --- | --- |
| Crank | GPIO3 |
| Cam 1 | GPIO4 |
| Cam 2 | GPIO5 |
| Optional RPM pot wiper | GPIO0 |
| Pot ends | 3V3 and GND |
| GUI and firmware upload | Board USB-C connector |

Outputs are 3.3 V push-pull, not 5 V tolerant. Use a suitable buffer for 5 V
receivers, keep analog input within 0–3.3 V, and connect a common ground.
GPIO2/8/9 (boot straps), GPIO8's usual LED, and GPIO18/19 (USB) are left alone.
Confirm your seller's pinout because Super Mini clones vary. ADC attenuation is
11 dB and readings are scaled to 10 bits; the C3 ADC can saturate below 3.3 V,
so this is an RPM control rather than a precision voltage measurement.

### USB upload

**The C3 cannot use USB DFU.** Its built-in USB Serial/JTAG bootloader supports
`esptool` uploads over the board USB connector, without an external programmer.
See [Espressif's USB Serial/JTAG documentation](https://docs.espressif.com/projects/esp-idf/en/stable/esp32c3/api-guides/usb-serial-jtag-console.html).
The STM32 target continues to use DFU.

```sh
pio run -e esp32c3_supermini
pio run -e esp32c3_supermini -t upload --upload-port /dev/ttyACM0
```

Replace the port with the actual board port (Windows: `COMx`). Close the GUI or
serial monitor before uploading. For first upload or recovery, hold **BOOT**,
press and release **RESET**, then release BOOT. Upload, then press RESET with
BOOT released if the board remains in download mode. Subsequent uploads can
normally enter download mode automatically over USB. Use a USB data cable.

After reset select the board's USB serial port in the existing GUI at 115200.
The 18-byte configuration protocol is unchanged. Use PlatformIO for uploading;
the GUI's Nano upload button does not upload ESP32 firmware. No GUI changes are
included. Start in fixed RPM mode without a pot; fresh settings select pot mode.

### Backend and validation

`ardustim/pattern_backend.h` maps shared operations to F401 or C3 implementations.
`ardustim/esp32c3_backend.cpp` uses timer group 0/timer 0 at 10 MHz. Absolute alarm
deadlines avoid accumulating callback latency, and RPM changes apply when the
next alarm is scheduled. If an interrupt overruns its next deadline, it schedules
one full interval ahead rather than producing a burst of catch-up samples.
GPIO3–5 use masked clear/set writes; unlike F401 BSRR, falling and rising edges
on different outputs have a small inter-write skew. No Wi-Fi/Bluetooth stacks
are started. The interrupt and pattern tables are flash-backed, not IRAM-safe.

Settings use a 32-byte EEPROM emulation buffer with one commit per changed save.
Outputs pause low during commit and restart at sample zero; unchanged saves do
not pause. A failed commit remains pending for the next Save. Saving is not a
power-loss-safe transaction and may delay GUI responses. Do not add runtime
flash writes or radio features without reviewing interrupt/cache behavior.

C3 builds and host serial/storage regression tests pass. Hardware upload, GUI,
settings retention, potentiometer behavior, three-output phase/skew and maximum
reliable RPM remain unverified. Before relying on it, repeat the F401 scope tests
above using GPIO3/4/5, including dense patterns under repeated GUI commands,
low cranking RPM, sweep/compression, changed and unchanged saves, power cycling,
and USB disconnect/reconnect. Timer resolution does not establish actual jitter.

## Pre-existing Ardu-Stim issues unrelated to board additions

### Dynamic amplitude setting does not persist — unresolved

Dynamic amplitude retention is not fixed. The downloaded AppImage was used for
an ESP32-C3 bench test in which the setting was lost after reset/power cycling.
The issue also requires a GUI software fix; firmware updates alone cannot
correct the packet sent by the existing application.

The existing GUI sends Dynamic amplitude in byte 16, overwriting the high byte
of the two-byte compression offset (bytes 15–16). Firmware expects the flag in
byte 17. The GUI correction has been reverted because application changes are
outside the current firmware-only scope. No corrected AppImage is included.

Original AVR firmware also omits this flag from EEPROM saves and loads. The
attempted AVR persistence fix has been reverted; Uno, Nano and Duemilanove keep
their prior behavior. F401 and ESP32-C3 retain the storage support included with
their board backends, but this does not resolve the existing GUI packet issue.

Future work must correct the GUI packet, add AVR persistence, and bench-test
both checkbox states with compression offset retained across Save, reconnect,
reset and full power cycling on every supported board. These checks remain
pending in TODO.md. No end-to-end dynamic amplitude retention fix is claimed.

### Incomplete serial commands stall controls — unresolved

`comms.cpp` waits indefinitely for the remaining bytes of configuration (`c`),
sweep (`r`) and wheel (`S`) commands. A truncated command or disconnect during
receipt can stall the main loop. Timer outputs continue, but potentiometer,
sweep, compression updates and further command processing stop progressing.
A future fix needs bounded packet reception and recovery after incomplete input.

### Two-cylinder compression reads beyond its table — unresolved

`calculateCurrentCrankAngle()` permits exactly 360 degrees. Two-cylinder
compression divides that angle by two and reads index 180 of the 180-entry
`sin_100_180` table, whose last valid index is 179. This can produce an incorrect
compression modifier. A host test against the shared implementation reproduced
the out-of-bounds read with the undefined-behavior sanitizer. A future fix should
normalize angles to 0–359 degrees before lookup and check the boundary cases.

### Sweep timing fails near micros rollover — unresolved

The sweep test compares `micros()` against `sweep_time_counter + sweep_interval`.
That addition can wrap, causing steps to run too quickly near the roughly
71-minute microsecond-counter rollover. A host reproduction stepped after only
16 microseconds with a 1,000-microsecond interval. A future fix should compare
unsigned elapsed time and test both sides of rollover.

### Zero compression threshold can divide by zero — unresolved

With dynamic compression enabled, base RPM zero and `compressionRPM` zero,
`calculateCompressionModifier()` divides by zero. Serial configuration and
settings loading do not reject a zero threshold. The GUI normally limits this
field to at least 20 RPM, but firmware must also validate it. A host sanitizer
test reproduced the division by zero. A future fix needs input validation and a
guard in the calculation, including zero-pot and stored-settings cases.

These shared issues predate the MCU additions and remain unfixed. The board
timing fixes do not resolve them.
