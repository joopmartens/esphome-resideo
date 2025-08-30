#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace cm1106_sniffer {

class CM1106Sniffer : public PollingComponent, public uart::UARTDevice {
 public:
  // Lifecycle methods for ESPHome.
  void setup() override;
  void loop() override;
  void dump_config() override;
  void update() override;

  // Setters to configure the sensor properties.
  void set_unit_of_measurement(const std::string &unit) { this->unit_of_measurement_ = unit; }
  void set_icon(const std::string &icon) { this->icon_ = icon; }
  void set_device_class(const std::string &device_class) { this->device_class_ = device_class; }
  void set_state_class(const std::string &state_class) { this->state_class_ = state_class; }
  void set_accuracy_decimals(int accuracy) { this->accuracy_decimals_ = accuracy; }
  void set_name(const std::string &name) { this->name_ = name; }

 protected:
  void handle_byte(uint8_t byte);
  void reset_buffer_();

  uint8_t buffer_[9] = {0};
  uint8_t buffer_pos_ = 0;
  
  // Store the properties from the Python configuration.
  std::string unit_of_measurement_;
  std::string icon_;
  std::string device_class_;
  std::string state_class_;
  int accuracy_decimals_ = 0;
  std::string name_;
};

}  // namespace cm1106_sniffer
}  // namespace esphome
