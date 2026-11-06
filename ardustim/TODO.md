# TODO

Add support for one MCU at a time. Finish testing each board before starting
the next. Keep a note of the board, firmware revision, pattern, RPM and any
measurements so the results are useful later.

## STM32F401CC Black Pill — finish validation

Bench results:

- [x] Firmware runs on the F401.
- [x] USB serial connects to the existing GUI without issue.
- [x] RPM adjustment works well.
- [x] Crank output works.
- [x] CAM1 output works.
- [x] CAM2 output works (BMW N20 selected for the CAM2 bench check).
- [x] Trigger switching works well.
- [x] Settings survive reset and power cycling.

Bench checks:

- [ ] Recheck wheel transitions after the timer restart fix, including sparse
      low-RPM to dense high-RPM patterns; verify no old-period samples remain.
- [ ] Check compression timing across SysTick boundaries after the F401
      pending-tick timestamp fix; confirm no approximately 1 ms timestamp jumps.
- [x] Measure PB0 crank, PB1 CAM1 and PB2 CAM2 signal levels and timing with a
      scope or logic analyzer. With BMW N20, CAM1 and CAM2 should match.
- [ ] Check 60-2 at 3000 RPM: approximately 166.67 us per table sample and 20 ms
      per crank revolution. Compare cam pulse positions with their pattern tables.
- [ ] Test low cranking RPM and dense patterns at high RPM. Record the highest
      reliable RPM and check timing while sending repeated GUI commands.
- [x] Switch between long and short patterns while running. Check for stray
      pulses and verify RPM and phase after the switch.
- [ ] Test fixed RPM, sweeps and compression, including sweep limits and
      transitions. Ignore the known dynamic amplitude retention issue during
      board validation; it is deferred and does not block board testing (see README).
- [ ] Test the optional PA1 potentiometer across its 0–3.3 V range and confirm
      mode switching between potentiometer, fixed RPM and sweep operation.
- [x] Save changed settings, reset and fully power cycle; verify the selected
      trigger, base RPM, mode, sweep and compression settings are restored,
      excluding the deferred dynamic amplitude setting.
- [ ] Observe the intentional output-low gap during a changed save and the
      restart at sample zero; confirm USB/GUI operation recovers afterward.
- [ ] Repeat an unchanged save and confirm there is no flash-induced output gap.
- [x] Reconnect USB and restart the GUI. Check that settings load and the
      controls still work.
- [ ] Confirm repeatable USB DFU upload and normal startup with BOOT0 released.
- [ ] Re-run host regression checks and all existing AVR builds after any fixes.

## STM32F411 — next addition

- [ ] Identify the exact board, MCU, crystal, flash/RAM capacity and pinout.
- [ ] Confirm the PlatformIO target, USB CDC/DFU setup, timer clock and GPIO mapping.
- [ ] Verify the settings flash reservation against the F411 sector layout and
      linker limits; do not assume the F401 storage layout transfers unchanged.
- [ ] Implement board support and document wiring, upload and storage behavior.
- [ ] Build the F411, F401 and existing AVR targets; run host regression checks.
- [ ] Record measured timing limits and remaining board-specific issues before
      starting another MCU.

Bench checks (use the pinout confirmed for this board):

- [ ] Connect to the GUI and check settings reads and RPM adjustment.

- [ ] Measure crank, CAM1 and CAM2 signal levels and timing with a
      scope or logic analyzer. With BMW N20, CAM1 and CAM2 should match.
- [ ] Check 60-2 at 3000 RPM: approximately 166.67 us per table sample and 20 ms
      per crank revolution. Compare cam pulse positions with their pattern tables.
- [ ] Test low cranking RPM and dense patterns at high RPM. Record the highest
      reliable RPM and check timing while sending repeated GUI commands.
- [ ] Switch between long and short patterns while running. Check for stray
      pulses and verify RPM and phase after the switch.
- [ ] Test fixed RPM, sweeps and compression, including sweep limits and
      transitions. Ignore the known dynamic amplitude retention issue during
      board validation; it is deferred and does not block board testing (see README).
- [ ] Test the optional potentiometer on the confirmed ADC pin, within its
      documented voltage range. Switch between potentiometer, fixed RPM and sweep modes.
- [ ] Save changed settings, reset and fully power cycle; verify the selected
      trigger, base RPM, mode, sweep and compression settings are restored,
      excluding the deferred dynamic amplitude setting.
- [ ] Observe the intentional output-low gap during a changed save and the
      restart at sample zero; confirm USB/GUI operation recovers afterward.
- [ ] Repeat an unchanged save and confirm there is no flash-induced output gap.
- [ ] Reconnect USB and restart the GUI. Check that settings load and the
      controls still work.
- [ ] Confirm the chosen upload/recovery method works repeatedly and the board
      starts normally afterward. Check DFU and BOOT0 steps if used.
- [ ] Re-run host regression checks and all existing AVR builds after any fixes.

## ESP32-C3 Super Mini — feasibility, then implementation

- [ ] Identify the exact board/module, flash capacity, pinout and USB arrangement.
- [ ] Check whether suitable pins are available for crank, CAM1, CAM2 and an
      optional analog potentiometer without conflicting with USB, boot straps
      or onboard components.
- [ ] Assess timer/output timing, interrupt jitter and simultaneous three-output
      updates under USB activity; determine whether timing meets bench needs.
- [ ] Assess Arduino/PlatformIO compatibility, the existing GUI serial protocol,
      ADC input limits and settings storage/write interruptions.
- [ ] Decide feasibility and document limitations before implementing support.
- [ ] If feasible, implement and document the target, wiring, upload and storage.
- [ ] Build all supported targets and run host regression checks.

Bench checks (use the pinout confirmed for this board):

- [x] Connect to the GUI and check settings reads and RPM adjustment.

- [x] Measure crank, CAM1 and CAM2 signal levels and timing with a
      scope or logic analyzer. With BMW N20, CAM1 and CAM2 should match.
- [ ] Check 60-2 at 3000 RPM: approximately 166.67 us per table sample and 20 ms
      per crank revolution. Compare cam pulse positions with their pattern tables.
- [ ] Test low cranking RPM and dense patterns at high RPM. Record the highest
      reliable RPM and check timing while sending repeated GUI commands.
- [x] Switch between long and short patterns while running. Check for stray
      pulses and verify RPM and phase after the switch.
- [ ] Test fixed RPM, sweeps and compression, including sweep limits and
      transitions. Ignore the known dynamic amplitude retention issue during
      board validation; it is deferred and does not block board testing (see README).
- [x] Test the optional potentiometer on the confirmed ADC pin, within its
      documented voltage range. Switch between potentiometer, fixed RPM and sweep modes.
- [x] Save changed settings, reset and fully power cycle; verify the selected
      trigger, base RPM, mode, sweep and compression settings are restored,
      excluding the deferred dynamic amplitude setting.
- [ ] Check output behavior during a changed save against the documented storage
      behavior. Confirm outputs and USB/GUI controls recover afterward.
- [ ] Recheck wheel transitions after the timer restart fix; scope sparse
      low-RPM to dense high-RPM changes and verify no old-period samples remain.
- [ ] Repeat an unchanged save and check that it skips unnecessary flash writes
      and output interruptions.
- [ ] Reconnect USB and restart the GUI. Check that settings load and the
      controls still work.
- [ ] Confirm the documented upload and recovery steps work repeatedly, and
      check normal startup afterward.
- [ ] Re-run host regression checks and all existing AVR builds after any fixes.

## ESP32-C3 Super Mini

- [x] Add pinned PlatformIO target and native USB Serial/JTAG upload configuration.
- [x] Add three-output timer backend, optional pot, and buffered settings commits.
- [x] Compile C3 and existing AVR/F401 targets; run shared host protocol/storage checks.
- [ ] Physical USB upload/recovery and GUI reconnect testing.
- [ ] Scope all outputs: phase/skew, dense patterns/high RPM, USB traffic jitter.
- [ ] Check pot range, sweep/compression, settings persistence and save pause/restart,
      excluding the deferred dynamic amplitude retention issue.

## Pre-existing issue — dynamic amplitude (deferred software and firmware fixes)

Application changes are outside the current firmware-only scope. The GUI packet
correction and AVR persistence changes have been reverted. Earlier settings
retention results above do not establish dynamic amplitude retention. Ignore this
known issue for all current board validation; it is not a board acceptance
requirement. The work and tests below are deferred until this issue is revisited.

- [ ] Future application work: correct the dynamic flag to byte 17 without
      overwriting compression offset, then build and verify the GUI package.
- [ ] Future AVR firmware work: add dynamic flag persistence.

- [ ] Uno: complete the dynamic amplitude procedure below.
- [ ] Nano: complete the dynamic amplitude procedure below.
- [ ] Duemilanove/Diecimila ATmega328: complete the procedure below.
- [ ] F401 Black Pill: complete the procedure below.
- [ ] ESP32-C3 Super Mini: complete the procedure below.
- [ ] F411, when implemented: repeat the procedure when this issue is revisited.

ESP32-C3 bench status: the user tested the downloaded AppImage and reported the
setting lost after reset/power cycling. The GUI packet defect remains present;
this test does not establish a separate ESP32 flash-storage defect. Keep the
issue open and defer the following bench procedure until fixes are implemented.

Deferred procedure for each board, after future fixes (record firmware/GUI
revision and measured results):

1. Enable compression, set a nonzero offset (including 359 degrees), then enable
   Dynamic amplitude. Read configuration back or reconnect the GUI and confirm
   both fields retain their values. Repeat with Dynamic amplitude disabled.
2. For each checkbox state, Save, reset, reconnect, then fully power cycle and
   reconnect. Confirm the checkbox and compression offset are restored together.
3. At fixed RPM below the compression RPM, scope a suitable crank pattern with
   compression enabled. Compare dynamic enabled/disabled at two different low
   RPMs; confirm the modulation amplitude changes as intended and the saved
   setting produces the same behavior after reboot.
4. On AVR, confirm an older settings image with address 20 erased loads Dynamic
   amplitude disabled; then Save enabled and verify retention. Preserve any
   settings needed before deliberately loading an older image.
