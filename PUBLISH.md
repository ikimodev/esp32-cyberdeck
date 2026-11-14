# Publish to GitHub

This project is prepared for `ikimodev/esp32-cyberdeck`. It has not been published merely because this file exists.

1. On GitHub, create a new **public** repository named **esp32-cyberdeck**, owned by **ikimodev**.
2. Leave README, .gitignore and license initialization disabled: this folder already contains them.
3. Push the prepared local `main` branch:

```sh
git push -u origin main
```

If publishing a downloaded copy rather than the prepared Git folder, initialize it first:

```sh
git init -b main
git add .
git commit -m "Open-source ESP32 Cyberdeck HW2 with docs and preview"
git remote add origin https://github.com/ikimodev/esp32-cyberdeck.git
git push -u origin main
```

Use GitHub's normal login flow if Git requests authentication. Do not put a token or password in a remote URL or source file.

Suggested description: `ESP32 OLED cyberdeck: real Wi-Fi scanner, RSSI tracker, local phone drawing UI, and a standalone code-matrix game.`

Suggested topics: `esp32`, `arduino`, `ssd1306`, `oled`, `wifi-scanner`, `cyberdeck`, `wokwi`, `diy-electronics`.

Read [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) before redistributing graphical assets: the user-supplied reference logo is excluded from MIT. The source code remains available under the stated project license.
