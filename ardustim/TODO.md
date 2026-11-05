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

- [x] Measure PB0 crank, PB1 CAM1 and PB2 CAM2 signal levels and timing with a
      scope or logic analyzer. With BMW N20, CAM1 and CAM2 should match.
- [ ] Check 60-2 at 3000 RPM: approximately 166.67 us per table sample and 20 ms
      per crank revolution. Compare cam pulse positions with their pattern tables.
- [ ] Test low cranking RPM and dense patterns at high RPM. Record the highest
      reliable RPM and check timing while sending repeated GUI commands.
- [x] Switch between long and short patterns while running. Check for stray
      pulses and verify RPM and phase after the switch.
- [ ] Test fixed RPM, sweeps and compression, including sweep limits and
      transitions. Test dynamic compression only with the corrected
      GUI packet or a direct packet (see README protocol note).
- [ ] Test the optional PA1 potentiometer across its 0–3.3 V range and confirm
      mode switching between potentiometer, fixed RPM and sweep operation.
- [x] Save changed settings, reset and fully power cycle; verify the selected
      trigger, base RPM, mode, sweep and compression settings are restored.
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
      transitions. Test dynamic compression only with the corrected
      GUI packet or a direct packet (see README protocol note).
- [ ] Test the optional potentiometer on the confirmed ADC pin, within its
      documented voltage range. Switch between potentiometer, fixed RPM and sweep modes.
- [ ] Save changed settings, reset and fully power cycle; verify the selected
      trigger, base RPM, mode, sweep and compression settings are restored.
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
      transitions. Test dynamic compression only with the corrected
      GUI packet or a direct packet (see README protocol note).
- [ ] Test the optional potentiometer on the confirmed ADC pin, within its
      documented voltage range. Switch between potentiometer, fixed RPM and sweep modes.
- [ ] Save changed settings, reset and fully power cycle; verify the selected
      trigger, base RPM, mode, sweep and compression settings are restored.
- [ ] Check output behavior during a changed save against the documented storage
      behavior. Confirm outputs and USB/GUI controls recover afterward.
- [ ] Repeat an unchanged save and check that it skips unnecessary flash writes
      and output interruptions.
- [ ] Reconnect USB and restart the GUI. Check that settings load and the
      controls still work.
- [ ] Confirm the documented upload and recovery steps work repeatedly, and
      check normal startup afterward.
- [ ] Re-run host regression checks and all existing AVR builds after any fixes.
