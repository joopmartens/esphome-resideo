import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, uart
from esphome.const import (
    CONF_ID,
    CONF_UPDATE_INTERVAL,
    CONF_CO2,
    UNIT_PARTS_PER_MILLION,
    ICON_MOLECULE_CO2,
    STATE_CLASS_MEASUREMENT,
    DEVICE_CLASS_CARBON_DIOXIDE,
)

# Define the namespace for the C++ component
cm1106_sniffer_ns = cg.esphome_ns.namespace("cm1106_sniffer")

# Explicitly declare the C++ class from the header file
CM1106Sniffer = cm1106_sniffer_ns.class_("CM1106Sniffer", sensor.Sensor, cg.PollingComponent)

# Define the configuration schema for the component
CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(CM1106Sniffer),
            cv.Optional(CONF_CO2): sensor.sensor_schema(
                unit_of_measurement=UNIT_PARTS_PER_MILLION,
                icon=ICON_MOLECULE_CO2,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_CARBON_DIOXIDE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        },
    )
    .extend(
        {     
            # The component requires the UART bus ID to connect to
            cv.Required(uart.CONF_UART_ID): cv.use_id(uart.UARTComponent),
        }
    )
    .extend(cv.polling_component_schema("10s"))
)


def to_yaml(config):
    # This function is not strictly needed for this simple sensor, but is
    # good practice for more complex components to handle YAML output.
    pass


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)

    parent = yield cg.get_variable(config[CONF_UART_ID])
    cg.add(var.set_uart_bus(parent))

    if CONF_CO2 in config:
        sens = yield sensor.new_sensor(config[CONF_CO2])
        cg.add(var.set_co2_sensor(sens))