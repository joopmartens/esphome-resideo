#include "cm1106_sniffer_sensor.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace cm1106_sniffer {

static const char *const TAG = "cm1106_sniffer";

void CM1106Sniffer::setup() {
  ESP_LOGCONFIG(TAG, "Setting up CM1106 Sniffer Sensor...");
  if (this->uart_component_ == nullptr) {
    ESP_LOGE(TAG, "UART component not set!");
    return;
  }
  this->uart_component_->flush();
}

void CM1106Sniffer::loop() {
  if (this->uart_component_ == nullptr) {
    return;
  }
  while (this->uart_component_->available()) {
    uint8_t byte;
    this->uart_component_->read_byte(&byte);
    this->handle_byte(byte);
  }
}

void CM1106Sniffer::handle_byte(uint8_t byte) {
  if (this->buffer_pos_ == 0 && byte != 0x16) {
    return;
  }
  
  this->buffer_[this->buffer_pos_++] = byte;

  if (this->buffer_pos_ < 9) {
    return;
  }

  // Log the received frame for debugging
  ESP_LOGD(TAG, "Received frame: %s", format_hex_pretty(this->buffer_, 9).c_str());

  if (this->buffer_[0] != 0x16) {
    ESP_LOGW(TAG, "Invalid start byte: 0x%02X", this->buffer_[0]);
    this->reset_buffer_();
    return;
  }

  uint8_t checksum = 0;
  // Checksum is the two's complement of the sum of bytes 1 through 7.
  for (int i = 1; i < 8; ++i) {
    checksum += this->buffer_[i];
  }
  checksum = 0xFF - checksum + 1;
  
  if (this->buffer_[8] != checksum) {
    ESP_LOGW(TAG, "Checksum mismatch: calculated 0x%02X, received 0x%02X", checksum, this->buffer_[8]);
    this->reset_buffer_();
    return;
  }

  uint16_t co2_value = (uint16_t) this->buffer_[3] << 8 | this->buffer_[4];
  
  // Publish the new state directly from the handle_byte function
  this->publish_state(co2_value);
  ESP_LOGD(TAG, "CO2 value: %d ppm", co2_value);
  
  this->reset_buffer_();
}

void CM1106Sniffer::dump_config() {
  // Use the base class methods to dump all configuration properties
  PollingComponent::dump_config();
  sensor::Sensor::dump_config();
}

void CM1106Sniffer::update() {
  // Only send the command to request data from the sensor
  const uint8_t request_command[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  this->uart_component_->write_array(request_command, 9);
}

void CM1106Sniffer::reset_buffer_() {
  this->buffer_pos_ = 0;
}

}  // namespace cm1106_sniffer
}  // namespace esphome
