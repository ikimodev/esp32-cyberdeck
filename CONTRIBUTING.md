# Contributing

Small, focused pull requests are welcome. Explain the user-visible change, include a screenshot or a short capture for OLED changes, and say which checks actually ran.

1. Use Node.js 20+; the JavaScript tools have no npm dependencies.
2. Run `npm run build`, then `npm test`.
3. For firmware changes, compile `firmware/Cyberdeck_Hardware` with Arduino ESP32 3.3.12. Evaluate the 13 compile-time assertions with a C++17 compiler: `g++ -std=c++17 -fsyntax-only tests/power_compile_tests.cpp`.
4. Edit the structured files in `firmware/cyberdeck/` and `web/index.html`; regenerate the combined sketch with `npm run build:firmware`.
5. Keep OLED drawing binary and within 128×64. The yellow/blue boundary is fixed by the panel, not selected by software. Keep long holds, short releases, and scan cancellation responsive.
6. Label fixture data and screenshots as preview data. Do not add real SSIDs, BSSIDs, passwords, local paths, machine logs or personal photographs to test fixtures.

Please preserve the font notice and the logo licensing exception in [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md). Do not submit third-party artwork without a documented license.

Useful areas to improve include a fully original open-licensed wordmark, hardware validation reports, better accessibility, and additional Wi-Fi diagnostics. Device enumeration on a LAN is not implemented in this version.
