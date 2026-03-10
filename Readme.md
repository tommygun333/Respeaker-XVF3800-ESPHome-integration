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

### 🎯 Fixed Beam Mode (replaces Interference Rejection Angle)
The XVF3800 has two steerable beams that can be pointed toward specific directions where speakers usually sit. This replaces the old "Interference Rejection Angle" approach (which wrote to the wrong command — CMD 74 on RESID 33 is `AEC_MIC_ARRAY_GEO`, a read-only mic geometry command).

**How Fixed Beam Mode works:**
1. Enable **Fixed Beams** — tells the DSP to use fixed beam directions instead of adaptive tracking
2. Set **Fixed Beam 1 Angle** and **Fixed Beam 2 Angle** (0–360°) to point at where speakers usually sit (e.g., 315° and 45° to cover a 90° arc centered on 0°)
3. Enable **Fixed Beam Gating** — the DSP automatically selects whichever beam detects speech energy; inactive beams are silenced
4. Adjust **Fixed Beam Noise Threshold** — controls sensitivity of the beam selection

> **Tip:** The XVF3800 does not expose a "beam width" parameter — the width is determined by the 4-mic circular array geometry (~45–90° depending on frequency). To cover a wider area, space the two beams further apart.

All Fixed Beam commands use the official command IDs from [respeaker/reSpeaker_XVF3800_USB_4MIC_ARRAY](https://github.com/respeaker/reSpeaker_XVF3800_USB_4MIC_ARRAY/blob/4b1ed0aefea83e51c328c735d7d9c3b953afae24/python_control/xvf_host.py).

### 🎤 XVF3800 DSP Optimisations
The XVF3800 chip's built-in DSP features are now configured via I2C after boot for far-field voice capture in noisy environments, using the **correct official command IDs** from the XMOS/Seeed command map (RESID 17 = Post-Processing, RESID 33 = AEC, RESID 35 = Audio Manager):

| Feature | Default | Effect |
|---|---|---|
| AGC | Enabled | Auto-levels voice volume |
| Stationary NS gain-floor | 0.1 | Aggressively suppresses steady background noise |
| Non-Stationary NS gain-floor | 0.1 | Suppresses transient noise bursts |
| Echo Suppression | Enabled | Cancels speaker output from mic |

> **Note:** The previous `configure_dsp_()` used guessed RESID/CMD values (RESID 4 for NS and IT, RESID 5 for VNR) that do not appear in the official command map. These have been replaced with the correct `PP_RESID=17` commands.

### ⚡ I2C Speed Increase
I2C bus frequency increased from 100 kHz to 400 kHz (Fast Mode) for faster LED and DSP parameter updates.

### 🏠 New Home Assistant Entities

All entities are under `entity_category: config` with `optimistic: true` and `restore_value: true` so settings survive reboots.

#### Fixed Beam Controls
| Entity | Type | Values | Description |
|---|---|---|---|
| Fixed Beams | Switch | On/Off | Enables fixed directional beam mode |
| Fixed Beam 1 Angle | Number | 0–360°, step 5° | Direction for beam 1 |
| Fixed Beam 2 Angle | Number | 0–360°, step 5°, default 180° | Direction for beam 2 |
| Fixed Beam Gating | Switch | On/Off (default On) | Auto-selects active beam by speech energy |
| Fixed Beam Noise Threshold | Number | 0.0–1.0, step 0.05 | Noise suppressor update threshold per beam |

#### AEC / ASR Tuning
| Entity | Type | Values | Description |
|---|---|---|---|
| High-Pass Filter | Select | Off, 70Hz, 125Hz, 150Hz, 180Hz | Removes low-frequency rumble from mic input |
| AEC Emphasis | Select | Off, On, On+EQ | Pre/de-emphasis filtering for AEC |
| ASR Output | Switch | On/Off | Switch between AEC residuals and ASR-processed output |
| ASR Output Gain | Number | 0.0–100.0, step 0.5, initial 1.0 | Gain on ASR output |
| Mic Gain | Number | 0.0–200.0, step 1.0, initial 90.0 | Pre-SHF mic gain (AUDIO_MGR) |
| Reference Gain | Number | 0.0–200.0, step 0.5, initial 8.0 | Pre-SHF reference gain (AUDIO_MGR) |
| System Delay | Number | 0–100 samples, step 1, initial 12 | AEC reference alignment delay |

#### Post-Processing / Noise Suppression
| Entity | Type | Values | Description |
|---|---|---|---|
| Noise Suppression Level | Select | Low, Medium, High, Maximum | Convenience preset mapping to PP_MIN_NS/PP_MIN_NN gain-floor |
| Stationary Noise Suppression | Number | 0.0–1.0, step 0.01, initial 0.1 | PP_MIN_NS gain-floor — lower = more aggressive |
| Non-Stationary Noise Suppression | Number | 0.0–1.0, step 0.01, initial 0.1 | PP_MIN_NN gain-floor — lower = more aggressive |
| AGC | Switch | On/Off (default On) | Automatic Gain Control |
| AGC Max Gain | Number | 1.0–1000.0, step 1.0, initial 64.0 | Maximum AGC boost |
| AGC Desired Level | Number | 0.0–1.0, step 0.01, initial 0.1 | Target output power level |
| Output Limiter | Switch | On/Off (default On) | Hard limiter to prevent clipping |
| Limiter Power | Number | 0.0–1.0, step 0.01, initial 0.5 | Maximum limiter power |
| Echo Suppression | Switch | On/Off (default On) | Post-processing echo suppression |
| Echo Gamma E | Number | 0.0–2.0, step 0.1, initial 1.0 | Echo over-subtraction (direct path) |
| Echo Gamma E Tail | Number | 0.0–2.0, step 0.1, initial 1.0 | Echo over-subtraction (tail) |
| Echo Gamma ENL | Number | 0.0–5.0, step 0.1, initial 1.0 | Non-linear echo over-subtraction |
| NL Echo Attenuation | Switch | On/Off (default On) | Non-linear echo attenuation |
| Doubletalk Sensitivity | Select | 0–5, 10–15, initial 3 | Echo vs doubletalk tradeoff |
| Non-Speech Attenuation | Switch | On/Off (default Off) | Extra AGC reduction during non-speech |
| Non-Speech Attenuation Level | Number | 0.0–1.0, step 0.05, initial 0.5 | Amount of non-speech attenuation |

### 📦 ESPHome 2026.2.4 Compatibility
- Version bumped to `2026.2.4` / `min_version: 2026.2.0`
- Removed `external_components` entries for `mixer` and `resampler` — these are now merged into ESPHome 2026.2.0 core
- Updated commit SHA refs for external components `audio`, `media_player`, `sendspin`, `mdns`, and `file`/`http_request`/`media_source`/`speaker_source` to match the upstream formatBCE 2026.2.0 update
- `respeaker_xvf3800` and `aic3104` components now pulled from **this fork** (`tommygun333/Respeaker-XVF3800-ESPHome-integration`) so that the DSP features (noise suppression, interference tracking) are actually used

### 🚀 ESPHome Device Builder Support
Added `config/respeaker-xvf-device-builder.yaml` — a small stub file you can drop into your ESPHome config directory. It uses `packages:` to pull the full config from this repo, making it easy to switch between branches without copy-pasting the entire config each time.

## Known issues:
1. There's no buttons, so no way to stop timer or response except saying "stop", and no way to start pipeline manually.
2. No volume controls besides the software one (similar to Respeaker Lite Voice Kit).
3. No exposed light (LED is controlled via I2C).
4. ...?
