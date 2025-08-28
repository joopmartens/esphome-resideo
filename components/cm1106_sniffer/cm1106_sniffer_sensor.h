#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace cm1106_sniffer {

class CM1106Sniffer : public esphome::PollingComponent, public uart::UARTDevice {
 public:
  // Public methods required by ESPHome
  void setup() override;
  void loop() override;
  void dump_config() override;
  void update() override;

  // Setters to link the component to the ESPHome sensor and UART bus.
  void set_co2_sensor(sensor::Sensor *sensor) { this->co2_sensor_ = sensor; }
  void set_uart_bus(uart::UARTComponent *bus) { this->uart_bus_ = bus; }

 protected:
  // Private helper methods for handling the data stream
  void handle_byte(uint8_t byte);
  void reset_buffer_();

  // Member variables for the state machine
  uint8_t buffer_[9] = {0};
  uint8_t buffer_pos_{0};
  sensor::Sensor *co2_sensor_{nullptr};
  uart::UARTComponent *uart_bus_{nullptr};
};

}  // namespace cm1106_sniffer
}  // namespace esphome
