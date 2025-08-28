#include "cm1106_sniffer_sensor.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace cm1106_sniffer {

static const char *const TAG = "cm1106_sniffer";

void CM1106SnifferSensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up CM1106 Sniffer Sensor...");
  // Clear any data in the UART buffer at startup.
  this->uart_device_->flush();
}

void CM1106SnifferSensor::loop() {
  // Continuously check for new bytes available on the UART bus.
  while (this->available()) {
    uint8_t byte;
    this->read_byte(&byte);
    this->handle_byte(byte);
  }
}

void CM1106SnifferSensor::handle_byte(uint8_t byte) {
  // The CM1106 sends a 9-byte packet. This function processes the incoming bytes
  // one by one to assemble a full packet and validate it.
  // Packet structure:
  // Byte 0: 0x16 (Start byte)
  // Byte 1: 0x01 (Command)
  // Byte 2: 0x02 (Payload length)
  // Byte 3: High byte of CO2 value
  // Byte 4: Low byte of CO2 value
  // Byte 5-7: Reserved/ignored
  // Byte 8: Checksum
  
  // If we are at the beginning of a new packet, check for the start byte.
  if (this->buffer_pos_ == 0 && byte != 0x16) {
    // If it's not the start byte, discard it and wait for the correct one.
    return;
  }
  
  // Store the byte in the buffer.
  this->buffer_[this->buffer_pos_++] = byte;

  // If the buffer is not yet full, wait for more data.
  if (this->buffer_pos_ < 9) {
    return;
  }

  // We have a full 9-byte packet. Proceed with validation.
  
  // Check for the correct start byte again, as a safeguard.
  if (this->buffer_[0] != 0x16) {
    ESP_LOGW(TAG, "Invalid start byte: 0x%02X. Resetting buffer.", this->buffer_[0]);
    this->reset_buffer_();
    return;
  }

  // Calculate the checksum for bytes 0 through 7.
  uint8_t checksum = 0;
  for (int i = 0; i < 8; ++i) {
    checksum += this->buffer_[i];
  }
  checksum = 0xFF - checksum + 1;

  // Compare the calculated checksum with the received checksum.
  if (this->buffer_[8] != checksum) {
    ESP_LOGW(TAG, "Checksum mismatch: calculated 0x%02X, received 0x%02X. Discarding packet.", checksum, this->buffer_[8]);
    this->reset_buffer_();
    return;
  }

  // If the checksum is valid, extract the CO2 value from bytes 3 and 4.
  uint16_t co2_value = (uint16_t) this->buffer_[3] << 8 | this->buffer_[4];
  
  // Publish the CO2 value to the ESPHome sensor platform.
  this->publish_state(co2_value);
  ESP_LOGD(TAG, "CO2 value: %d ppm", co2_value);
  
  // Reset the buffer to be ready for the next packet.
  this->reset_buffer_();
}

void CM1106SnifferSensor::reset_buffer_() {
  this->buffer_pos_ = 0;
  // It's good practice to clear the buffer's contents.
  std::fill(this->buffer_, this->buffer_ + 9, 0);
}

void CM1106SnifferSensor::dump_config() {
  LOG_SENSOR("CM1106Sniffer", "CM1106Sniffer", this);
  LOG_UART_DEVICE(this->uart_device_);
}

}  // namespace cm1106_sniffer
}  // namespace esphome
