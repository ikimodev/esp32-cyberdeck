# Build and upload / البناء والرفع

[README](../README.md) · [العربية](../README.ar.md)

## Arduino IDE

Open [`firmware/Cyberdeck_Hardware/Cyberdeck_Hardware.ino`](../firmware/Cyberdeck_Hardware/Cyberdeck_Hardware.ino) inside its named folder. Install **esp32 by Espressif Systems 3.3.12**, choose **ESP32 Dev Module**, 4MB flash, default partitions and PSRAM disabled. Select your own serial port and upload. Open Serial Monitor at 115200 baud.

الملف الموحّد كامل؛ لا تنسخ الكود تحت قالب Arduino الافتراضي. استبدل القالب كله إذا كنت تنقله بالنسخ واللصق، واحفظه داخل مجلد باسم `Cyberdeck_Hardware`. لا يحتاج رفع صور أو ملفات إلى SPIFFS.

## Arduino CLI

Use Arduino CLI 1.5.1, the version used to validate this firmware, and install the pinned ESP32 platform:

```sh
arduino-cli core update-index --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core install esp32:esp32@3.3.12 --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli compile --fqbn esp32:esp32:esp32 --jobs 4 --output-dir build firmware/Cyberdeck_Hardware
arduino-cli board list
```

Upload using the serial port shown on your computer, for example replace `YOUR_PORT`:

```sh
arduino-cli upload --fqbn esp32:esp32:esp32 --port YOUR_PORT firmware/Cyberdeck_Hardware
```

No board is automatically selected or flashed by the repository's scripts or CI. The structured alternative is `firmware/cyberdeck/cyberdeck.ino` with its adjacent headers.

## Change configuration

In the structured source, edit [`config.h`](../firmware/cyberdeck/config.h), then regenerate the combined sketch. In the combined sketch, the same Config block is near the beginning.

| Setting | Default |
|---|---|
| AP SSID / password | ESP32-LAB / Lab32-2026 |
| OLED address | 0x3C |
| UP / DOWN / ENTER / BACK | 14 / 13 / 12 / 27 |
| SDA / SCL | 21 / 22 |
| Target frame interval | 83ms, approximately 12fps |
| Drawing / message duration | 20,000ms |
| Power hold | 2,000ms |
| Reduced motion | false |

Keep `DECK_DEMO=0` for real radio and system readings. `DECK_DEMO=1` is an explicitly fictional fixture build; it does not exercise Wi-Fi or HTTP. `DECK_SELF_TEST=1` enables the optional reference-frame/game self-test code; results are not assumed to pass just because fixtures exist.

## Rebuild generated files

With Node.js 20+:

```sh
npm run build
npm test
```

`build:firmware` embeds `web/index.html`, converts the preview bitmap data to a C++ header, regenerates 172 reference frames and combines the production sketch. `build:preview` bundles the offline HTML from source. Neither command uploads or starts a radio.

For the C++ compile-time assertions, use any C++17 compiler or the installed Xtensa compiler:

```sh
g++ -std=c++17 -fsyntax-only tests/power_compile_tests.cpp
```

## Troubleshooting

- **Phone says no internet:** stay connected to ESP32-LAB and use `http://192.168.4.1`. Do not enter 199.168.4.1 or force HTTPS.
- **OLED is blank:** check power, common GND, SDA21/SCL22 and address 0x3C; inspect Serial Monitor. In soft-off mode, hold BACK two seconds to wake.
- **Button remains IDLE:** check both sides of the switch and the actual breadboard column. Match the exact diagram rather than only the wire color.
- **Temporary loss of phone connection while scanning:** AP+STA uses one radio. The page retries its read requests; reconnect if needed.
- **HTTP 423 during startup:** wait for the 10-second startup to finish. HTTP 409 during the game means exit or finish the game before changing the OLED or starting a full scan.

References: [Espressif Arduino Wi-Fi API](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/wifi.html), [Arduino CLI](https://arduino.github.io/arduino-cli/), [official setup-arduino-cli action](https://github.com/arduino/setup-arduino-cli).
