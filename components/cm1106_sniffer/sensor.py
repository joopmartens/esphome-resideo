import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, sensor
from esphome.const import (
    CONF_ID,
    CONF_CO2,
    CONF_UART_ID,
    UNIT_PARTS_PER_MILLION,
    ICON_MOLECULE_CO2,
    DEVICE_CLASS_CARBON_DIOXIDE,
    STATE_CLASS_MEASUREMENT,
    CONF_ACCURACY_DECIMALS,
    CONF_ICON,
    CONF_UNIT_OF_MEASUREMENT,
)

# Define the namespace for the component
# This should match the namespace in your C++ code
cm1106_sniffer_ns = cg.esphome_ns.namespace("cm1106_sniffer")

# Declare the C++ class name and its base classes
CM1106Sniffer = cm1106_sniffer_ns.class_("CM1106Sniffer", cg.PollingComponent, uart.UARTDevice)

# Dependencies for the component
DEPENDENCIES = ["uart"]
AUTO_LOAD = ["sensor"]

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            # Component ID
            cv.GenerateID(): cv.declare_id(CM1106Sniffer),
            # The component requires a UART ID to connect to
            cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),
            # Define an optional CO2 sensor within the component
            cv.Optional(CONF_CO2): sensor.sensor_schema(
                unit_of_measurement=UNIT_PARTS_PER_MILLION,
                icon=ICON_MOLECULE_CO2,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_CARBON_DIOXIDE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    )
    .extend(cv.polling_component_schema("10s"))
)

def to_code(config):
    # Create an instance of the C++ class
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)

    # Get the UART component instance and set it in our C++ component
    parent = yield cg.get_variable(config[CONF_UART_ID])
    cg.add(var.set_uart_device(parent))

    # If the user defined a CO2 sensor in the YAML, create it and set it
    if CONF_CO2 in config:
        sens = yield sensor.new_sensor(config[CONF_CO2])
        cg.add(var.set_co2_sensor(sens))
