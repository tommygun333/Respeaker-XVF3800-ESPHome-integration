# ESPHome components and example config for [Respeaker XVF3800](https://github.com/respeaker/reSpeaker_XVF3800_USB_4MIC_ARRAY)

## _ATTENTION! Under development, use on your own risk!_
## Testing appreciated.

## What's New (this fork)

This fork of [formatBCE/Respeaker-XVF3800-ESPHome-integration](https://github.com/formatBCE/Respeaker-XVF3800-ESPHome-integration) adds the following improvements:

### 🔇 Voice Cut-off Fix
Added a 500 ms guard timer in the media player `on_idle` callback to prevent premature phase transitions during multi-sentence TTS responses. Brief pauses between TTS sentences no longer cause the device to reset to idle. The `control_leds` script mode was also changed from `single` to `restart` so rapid back-to-back state changes are never silently dropped.

### 🔒 Security Fix (Secrets)
Hardcoded WiFi credentials (`ssid`, `password`) and OTA password have been replaced with `!secret` references. Create a `secrets.yaml` file alongside the config with:
```yaml
wifi_ssid: "YourWifiNetwork"
wifi_password: "YourWifiPassword"
ota_password: "YourOtaPassword"
```

### 🎤 XVF3800 DSP Optimisations
The XVF3800 chip's built-in DSP features are now configured via I2C after boot for far-field voice capture in noisy environments:

| Feature | Setting | Effect |
|---|---|---|
| Interference Tracker (IT) | Enabled | Tracks and suppresses steady noise sources (TV, radio) |
| Noise Suppression (NS) | High (level 3) | Reduces broadband background noise |
| VNR Threshold | 80 (lowered) | More sensitive to voice in noisy conditions |

> **Note:** The exact RESID/CMD values for NS, IT, and VNR were derived from the XVF3800 firmware. To verify or explore available commands on your device, use [xvf_extract_command_info](https://github.com/arjanwestveld/xvf_extract_command_info).

### ⚡ I2C Speed Increase
I2C bus frequency increased from 100 kHz to 400 kHz (Fast Mode) for faster LED and DSP parameter updates.

### 🏠 New Home Assistant Entities

| Entity | Type | Description |
|---|---|---|
| Noise Suppression Level | Select | Low / Medium / High / Maximum — adjusts NS aggressiveness at runtime |
| Interference Rejection Angle | Number | 0–360°, step 15° — steers the null beam toward your TV/radio for better rejection |

### 📦 ESPHome 2026.2.4 Compatibility
- Version bumped to `2026.2.4` / `min_version: 2026.2.0`
- Removed `external_components` entries for components now merged into ESPHome 2026.2.0 core (`mixer`, `resampler`, `audio`, `media_player`, `mdns`, `sendspin`, `file`, `http_request`, `media_source`, `speaker_source`)

## Known issues:
1. There's no buttons, so no way to stop timer or response except saying "stop", and no way to start pipeline manually.
2. No volume controls besides the software one (similar to Respeaker Lite Voice Kit).
3. No exposed light (LED is controlled via I2C).
4. ...?
