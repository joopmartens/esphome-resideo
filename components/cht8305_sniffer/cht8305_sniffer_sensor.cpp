/**
 * Specifications CHT8305 T/H Sensor
 * Oct 2017 rev 1.1 SENSYLINK MIcroelectronics Co. LTD
 * https://www.semiee.com/file/Sensylink/Sensylink-CHT8305.pdf
 */

#include "cht8305_sniffer_sensor.h"
#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>
#include <esp_log.h>

namespace esphome {
namespace cht8305_sniffer {

static const char *TAG = "cht8305_sniffer";

///////////////////////////////////////////////////////////////////////////////////////////////////
// C++ variables and ISRs require a context pointer to access class members.
// We'll use a static instance pointer to make the ISRs simple C-style functions
// and allow them to call member functions of the class.
static CHT8305SnifferSensor *global_instance = nullptr;

static volatile uint8_t device_address;
static volatile uint8_t device_register[256];
static volatile uint8_t device_register_ptr;

static volatile bool i2cIdle = true; // true if the I2C BUS is idle
static volatile int bitIdx = 0;      // the bitindex within the frame
static volatile int byteIdx = 0;     // nr of bytes within this frame
static volatile uint8_t data = 0;    // the byte under construction
static volatile bool writing = true; // is the master reading or writing to the device
static gpio_num_t PIN_SCL;
static gpio_num_t PIN_SDA;

// A NACK will end our sniffing session
#define I2C_HANDLE_NACK if (gpio_get_level(PIN_SDA) > 0) { i2cIdle = true; return; }

// Rising SCL makes us reading the SDA pin. This is our ISR handler.
void IRAM_ATTR i2cTriggerOnRaisingSCL(void *arg) {
    if (i2cIdle) {
        return;
    }

    // Get the value from SDA using the ESP-IDF function
    int sda_val = gpio_get_level(PIN_SDA);

    // First byte is 7-bit address, 8th bit is R/W
    if (byteIdx == 0 && bitIdx == 7) {
        writing = (sda_val == 0); // if SDA is LOW, the master wants to write
    } else if (bitIdx != 8) { // data bit
        data = (data << 1) | (sda_val > 0 ? 1 : 0);
    } else { // we are at the ninth (N)ACK bit
        if (byteIdx == 0) {  //slave address byte
            I2C_HANDLE_NACK; // On NACK we end the conversation as the slave declined and a restart (Sr) is expected
            device_address = data;
        } else if (byteIdx == 1 && writing) { // if the master is writing, the second byte is the register address
            I2C_HANDLE_NACK; // On NACK we end the conversation as the slave declined the address
            device_register_ptr = data;
        } else {
            device_register[device_register_ptr++] = data;
            I2C_HANDLE_NACK; // On NACK we end the conversation AFTER storing the data. 
        }
        byteIdx++;
        data = 0;
        bitIdx = -1;
    }
    bitIdx++;
}

/**
 * This is for recognizing I2C START and STOP.
 * We must check the state of both SCL and SDA at the moment of the interrupt.
 * If SCL is HIGH, a falling edge on SDA is a START, and a rising edge is a STOP.
 */
void IRAM_ATTR i2cTriggerOnChangeSDA(void *arg) {
    if (gpio_get_level(PIN_SCL) == 0) {
        return;
    }

    if (gpio_get_level(PIN_SDA) > 0) { // RISING if SDA is HIGH (1) -> STOP
        i2cIdle = true;
    } else { // FALLING if SDA is LOW -> START?
        if (i2cIdle) { // If we are idle, this is a START
            bitIdx = 0;
            byteIdx = 0;
            data = 0;
            i2cIdle = false;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// We set the SCL and SDA pins to the values given in the configuration.
// This is now done using ESP-IDF GPIO functions.
///////////////////////////////////////////////////////////////////////////////////////////////////
void CHT8305SnifferSensor::setup() {
    global_instance = this;

    PIN_SCL = (gpio_num_t)scl_pin_;
    PIN_SDA = (gpio_num_t)sda_pin_;

    // Reset variables
    memset((void *)device_register, 0, sizeof(device_register));
    i2cIdle = true;
    device_register_ptr = 0;

    // Configure GPIOs using ESP-IDF functions
    gpio_config_t scl_config = {
        .pin_bit_mask = (1ULL << PIN_SCL),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_POSEDGE // Interrupt on rising edge of SCL
    };
    gpio_config(&scl_config);

    gpio_config_t sda_config = {
        .pin_bit_mask = (1ULL << PIN_SDA),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE // Interrupt on any edge of SDA
    };
    gpio_config(&sda_config);

    // Install the global GPIO ISR service
    ESP_ERROR_CHECK(gpio_install_isr_service(0));

    // Add the specific handlers for each pin
    ESP_ERROR_CHECK(gpio_isr_handler_add(PIN_SCL, i2cTriggerOnRaisingSCL, nullptr));
    ESP_ERROR_CHECK(gpio_isr_handler_add(PIN_SDA, i2cTriggerOnChangeSDA, nullptr));

    ESP_LOGI(TAG, "CHT8305 Sniffer initialized on SCL:%d, SDA:%d", (int)PIN_SCL, (int)PIN_SDA);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Loop and Update methods remain unchanged as they are not dependent on Arduino-specific functions.
///////////////////////////////////////////////////////////////////////////////////////////////////
void CHT8305SnifferSensor::loop() {
    unsigned long now = millis();
    if (i2cIdle && (now - last_sample_time > 100)) {
        last_sample_time = now;
        uint16_t temp = (uint16_t)device_register[0] << 8 | device_register[1];
        uint16_t humidity = (uint16_t)device_register[2] << 8 | device_register[3];

        temperature_raw_.push_back(temp);
        humidity_raw_.push_back(humidity);
    }
}

void CHT8305SnifferSensor::update() {
    if (temperature_raw_.empty() || humidity_raw_.empty()) {
        ESP_LOGW(TAG, "No data available to update sensors.");
        return;
    }
    ESP_LOGD(TAG, "Taking the mean from a window size %d", temperature_raw_.size());
    std::sort(temperature_raw_.begin(), temperature_raw_.end());
    std::sort(humidity_raw_.begin(), humidity_raw_.end());
    
    uint16_t temp_median = temperature_raw_[temperature_raw_.size() / 2];
    uint16_t hum_median = humidity_raw_[humidity_raw_.size() / 2];

    float temp = (static_cast<float>(temp_median) * 165.0f / 65535.0f) - 40.0f;
    float hum = (static_cast<float>(hum_median) * 100.0f / 65535.0f);

    temperature_raw_.clear();
    humidity_raw_.clear();

    if (temperature_sensor_ != nullptr)
        temperature_sensor_->publish_state(temp);
    if (humidity_sensor_ != nullptr)
        humidity_sensor_->publish_state(hum);
}

} // namespace cht8305_sniffer
} // namespace esphome
