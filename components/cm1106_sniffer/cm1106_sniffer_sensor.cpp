#include "cm1106_sniffer_sensor.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "esphome/components/uart/uart_component.h"
#include <string.h>
#include <vector>
#include <algorithm>

namespace esphome {
namespace cm1106_sniffer {

static const char *const TAG = "cm1106_sniffer";

void CM1106SnifferSensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up CM1106 Sniffer Sensor...");
}

void CM1106SnifferSensor::loop() {
  while (this->available()) {
    uint8_t byte;
    this->read_byte(&byte);
    this->handle_byte(byte);
  }
}

void CM1106SnifferSensor::handle_byte(uint8_t byte) {
  if (this->buffer_pos_ == 0 && byte != 0x16) {
    return;
  }
  
  this->buffer_[this->buffer_pos_++] = byte;

  if (this->buffer_pos_ < 9) {
    return;
  }

  if (this->buffer_[0] != 0x16) {
    ESP_LOGW(TAG, "Invalid start byte: 0x%02X", this->buffer_[0]);
    this->reset_buffer_();
    return;
  }

  uint8_t checksum = 0;
  for (int i = 0; i < 8; ++i) {
    checksum += this->buffer_[i];
  }
  checksum = 0xFF - checksum;
  checksum += 1;

  if (this->buffer_[8] != checksum) {
    ESP_LOGW(TAG, "Checksum mismatch: calculated 0x%02X, received 0x%02X", checksum, this->buffer_[8]);
    this->reset_buffer_();
    return;
  }

  uint16_t co2_value = (uint16_t) this->buffer_[3] << 8 | this->buffer_[4];
  
  if (this->co2_sensor_ != nullptr) {
    this->co2_sensor_->publish_state(co2_value);
    ESP_LOGD(TAG, "CO2 value: %d ppm", co2_value);
  }
  
  this->reset_buffer_();
}

void CM1106SnifferSensor::reset_buffer_() {
  this->buffer_pos_ = 0;
  std::fill(this->buffer_, this->buffer_ + 9, 0);
}

void CM1106SnifferSensor::dump_config() {
  LOG_COMPONENT_CONFIG(TAG, "CM1106 Sniffer");
  LOG_UART_DEVICE(this);
  if (this->co2_sensor_ != nullptr) {
    LOG_SENSOR("  ", "CO2", this->co2_sensor_);
  }
}

void CM1106SnifferSensor::update() {
  const uint8_t request_command[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  this->write_array(request_command, 9);
}

}  // namespace cm1106_sniffer
}  // namespace esphome
