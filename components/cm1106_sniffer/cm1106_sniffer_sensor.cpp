#include "cm1106_sniffer_sensor.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace cm1106_sniffer {

static const char *const TAG = "cm1106_sniffer";

void CM1106Sniffer::setup() {
  ESP_LOGCONFIG(TAG, "Setting up CM1106 Sniffer...");
  this->reset_buffer_();
}

void CM1106Sniffer::loop() {
  while (this->available()) {
    uint8_t byte;
    this->read_byte(&byte);
    this->handle_byte(byte);
  }
}

void CM1106Sniffer::update() {
  // Send the command to the sensor to request a reading.
  // The sensor's response will be read in the loop() function.
  const uint8_t request_command[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  this->write_array(request_command, 9);
}

void CM1106Sniffer::handle_byte(uint8_t byte) {
  if (this->buffer_pos_ == 0 && byte != 0xFF) {
    return;
  }
  this->buffer_[this->buffer_pos_++] = byte;
  if (this->buffer_pos_ < 9) {
    return;
  }

  uint8_t checksum = 0;
  for (int i = 1; i < 8; i++) {
    checksum += this->buffer_[i];
  }
  checksum = 0xFF - checksum;
  checksum += 1;

  if (this->buffer_[8] != checksum) {
    ESP_LOGW(TAG, "Checksum mismatch: calculated 0x%02X, received 0x%02X", checksum, this->buffer_[8]);
    this->reset_buffer_();
    return;
  }

  uint16_t co2_value = (uint16_t) this->buffer_[2] << 8 | this->buffer_[3];
  if (this->co2_sensor_ != nullptr) {
    this->co2_sensor_->publish_state(co2_value);
    ESP_LOGD(TAG, "CO2 value: %u ppm", co2_value);
  }

  this->reset_buffer_();
}

void CM1106Sniffer::reset_buffer_() {
  this->buffer_pos_ = 0;
}

void CM1106Sniffer::dump_config() {
  ESP_LOGCONFIG(TAG, "CM1106 Sniffer");
  this->dump_polling_component_config(TAG);
  if (this->co2_sensor_ != nullptr) {
    LOG_SENSOR("  CO2 Sensor", "CO2", this->co2_sensor_);
  }
}

} // namespace cm1106_sniffer
} // namespace esphome
