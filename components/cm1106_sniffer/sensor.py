import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, uart
from esphome.const import (
    CONF_ID,
    CONF_UART_ID,
    CONF_CO2,
    CONF_UPDATE_INTERVAL,
    UNIT_PARTS_PER_MILLION,
    ICON_MOLECULE_CO2,
    DEVICE_CLASS_CARBON_DIOXIDE,
    STATE_CLASS_MEASUREMENT,
    CONF_ACCURACY_DECIMALS,
)

# Define the namespace for the component
cm1106_sniffer_ns = cg.esphome_ns.namespace("cm1106_sniffer")

# Declare the C++ class name and its base classes
CM1106SnifferComponent = cm1106_sniffer_ns.class_(
    "CM1106SnifferComponent", cg.PollingComponent, uart.UARTDevice
)

# The configuration schema for the nested CO2 sensor
CO2_SENSOR_SCHEMA = (
    sensor.sensor_schema(
        UNIT_PARTS_PER_MILLION, ICON_MOLECULE_CO2, 0, DEVICE_CLASS_CARBON_DIOXIDE, STATE_CLASS_MEASUREMENT
    ).extend({})
)

# Main configuration schema for the component
CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(CM1106SnifferComponent),
            cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),
            cv.Optional(CONF_CO2): CO2_SENSOR_SCHEMA,
        }
    )
    .extend(cv.polling_component_schema(CONF_UPDATE_INTERVAL))
)

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield uart.register_uart_device(var, config)

    if CONF_CO2 in config:
        conf = config[CONF_CO2]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_co2_sensor(sens))
