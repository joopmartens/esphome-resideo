#include "cm1106_sniffer_sensor.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace cm1106_sniffer {

static const char *const TAG = "cm1106_sniffer";

void CM1106Sniffer::setup() {
  ESP_LOGCONFIG(TAG, "Setting up CM1106 Sniffer Sensor...");
  // Clear any data in the UART buffer
  this->uart_device_->flush();
}

void CM1106Sniffer::loop() {
  // Read any available data from the UART bus
  while (this->available()) {
    uint8_t byte;
    this->read_byte(&byte);
    this->handle_byte(byte);
  }
}

void CM1106Sniffer::handle_byte(uint8_t byte) {
  // This is a simple state machine to parse the CM1106 protocol.
  // The CM1106 sends a 9-byte packet.
  // Byte 0: 0x16 (Start byte)
  // Byte 1: 0x01 (Command)
  // Byte 2: 0x02 (Payload length)
  // Byte 3: High byte of CO2 value
  // Byte 4: Low byte of CO2 value
  // Byte 5-7: Reserved/ignored
  // Byte 8: Checksum
  
  if (this->buffer_pos_ == 0 && byte != 0x16) {
    // We are at the start of a packet, but the byte is not the start byte.
    // Discard and wait for the correct start byte.
    return;
  }
  
  this->buffer_[this->buffer_pos_++] = byte;

  if (this->buffer_pos_ < 9) {
    // Wait for the full 9-byte packet to arrive.
    return;
  }

  // We have a full packet.
  if (this->buffer_[0] != 0x16) {
    // This should not happen if the start byte check works, but as a safeguard
    ESP_LOGW(TAG, "Invalid start byte: 0x%02X", this->buffer_[0]);
    this->reset_buffer_();
    return;
  }

  // Calculate the checksum
  uint8_t checksum = 0;
  for (int i = 0; i < 8; ++i) {
    checksum += this->buffer_[i];
  }
  checksum = 0xFF - checksum;
  checksum += 1;

  if (this->buffer_[8] != checksum) {
    // Checksum mismatch
    ESP_LOGW(TAG, "Checksum mismatch: calculated 0x%02X, received 0x%02X", checksum, this->buffer_[8]);
    this->reset_buffer_();
    return;
  }

  // Extract the CO2 value
  uint16_t co2_value = (uint16_t) this->buffer_[3] << 8 | this->buffer_[4];
  
  // Publish the value to the sensor
  if (this->co2_sensor_ != nullptr) {
    this->co2_sensor_->publish_state(co2_value);
    ESP_LOGD(TAG, "CO2 value: %d ppm", co2_value);
  }
  
  // Reset the buffer for the next packet
  this->reset_buffer_();
}

void CM1106Sniffer::reset_buffer_() {
  this->buffer_pos_ = 0;
  std::fill(this->buffer_, this->buffer_ + 9, 0);
}

void CM1106Sniffer::dump_config() {
  LOG_COMPONENT_CONFIG(TAG, "CM1106 Sniffer");
  LOG_UART_DEVICE(this);
}

// update() is not needed as the sensor is event-driven by the UART data
void CM1106Sniffer::update() {}

}  // namespace cm1106_sniffer
}  // namespace esphome
