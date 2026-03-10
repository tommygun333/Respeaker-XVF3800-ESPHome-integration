#pragma once

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/number/number.h"
#include "esphome/components/select/select.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/core/hal.h"
#include <cstring>
#include <cmath>

namespace esphome {
namespace respeaker_xvf3800 {

// Forward-declare the main component class
class RespeakerXVF3800;

static const uint8_t REGISTER_CHANNEL_1_STAGE = 0x40;

// Configuration servicer resource IDs
static const uint8_t DFU_CONTROLLER_SERVICER_RESID = 240;
static const uint8_t CONFIGURATION_SERVICER_RESID = 241;
static const uint8_t CONFIGURATION_COMMAND_READ_BIT = 0x80;
static const uint8_t DFU_COMMAND_READ_BIT = 0x80;

static const uint16_t DFU_TIMEOUT_MS = 4000;
static const uint16_t MAX_XFER = 128;  // maximum number of bytes we can transfer per block

// Original XVF3800 constants
const uint8_t GPO_SERVICER_RESID = 20;
const uint8_t GPO_SERVICER_RESID_GPO_READ_VALUES = 0;
const uint8_t GPO_SERVICER_RESID_GPO_WRITE_VALUE = 1;
const uint8_t GPO_SERVICER_RESID_LED_RING_VALUE = 18;
const uint8_t GPO_GPO_READ_NUM_BYTES = 5;

// AEC Azimuth constants for LED beam sensor
const uint8_t AEC_SERVICER_RESID = 33;
const uint8_t AEC_AZIMUTH_VALUES_CMD = 75;

// Fixed Beam Mode (AEC_RESID = 33) — from official respeaker/reSpeaker_XVF3800_USB_4MIC_ARRAY command map
const uint8_t AEC_FIXEDBEAMSONOFF_CMD = 37;      // int32: 0=off, 1=on
const uint8_t AEC_FIXEDBEAMNOISETHR_CMD = 38;    // 2x float: noise threshold per beam [0.0..1.0]
const uint8_t AEC_FIXEDBEAMSAZIMUTH_CMD = 81;    // 2x float(radians): azimuth for beam 1, beam 2
const uint8_t AEC_FIXEDBEAMSELEVATION_CMD = 82;  // 2x float(radians): elevation for beam 1, beam 2
const uint8_t AEC_FIXEDBEAMSGATING_CMD = 83;     // uint8: 0=off, 1=on (speech energy selects active beam)

// AEC Tuning (AEC_RESID = 33)
const uint8_t AEC_HPFONOFF_CMD = 1;           // int32: 0=off, 1=70Hz, 2=125Hz, 3=150Hz, 4=180Hz
const uint8_t AEC_AECEMPHASISONOFF_CMD = 4;   // int32: 0=off, 1=on, 2=on_eq
const uint8_t AEC_FAR_EXTGAIN_CMD = 5;        // float: external gain dB on far-end ref
const uint8_t AEC_ASROUTONOFF_CMD = 35;       // int32: 0=AEC residuals, 1=ASR processed
const uint8_t AEC_ASROUTGAIN_CMD = 36;        // float: gain on ASR output [0.0..1000.0]

// Post-Processing / Noise Suppression (PP_RESID = 17)
const uint8_t PP_RESID = 17;
const uint8_t PP_AGCONOFF_CMD = 10;          // int32: 0=off, 1=on
const uint8_t PP_AGCMAXGAIN_CMD = 11;        // float: max AGC gain [1.0..1000.0]
const uint8_t PP_AGCDESIREDLEVEL_CMD = 12;   // float: target output power [1e-8..1.0]
const uint8_t PP_AGCGAIN_CMD = 13;           // float: current AGC gain [1.0..1000.0]
const uint8_t PP_AGCTIME_CMD = 14;           // float: ramp time [0.5..4.0]s
const uint8_t PP_AGCFASTTIME_CMD = 15;       // float: fast ramp-down [0.05..4.0]s
const uint8_t PP_LIMITONOFF_CMD = 19;        // int32: limiter 0=off, 1=on
const uint8_t PP_LIMITPLIMIT_CMD = 20;       // float: max limiter power [1e-8..1.0]
const uint8_t PP_MIN_NS_CMD = 21;            // float: stationary noise suppression gain-floor [0.0..1.0]
const uint8_t PP_MIN_NN_CMD = 22;            // float: non-stationary noise suppression gain-floor [0.0..1.0]
const uint8_t PP_ECHOONOFF_CMD = 23;         // int32: echo suppression 0=off, 1=on
const uint8_t PP_GAMMA_E_CMD = 24;           // float: echo over-subtraction direct [0.0..2.0]
const uint8_t PP_GAMMA_ETAIL_CMD = 25;       // float: echo over-subtraction tail [0.0..2.0]
const uint8_t PP_GAMMA_ENL_CMD = 26;         // float: non-linear echo over-subtraction [0.0..5.0]
const uint8_t PP_NLATTENONOFF_CMD = 27;      // int32: non-linear echo attenuation 0=off, 1=on
const uint8_t PP_DTSENSITIVE_CMD = 31;       // int32: echo vs doubletalk [0..5, 10..15]
const uint8_t PP_ATTNS_MODE_CMD = 32;        // int32: non-speech AGC reduction 0=off, 1=on
const uint8_t PP_ATTNS_NOMINAL_CMD = 33;     // float: non-speech attenuation [0.0..1.0]

// Audio Manager (AUDIO_MGR_RESID = 35)
const uint8_t AUDIO_MGR_RESID = 35;
const uint8_t AUDIO_MGR_MIC_GAIN_CMD = 0;   // float: pre-SHF mic gain
const uint8_t AUDIO_MGR_REF_GAIN_CMD = 1;   // float: pre-SHF reference gain
const uint8_t AUDIO_MGR_SYS_DELAY_CMD = 26; // int32: system delay in samples

const uint8_t RESID_LED = 0x0C;
const uint8_t RESID_DFU_VERSION = 0xFE;
const uint8_t I2C_COMMAND_READ_BIT = 0x80;

enum TransportProtocolReturnCode : uint8_t {
  CTRL_DONE = 0,
  CTRL_WAIT = 1,
  CTRL_INVALID = 3,
};

enum RespeakerXVF3800UpdaterStatus : uint8_t {
  UPDATE_OK,
  UPDATE_COMMUNICATION_ERROR,
  UPDATE_READ_VERSION_ERROR,
  UPDATE_TIMEOUT,
  UPDATE_FAILED,
  UPDATE_BAD_STATE,
  UPDATE_IN_PROGRESS,
  UPDATE_REBOOT_PENDING,
  UPDATE_VERIFY_NEW_VERSION,
};

// Configuration enums from the XMOS firmware's src/configuration/configuration_servicer.h
enum ConfCommands : uint8_t {
  CONFIGURATION_SERVICER_RESID_VNR_VALUE = 0x00,
};

// DFU enums from https://github.com/xmos/sln_voice/blob/develop/examples/ffva/src/dfu_int/dfu_state_machine.h
enum DfuIntAltSetting : uint8_t {
  DFU_INT_ALTERNATE_FACTORY,
  DFU_INT_ALTERNATE_UPGRADE,
};

enum DfuIntState : uint8_t {
  DFU_INT_APP_IDLE,    // unused
  DFU_INT_APP_DETACH,  // unused
  DFU_INT_DFU_IDLE,
  DFU_INT_DFU_DNLOAD_SYNC,
  DFU_INT_DFU_DNBUSY,
  DFU_INT_DFU_DNLOAD_IDLE,
  DFU_INT_DFU_MANIFEST_SYNC,
  DFU_INT_DFU_MANIFEST,
  DFU_INT_DFU_MANIFEST_WAIT_RESET,
  DFU_INT_DFU_UPLOAD_IDLE,
  DFU_INT_DFU_ERROR,
};

enum DfuIntStatus : uint8_t {
  DFU_INT_DFU_STATUS_OK,
  DFU_INT_DFU_STATUS_ERR_TARGET,
  DFU_INT_DFU_STATUS_ERR_FILE,
  DFU_INT_DFU_STATUS_ERR_WRITE,
  DFU_INT_DFU_STATUS_ERR_ERASE,
  DFU_INT_DFU_STATUS_ERR_CHECK_ERASED,
  DFU_INT_DFU_STATUS_ERR_PROG,
  DFU_INT_DFU_STATUS_ERR_VERIFY,
  DFU_INT_DFU_STATUS_ERR_ADDRESS,
  DFU_INT_DFU_STATUS_ERR_NOTDONE,
  DFU_INT_DFU_STATUS_ERR_FIRMWARE,
  DFU_INT_DFU_STATUS_ERR_VENDOR,
  DFU_INT_DFU_STATUS_ERR_USBR,
  DFU_INT_DFU_STATUS_ERR_POR,
  DFU_INT_DFU_STATUS_ERR_UNKNOWN,
  DFU_INT_DFU_STATUS_ERR_STALLEDPKT,
};

enum DfuCommands : uint8_t {
  DFU_CONTROLLER_SERVICER_RESID_DFU_DETACH = 0,
  DFU_CONTROLLER_SERVICER_RESID_DFU_DNLOAD = 1,
  DFU_CONTROLLER_SERVICER_RESID_DFU_UPLOAD = 2,
  DFU_CONTROLLER_SERVICER_RESID_DFU_GETSTATUS = 3,
  DFU_CONTROLLER_SERVICER_RESID_DFU_CLRSTATUS = 4,
  DFU_CONTROLLER_SERVICER_RESID_DFU_GETSTATE = 5,
  DFU_CONTROLLER_SERVICER_RESID_DFU_ABORT = 6,
  DFU_CONTROLLER_SERVICER_RESID_DFU_SETALTERNATE = 64,
  DFU_CONTROLLER_SERVICER_RESID_DFU_TRANSFERBLOCK = 65,
  DFU_CONTROLLER_SERVICER_RESID_DFU_GETVERSION = 88,
  DFU_CONTROLLER_SERVICER_RESID_DFU_REBOOT = 89,
};

enum DFUAutomationState {
  DFU_COMPLETE = 0,
  DFU_START,
  DFU_IN_PROGRESS,
  DFU_ERROR,
};

// --- Component Classes ---

// MuteSwitch class that handles the mute functionality
class MuteSwitch : public switch_::Switch, public PollingComponent {
 public:
  void set_parent(RespeakerXVF3800 *parent) {
    parent_ = parent;
    ESP_LOGD("respeaker_xvf3800", "MuteSwitch parent set to %p", parent);
  }
  void setup() override;
  void write_state(bool state) override;
  void update() override;
  void dump_config() override;

 protected:
  RespeakerXVF3800 *parent_{nullptr};
};

// DFUVersionTextSensor class to read the firmware version
class DFUVersionTextSensor : public text_sensor::TextSensor, public PollingComponent {
 public:
  void set_parent(RespeakerXVF3800 *parent) { parent_ = parent; }
  void setup() override;
  void update() override;
  void dump_config() override;

 protected:
  RespeakerXVF3800 *parent_{nullptr};
};

// LEDBeamSensor class to read the active LED beam direction
class LEDBeamSensor : public sensor::Sensor, public PollingComponent {
 public:
  void set_parent(RespeakerXVF3800 *parent) { parent_ = parent; }
  void setup() override;
  void update() override;
  void dump_config() override;

 protected:
  RespeakerXVF3800 *parent_{nullptr};
};

// --- Main Hub Class ---

class RespeakerXVF3800 : public i2c::I2CDevice, public Component {
 public:
  void setup() override;
  bool can_proceed() override {
    return this->is_failed() || (this->version_read_() && (this->versions_match_() || !this->firmware_bin_is_valid_()));
  }
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::HARDWARE - 1; }
  void loop() override;

#ifdef USE_RESPEAKER_XVF3800_STATE_CALLBACK
  void add_on_state_callback(std::function<void(DFUAutomationState, float, RespeakerXVF3800UpdaterStatus)> &&callback) {
    this->state_callback_.add(std::move(callback));
  }
#endif

  void set_reset_pin(GPIOPin *reset_pin) { reset_pin_ = reset_pin; }

  void set_firmware_bin(const uint8_t *data, const uint32_t len) {
    this->firmware_bin_ = data;
    this->firmware_bin_length_ = len;
  }

  void set_mute_state(binary_sensor::BinarySensor* mute_state) {
    this->mute_state_ = mute_state;
  }

  void set_firmware_version(text_sensor::TextSensor* firmware_version) {
    this->firmware_version_ = firmware_version;
  }
  
  void set_firmware_version(uint8_t major, uint8_t minor, uint8_t patch) {
    this->firmware_bin_version_major_ = major;
    this->firmware_bin_version_minor_ = minor;
    this->firmware_bin_version_patch_ = patch;
  }

  void start_dfu_update();
  uint8_t read_vnr();

  // Public methods for child components
  bool read_gpo_values(uint8_t *buffer, uint8_t *status);
  bool read_gpio_status(uint32_t *gpio_status);
  bool read_mute_status();
  void write_mute_status(bool value);
  
  // Individual LED ring control (12 LEDs)
  void set_led_ring(uint32_t *rgb_array);
  
  std::string read_dfu_version();
  
  // Read LED beam direction (0-11)
  int read_led_beam_direction();

  // DSP optimization methods
  void set_noise_suppression_level(uint8_t level);   // 0=off, 1=low, 2=medium, 3=high, 4=max (maps to PP_MIN_NS)

  // Fixed Beam Mode
  void set_fixed_beams_enabled(bool enabled);
  void set_fixed_beam_azimuths(float beam1_degrees, float beam2_degrees);
  void set_fixed_beam_gating(bool enabled);
  void set_fixed_beam_noise_threshold(float beam1_thr, float beam2_thr);

  // AEC Tuning
  void set_hpf_mode(uint8_t mode);        // 0=off, 1=70Hz, 2=125Hz, 3=150Hz, 4=180Hz
  void set_aec_emphasis(uint8_t mode);    // 0=off, 1=on, 2=on_eq
  void set_far_end_gain(float gain_db);
  void set_asr_output(bool enabled);
  void set_asr_output_gain(float gain);

  // Post-Processing
  void set_agc_enabled(bool enabled);
  void set_agc_max_gain(float gain);
  void set_agc_desired_level(float level);
  void set_pp_limiter(bool enabled);
  void set_pp_limiter_level(float level);
  void set_ns_gain_floor(float value);    // PP_MIN_NS: stationary noise suppression gain-floor
  void set_nn_gain_floor(float value);    // PP_MIN_NN: non-stationary noise suppression gain-floor
  void set_echo_suppression(bool enabled);
  void set_echo_gamma_e(float value);
  void set_echo_gamma_etail(float value);
  void set_echo_gamma_enl(float value);
  void set_nl_attenuation(bool enabled);
  void set_dt_sensitive(uint8_t value);   // 0-5 or 10-15
  void set_attns_mode(bool enabled);
  void set_attns_nominal(float value);

  // Audio Manager
  void set_mic_gain(float gain);
  void set_ref_gain(float gain);
  void set_sys_delay(int32_t samples);

  // Setters for child components
  void set_mute_switch(MuteSwitch *mute_switch) { mute_switch_ = mute_switch; }
  void set_dfu_version_sensor(DFUVersionTextSensor *dfu_version_sensor) { dfu_version_sensor_ = dfu_version_sensor; }
  void set_led_beam_sensor(LEDBeamSensor *led_beam_sensor) { led_beam_sensor_ = led_beam_sensor; }

 protected:
#ifdef USE_RESPEAKER_XVF3800_STATE_CALLBACK
  CallbackManager<void(DFUAutomationState, float, RespeakerXVF3800UpdaterStatus)> state_callback_{};
#endif
  RespeakerXVF3800UpdaterStatus dfu_update_send_block_();
  uint32_t load_buf_(uint8_t *buf, const uint8_t max_len, const uint32_t offset);
  bool firmware_bin_is_valid_() { return this->firmware_bin_ != nullptr && this->firmware_bin_length_; }
  bool version_read_();
  bool versions_match_();

  bool dfu_get_status_();
  bool dfu_get_version_();
  bool dfu_reboot_();
  bool dfu_set_alternate_();
  bool dfu_check_if_ready_();

  GPIOPin *reset_pin_{nullptr};
  binary_sensor::BinarySensor *mute_state_{nullptr};
  text_sensor::TextSensor *firmware_version_{nullptr};

  bool get_firmware_version_();

  uint8_t dfu_state_{0};
  uint8_t dfu_status_{0};
  uint32_t dfu_status_next_req_delay_{0};

  uint8_t const *firmware_bin_{nullptr};
  uint32_t firmware_bin_length_{0};
  uint8_t firmware_bin_version_major_{0};
  uint8_t firmware_bin_version_minor_{0};
  uint8_t firmware_bin_version_patch_{0};

  uint8_t firmware_version_major_{0};
  uint8_t firmware_version_minor_{0};
  uint8_t firmware_version_patch_{0};

  uint32_t bytes_written_{0};
  uint32_t last_progress_{0};
  uint32_t last_ready_{0};
  uint32_t status_last_read_ms_{0};
  uint32_t update_start_time_{0};
  RespeakerXVF3800UpdaterStatus dfu_update_status_{UPDATE_OK};

  // Child components
  MuteSwitch *mute_switch_{nullptr};
  DFUVersionTextSensor *dfu_version_sensor_{nullptr};
  LEDBeamSensor *led_beam_sensor_{nullptr};
  
  // Helper method for XMOS communication
  void xmos_write_bytes(uint8_t resid, uint8_t cmd, uint8_t *value, uint8_t write_byte_num);

  // DSP configuration helper (called from setup when firmware is current)
  void configure_dsp_();
};

}  // namespace respeaker_xvf3800
}  // namespace esphome
