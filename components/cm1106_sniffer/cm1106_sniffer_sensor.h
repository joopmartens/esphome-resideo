#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace cm1106_sniffer {

class CM1106SnifferSensor : public PollingComponent, public uart::UARTDevice {
 public:
  void set_co2_sensor(sensor::Sensor *co2_sensor) { this->co2_sensor_ = co2_sensor; }

  void setup() override;
  void loop() override;
  void update() override;
  void dump_config() override;

 protected:
  void handle_byte(uint8_t byte);
  void reset_buffer_();

  uint8_t buffer_[9];
  uint8_t buffer_pos_{0};
  sensor::Sensor *co2_sensor_{nullptr};
};

}  // namespace cm1106_sniffer
}  // namespace esphome
