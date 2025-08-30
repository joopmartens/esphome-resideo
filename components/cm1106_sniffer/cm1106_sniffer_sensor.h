#pragma once

#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace cm1106_sniffer {

class CM1106Sniffer : public sensor::Sensor, public PollingComponent, public uart::UARTDevice {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  void update() override;

 protected:
  void handle_byte(uint8_t byte);
  void reset_buffer_();
  
  uint8_t buffer_[9];
  uint8_t buffer_pos_{0};
};

}  // namespace cm1106_sniffer
}  // namespace esphome
