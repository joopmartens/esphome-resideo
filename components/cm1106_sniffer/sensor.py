import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, uart
from esphome.const import (
    CONF_ID,
    CONF_ICON,
    CONF_UNIT_OF_MEASUREMENT,
    CONF_ACCURACY_DECIMALS,
    DEVICE_CLASS_CARBON_DIOXIDE,
    STATE_CLASS_MEASUREMENT,
    UNIT_PARTS_PER_MILLION,
    ICON_MOLECULE_CO2,
)

# Define the namespace for the C++ component
cm1106_sniffer_ns = cg.esphome_ns.namespace("cm1106_sniffer")

# Explicitly declare the C++ class from the header file
CM1106Sniffer = cm1106_sniffer_ns.class_("CM1106Sniffer", sensor.Sensor, cg.PollingComponent)

# Define the configuration schema for the component
CONFIG_SCHEMA = (
    sensor.sensor_schema()
    .extend(
        {
            cv.GenerateID(): cv.declare_id(CM1106Sniffer),
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


async def to_code(config):
    # Get the UART bus ID from the configuration
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    # --- NEW: Explicitly set the sensor's fixed properties ---
    # This is a more reliable method than setting defaults in the schema.
    cg.add(var.set_unit_of_measurement(UNIT_PARTS_PER_MILLION))
    cg.add(var.set_icon(ICON_MOLECULE_CO2))
    cg.add(var.set_accuracy_decimals(0))
    cg.add(var.set_device_class(DEVICE_CLASS_CARBON_DIOXIDE))
    cg.add(var.set_state_class(STATE_CLASS_MEASUREMENT))
    # --- End of new code ---

    # Get a reference to the UART component variable
    uart_component = await cg.get_variable(config[uart.CONF_UART_ID])
    # Call the setter method on the C++ component
    cg.add(var.set_uart_parent(uart_component))

    # Use a sensor schema
    await sensor.register_sensor(var, config)