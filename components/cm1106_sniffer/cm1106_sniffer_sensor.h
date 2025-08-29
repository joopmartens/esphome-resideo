#pragma once

#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"

namespace esphome {
namespace cm1106_sniffer {

class CM1106Sniffer : public PollingComponent, public uart::UARTDevice {
 public:
  void set_co2_sensor(sensor::Sensor *co2_sensor) { this->co2_sensor_ = co2_sensor; }
  void set_uart_bus(uart::UARTComponent *uart_bus) { this->set_uart_parent(uart_bus); }

  void setup() override;
  void loop() override;
  void dump_config() override;

 protected:
  void handle_byte(uint8_t byte);
  void reset_buffer_();

  sensor::Sensor *co2_sensor_{nullptr};
  uint8_t buffer_[9];
  uint8_t buffer_pos_{0};
};

} // namespace cm1106_sniffer
} // namespace esphome
