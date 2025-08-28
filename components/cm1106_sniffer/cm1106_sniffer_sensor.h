#pragma once

#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"

namespace esphome {
namespace cm1106_sniffer {

class CM1106Sniffer : public sensor::Sensor, public Component {
 public:
  // Public methods required by ESPHome
  void setup() override;
  void loop() override;
  void dump_config() override;

  // Setters for the UART component
  void set_uart_parent(uart::UARTComponent *parent) { this->uart_component_ = parent; }

 protected:
  // Private helper methods for handling the data stream
  void handle_byte(uint8_t byte);
  void reset_buffer_();

  // Member variables for the state machine
  uint8_t buffer_[9] = {0};
  uint8_t buffer_pos_{0};

  // Explicit member for the UART component
  uart::UARTComponent *uart_component_{nullptr};
};

}  // namespace cm1106_sniffer
}  // namespace esphome
