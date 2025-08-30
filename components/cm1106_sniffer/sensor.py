import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, uart
from esphome.const import (
    CONF_ID,
    CONF_UART_ID,
    CONF_UPDATE_INTERVAL,
    UNIT_PARTS_PER_MILLION,
    ICON_MOLECULE_CO2,
    DEVICE_CLASS_CARBON_DIOXIDE,
    STATE_CLASS_MEASUREMENT,
    CONF_ACCURACY_DECIMALS,
    CONF_NAME,
    CONF_UNIT_OF_MEASUREMENT,
    CONF_ICON,
    CONF_DEVICE_CLASS,
    CONF_STATE_CLASS,
)

# Define the namespace for the component
cm1106_sniffer_ns = cg.esphome_ns.namespace("cm1106_sniffer")

# Declare the C++ class name and its base classes
CM1106Sniffer = cm1106_sniffer_ns.class_("CM1106Sniffer", sensor.Sensor, cg.PollingComponent, uart.UARTDevice)

# Dependencies for the component
DEPENDENCIES = ["uart"]
AUTO_LOAD = ["sensor"]

# Define the configuration schema to extend the sensor base schema
CONFIG_SCHEMA = cv.All(
    sensor.SENSOR_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(CM1106Sniffer),
            cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),
        }
    ).extend(cv.polling_component_schema(CONF_UPDATE_INTERVAL))
)

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield sensor.register_sensor(var, config)
    yield uart.register_uart_device(var, config)
