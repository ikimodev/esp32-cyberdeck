# Third-party material

The original application code, documentation, procedural city drawing and project tooling are offered under [MIT](LICENSE).

## Adafruit bitmap font

The classic 5×7 font data is derived from Adafruit GFX. Keep the complete [Adafruit BSD notice](LICENSES/Adafruit-GFX-BSD.txt) with redistributed source and binaries. The font is embedded in `preview/base/assets.mjs`, `firmware/cyberdeck/deck_assets.h`, the generated sketch and the wiring-test sketch.

## Cyberpunk reference logo

The `logo` and `smallLogo` bitmaps in the asset files are based on a reference supplied for this personal project. They reproduce a Cyberpunk logo and are **excluded from the MIT license**. This repository does not grant permission to that third-party logo or imply endorsement. Its redistribution rights have not been established by the project.

For a distribution requiring every graphical asset to have an established open license, replace those two bitmaps with original artwork before redistributing the corresponding firmware and preview. The rendering code does not depend on that specific logo: keep bitmap width, height, stride and packed data consistent, then run `npm run build`.

Cyberpunk names and visual references identify the inspiration only. This is an independent hobby project.

## Screenshots and animation

The preview images and GIF are generated from the project's renderer and contain fictional network data. The same logo exception applies to logo pixels visible in these images. The breadboard images show the project's Wokwi wiring layout; they are not evidence of a full-firmware simulation run.
