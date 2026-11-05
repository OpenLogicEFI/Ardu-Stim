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
- `ardustim/pattern_timing.h`: timer period calculation and GPIO bit mapping.
- `ardustim/ardustim.ino`: shared wheel tables, pattern advancement, sweep and
  compression logic; AVR register setup stays in the AVR branch.
- `ardustim/comms.cpp`: existing serial protocol. Configuration packets are
  published together and wheel changes recalculate timing. Sweep words are read
  in explicit byte order to work across AVR and ARM compilers.
- `ardustim/storage.ino`: AVR EEPROM updates or one buffered STM32 flash write.
  F401 also restores base RPM and persists the dynamic compression setting.
- `platformio.ini`: pins the STM32 platform to 19.7.1 (STM32duino 2.12.0), enables
  USB CDC and reserves flash sector 5 at `0x08020000` for settings. Both the linker
  and size check limit firmware to the first 128 KiB. The emulated EEPROM uses an
  8 KiB buffer inside the reserved 128 KiB sector; the entire sector is erased
  during a changed save. Do not allocate firmware or other data there.

The existing GUI currently puts `compressionDynamic` in byte 16 of its outgoing
packet, overlapping the high byte of `compressionOffset`; firmware expects it in
byte 17. This pre-existing GUI issue is outside this firmware change. Ordinary
pattern, RPM and sweep commands retain their protocol; testing that checkbox
needs a corrected GUI or a directly constructed configuration packet.

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
