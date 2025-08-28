#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/sensor/sensor.h"
#include <vector>

namespace esphome {
namespace cht8305_sniffer {

class CHT8305SnifferSensor : public PollingComponent {
 public:
  // Default constructor required by the ESPHome framework.
  CHT8305SnifferSensor()
      : PollingComponent(100) {}

  // The constructor initializes the polling interval and pin numbers.
  CHT8305SnifferSensor(int scl_pin, int sda_pin)
      : PollingComponent(100), scl_pin_(scl_pin), sda_pin_(sda_pin) {}

  // Lifecycle methods for ESPHome.
  void setup() override;
  void loop() override;
  void update() override;

  // Setters to link the component to the ESPHome sensors.
  void set_temperature_sensor(sensor::Sensor *temperature_sensor) {
    this->temperature_sensor_ = temperature_sensor;
  }
  void set_humidity_sensor(sensor::Sensor *humidity_sensor) {
    this->humidity_sensor_ = humidity_sensor;
  }

  // Pin numbers from the YAML configuration.
  int scl_pin_;
  int sda_pin_;

  // ESP-IDF GPIO handles. These are set in setup().
  gpio_num_t pin_scl_;
  gpio_num_t pin_sda_;

  // Variables for I2C sniffing, declared volatile as they are modified in ISRs.
  volatile bool i2c_idle_ = true;
  volatile uint8_t byte_idx_ = 0;
  volatile int8_t bit_idx_ = 0;
  volatile bool writing_ = false;
  volatile uint8_t data_ = 0;
  volatile uint8_t device_address_ = 0;
  volatile uint8_t device_register_ptr_ = 0;
  volatile uint8_t device_register_[4] = {0, 0, 0, 0};

  // Variables for data processing and timing.
  unsigned long last_sample_time_ = 0;
  std::vector<uint16_t> temperature_raw_;
  std::vector<uint16_t> humidity_raw_;
  
  sensor::Sensor *temperature_sensor_ = nullptr;
  sensor::Sensor *humidity_sensor_ = nullptr;
};

} // namespace cht8305_sniffer
} // namespace esphome
