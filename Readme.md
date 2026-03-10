# ESPHome components and example config for [Respeaker XVF3800](https://github.com/respeaker/reSpeaker_XVF3800_USB_4MIC_ARRAY)

## _ATTENTION! Under development, use on your own risk!_
## Testing appreciated.

## What's New (this fork)

### Bug Fixes
1. **Voice cut-off fix** — A 500 ms guard timer has been added to the media player `on_idle` handler.  
   The guard prevents the device from transitioning out of the REPLYING phase during brief pauses between sentences in a multi-sentence TTS response.  
   The `control_leds` master script mode has been changed from `single` to `restart` so that rapid back-to-back state changes are never silently dropped.
2. **Security fix** — Hardcoded Wi-Fi credentials and OTA password have been replaced with `!secret` references.  
   Create (or update) your `secrets.yaml` with:
   ```yaml
   wifi_ssid: "YourNetworkName"
   wifi_password: "YourWiFiPassword"
   ota_password: "YourOTAPassword"
   ```

### XVF3800 DSP Optimisations
The XVF3800 chip has a powerful built-in DSP pipeline (AEC, noise suppression, interference tracking, beamforming). The firmware now sends optimised I2C configuration commands on boot:

- **Noise suppression** is enabled at *High* level (configurable via Home Assistant).
- **Interference tracker (IT)** is enabled — it detects and suppresses a point noise source such as a TV or radio.
- **VNR threshold** is lowered to improve wake-word detection in noisy rooms.

#### I2C speed
The I2C bus speed has been increased from 100 kHz to 400 kHz (Fast Mode) for snappier LED updates and faster DSP parameter changes.

### New Home Assistant Entities

| Entity | Type | Description |
|--------|------|-------------|
| Noise Suppression Level | Select | Controls XVF3800 noise suppression: Low / Medium / High / Maximum |
| Interference Rejection Angle | Number (0–360 °) | Steers the XVF3800 null-beamformer towards a stationary noise source (e.g. TV). 0 ° = front of the device. |

> **Note:** The exact I2C resource and command IDs for the noise suppression and interference tracking features are derived from the XVF3800 firmware command map.  
> If your firmware version reports unexpected behaviour, you can extract the authoritative command map using [xvf_extract_command_info](https://github.com/arjanwestveld/xvf_extract_command_info) and update the constants in `respeaker_xvf3800.h`.

## Known issues:
1. There's no buttons, so no way to stop timer or response except saying "stop", and no way to start pipeline manually.
2. No volume controls besides the software one (similar to Respeaker Lite Voice Kit).
3. No exposed light (LED is controlled via I2C).
4. ...?
