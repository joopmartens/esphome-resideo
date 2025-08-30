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
    CONF_NAME,
)

# Define the namespace for the component
cm1106_sniffer_ns = cg.esphome_ns.namespace("cm1106_sniffer")

# Declare the C++ class name and its base classes
CM1106Sniffer = cm1106_sniffer_ns.class_("CM1106Sniffer", cg.PollingComponent, uart.UARTDevice)

# Dependencies for the component
DEPENDENCIES = ["uart"]
AUTO_LOAD = ["sensor"]

# Define the configuration schema
CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(CM1106Sniffer),
            cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),
            cv.Optional(CONF_CO2): sensor.sensor_schema(
                unit_of_measurement=UNIT_PARTS_PER_MILLION,
                icon=ICON_MOLECULE_CO2,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_CARBON_DIOXIDE,
                state_class=STATE_CLASS_MEASUREMENT,
            ).extend({
                cv.Optional(CONF_NAME): cv.string
            }),
        }
    ).extend(cv.polling_component_schema(CONF_UPDATE_INTERVAL))
)

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)

    parent = yield cg.get_variable(config[CONF_UART_ID])
    cg.add(var.set_uart_bus(parent))

    if CONF_CO2 in config:
        conf_co2 = config[CONF_CO2]
        sens = yield sensor.new_sensor(conf_co2)
        cg.add(var.set_co2_sensor(sens))
        
        # Explicitly set the sensor properties
        if CONF_NAME in conf_co2:
            cg.add(sens.set_name(conf_co2[CONF_NAME]))
        if CONF_UNIT_OF_MEASUREMENT in conf_co2:
            cg.add(sens.set_unit_of_measurement(conf_co2[CONF_UNIT_OF_MEASUREMENT]))
        if CONF_ICON in conf_co2:
            cg.add(sens.set_icon(conf_co2[CONF_ICON]))
        if CONF_ACCURACY_DECIMALS in conf_co2:
            cg.add(sens.set_accuracy_decimals(conf_co2[CONF_ACCURACY_DECIMALS]))
        if CONF_DEVICE_CLASS in conf_co2:
            cg.add(sens.set_device_class(conf_co2[CONF_DEVICE_CLASS]))
        if CONF_STATE_CLASS in conf_co2:
            cg.add(sens.set_state_class(conf_co2[CONF_STATE_CLASS]))
