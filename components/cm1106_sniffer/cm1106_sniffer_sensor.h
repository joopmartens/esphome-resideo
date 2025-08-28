#pragma once

#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"

namespace esphome {
namespace cm1106_sniffer {

class CM1106SnifferSensor : public sensor::Sensor, public Component, public uart::UARTDevice {
 public:
  // Public methods required by ESPHome
  void setup() override;
  void loop() override;
  void dump_config() override;

 protected:
  // Private helper methods for handling the data stream
  void handle_byte(uint8_t byte);
  void reset_buffer_();

  // Member variables for the state machine
  uint8_t buffer_[9] = {0};
  uint8_t buffer_pos_{0};
};

}  // namespace cm1106_sniffer
}  // namespace esphome
