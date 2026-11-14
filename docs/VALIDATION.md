# Validation record

Public-repository preparation: **2026-11-14**. Firmware baseline: **4.2.4-HW2**.

| Item | Evidence / scope |
|---|---|
| Complete production sketch | Previously compiled and linked with Arduino CLI 1.5.1, ESP32 core 3.3.12, `esp32:esp32:esp32` |
| Firmware flash / globals | 1,037,060 bytes (79%) / 56,464 bytes (17%) for the delivered baseline |
| Preview checks | 8 Node test groups: power states, exact duration, logo glitch, physical-row colors, navigation and bundled controls |
| Phone UI checks | 18 Node groups, including 2500 generated text cases, touch drawing, bitmap packing and request failures |
| Power/debounce assertions | 13 C++17 `static_assert` checks evaluated by the installed ESP32 compiler for the baseline |
| Breadboard connectivity | `wokwi/validate.mjs` checks the eight inputs, no cross-shorts, OLED pins, each button and lower-wire insertion points |
| Documentation and portability | `tools/check-repo.mjs` checks local links, generated sketch defaults and private paths/credential patterns |
| GitHub Actions | Workflow supplied; a successful cloud run is not claimed before it actually executes |

## Repository preparation checks, 2026-11-14

`npm run build` succeeded from the portable repository. `npm test` passed all 8 preview groups, all 18 phone UI groups, the breadboard graph checks, and the repository audit (44 text files / 50 local documentation links at the initial check). The regenerated complete firmware matched the previously compiled HW2 baseline; only trailing whitespace at the end of the generated file was removed for Git hygiene. The media builder produced a 22-second GIF and the overview PNG. This preparation did not flash or simulate ESP32 hardware.

The public builder regenerates the production sketch from the supplied structured source and shared assets. Repository preparation checks the generated result against the delivered baseline. New tests should report their own results rather than reusing the old build numbers for modified code.

## What was not tested

The complete HW2 firmware has not been uploaded to or executed on a physical ESP32 in this work. Real RF scans, the actual OLED frame rate, BACK long holds on a device, AP teardown/restart, and phone reconnection after soft power need hardware validation.

The browser preview uses fictional network names and simulated signal/system values. It is independent from the real radio implementation. The GIF is generated from that preview and is labeled accordingly.

The Wokwi screenshots are from an earlier screen/button wiring test. The full HW2 firmware was not simulated in Wokwi. The netlist checks are software graph checks, not an electrical simulator run.

There are 172 generated reference frames and optional C++ comparison code. The full C++/JavaScript pixel comparison and 500-puzzle runtime check have not been executed. Fixture generation alone is not a passed test.

The C++ syntax/assertion pass can emit existing aggregate-initializer warnings for empty `rawSsid` fields in the fixture network structs; these are not compiler errors. The font and logo license scopes are described in [third-party notices](../THIRD_PARTY_NOTICES.md).

No personal network scans, local machine telemetry, user images or account credentials are included in the public repository. The AP password is a published example default, intended to be changed in an individual build.
