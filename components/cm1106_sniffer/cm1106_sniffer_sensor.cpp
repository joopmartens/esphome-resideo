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

  if (this->buffer_[0] != 0x16) {
    ESP_LOGW(TAG, "Invalid start byte: 0x%02X", this->buffer_[0]);
    this->reset_buffer_();
    return;
  }

  uint8_t checksum = 0;
  for (int i = 0; i < 8; ++i) {
    checksum += this->buffer_[i];
  }
  checksum = 0xFF - checksum + 1;

  if (this->buffer_[8] != checksum) {
    ESP_LOGW(TAG, "Checksum mismatch: calculated 0x%02X, received 0x%02X", checksum, this->buffer_[8]);
    this->reset_buffer_();
    return;
  }

  uint16_t co2_value = (uint16_t) this->buffer_[3] << 8 | this->buffer_[4];
  
  this->publish_state(co2_value);
  ESP_LOGD(TAG, "CO2 value: %d ppm", co2_value);
  
  this->reset_buffer_();
}

void CM1106Sniffer::dump_config() {
  LOG_SENSOR("CM1106Sniffer", "CM1106Sniffer", this);
  LOG_UART_SETTINGS(this->uart_component_);
}

}  // namespace cm1106_sniffer
}  // namespace esphome
