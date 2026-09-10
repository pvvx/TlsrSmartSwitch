"""Alternative firmwares for TlsrSmartSwitch on the TLSR825x/TLSR8656 chipset.
see https://github.com/pvvx/TlsrSmartSwitch
Code processing: DeepSeek
"""
from zigpy.quirks.v2 import QuirkBuilder, ReportingConfig, EntityType
from zigpy.quirks.v2.homeassistant import (
    UnitOfTemperature,
    UnitOfTime,
    UnitOfElectricPotential,
    UnitOfElectricCurrent,
    UnitOfPower,
)
from zhaquirks import CustomCluster
from zigpy.zcl.clusters.homeautomation import ElectricalMeasurement
from zigpy.zcl.clusters.general import OnOffConfiguration, MultistateInput, OnOff
from zigpy.zcl.foundation import ZCLAttributeDef
import zigpy.types as t

ENDPOINT = 1


# --------------------------------------------------------------------------
# Типы и enum'ы
# --------------------------------------------------------------------------

class TssGPIO_rx(t.enum16):
    PA0 = 0x0001
    PB0 = 0x0101
    PB7 = 0x0180
    PC3 = 0x0208
    PC5 = 0x0220
    PD6 = 0x0340


class TssGPIO_tx(t.enum16):
    PA2 = 0x0002
    PB1 = 0x0102
    PC2 = 0x0204
    PD0 = 0x0301
    PD3 = 0x0308
    PD7 = 0x0380


class TssGPIO_all(t.enum16):
    Off = 0
    PA0 = 0x0001
    PA1 = 0x0002
    PA2 = 0x0004
    PA3 = 0x0008
    PA4 = 0x0010
    PA5 = 0x0020
    PA6 = 0x0040
    PA7 = 0x0080
    PB0 = 0x0101
    PB1 = 0x0102
    PB2 = 0x0104
    PB3 = 0x0108
    PB4 = 0x0110
    PB5 = 0x0120
    PB6 = 0x0140
    PB7 = 0x0180
    PC0 = 0x0201
    PC1 = 0x0202
    PC2 = 0x0204
    PC3 = 0x0208
    PC4 = 0x0210
    PC5 = 0x0220
    PC6 = 0x0240
    PC7 = 0x0280
    PD0 = 0x0301
    PD1 = 0x0302
    PD2 = 0x0304
    PD3 = 0x0308
    PD4 = 0x0310
    PD5 = 0x0320
    PD6 = 0x0340
    PD7 = 0x0380
    PE0 = 0x0401
    PE1 = 0x0402
    PE2 = 0x0404
    PE3 = 0x0408


class TssGPIO_LED_config(t.enum16):
    LED1_Direct_LED2_Direct = 0
    LED1_Inverse_LED2_Direct = 1
    LED1_Direct_LED2_Inverse = 2
    LED1_Inverse_LED2_Inverse = 3


class TssSwitchActions(t.enum8):
    Toggle = 0
    Momentary = 1
    Multifunction = 2
    # Thermostat = 4


class TssSwitchDecoupled(t.enum8):
    ControlRelay = 0
    Decoupled = 1


class TssLedControl(t.enum8):
    Led_Off = 0
    Led_On = 1
    Led_OnOff = 2


class TssPowerPrecision(t.enum8):
    auto = 0
    max_32767W = 1
    max_3276W7 = 2
    max_327W67 = 3
    max_32W767 = 4


class TssAlarmMask(t.bitmap8):
    Over_Voltage = 0b00000001
    Under_Voltage = 0b00000010
    Over_Current = 0b00000100
    Over_Temp = 0b00001000
    Under_Temp = 0b00010000
    Error_TS = 0b00100000


def Alarm_converter(value: int) -> str:
    if value == 0:
        return "None"
    if value > 63:
        return "Unknown"
    names = ["OV", "UV", "UI", "OT", "UT", "TS"]
    parts = [names[i] for i in range(6) if value & (1 << i)]
    return ", ".join(parts)


class TssCalibration(t.enum8):
    Calibrate_ok = 0
    Calibrate_V = 1
    Calibrate_I = 2
    Calibrate_VI = 3
    Calibrate_P = 4
    Calibrate_VP = 5
    Calibrate_IP = 6
    Calibrate_VIP = 7
    Recalculate_Power = 8


def StatusCalibration_converter(value: int) -> str:
    actions = {
        0: "Ok",
        1: "Calibrate V",
        2: "Calibrate I",
        3: "Calibrate V and I",
        4: "Calibrate P",
        5: "Calibrate V and P",
        6: "Calibrate I and P",
        7: "Calibrate V, I, P",
        8: "Recalculate Power",
        65: "Error value V",
        66: "Error value I",
        67: "Error value V and I",
        68: "Error value P",
        69: "Error value V and P",
        70: "Error value I and P",
        71: "Error value V, I, P",
        128: "Calibration error",
        129: "V calibration error",
        130: "I calibration error",
        131: "V and I calibration error",
        132: "P calibration error",
        133: "V and P calibration error",
        134: "I and P calibration error",
        135: "V, I, P calibration error",
    }
    return actions.get(value)


# --------------------------------------------------------------------------
# Кластеры
# --------------------------------------------------------------------------

class TssOnOff(CustomCluster, OnOff):

    class AttributeDefs(OnOff.AttributeDefs):
        relay_state = ZCLAttributeDef(
            id=0xf000, type=t.Bool, access="r",
            is_manufacturer_specific=True,
        )
        key_lock = ZCLAttributeDef(
            id=0xf001, type=t.Bool, access="rw",
            is_manufacturer_specific=True,
        )
        led_control = ZCLAttributeDef(
            id=0xf002, type=TssLedControl, access="rw",
            is_manufacturer_specific=True,
        )
        gpio_relay = ZCLAttributeDef(
            id=0xf100, type=TssGPIO_all, access="rw",
            is_manufacturer_specific=True,
        )
        gpio_led1 = ZCLAttributeDef(
            id=0xf101, type=TssGPIO_all, access="rw",
            is_manufacturer_specific=True,
        )
        gpio_led2 = ZCLAttributeDef(
            id=0xf102, type=TssGPIO_all, access="rw",
            is_manufacturer_specific=True,
        )
        gpio_key = ZCLAttributeDef(
            id=0xf103, type=TssGPIO_all, access="rw",
            is_manufacturer_specific=True,
        )
        gpio_switch = ZCLAttributeDef(
            id=0xf104, type=TssGPIO_all, access="rw",
            is_manufacturer_specific=True,
        )
        gpio_swire = ZCLAttributeDef(
            id=0xf105, type=TssGPIO_all, access="rw",
            is_manufacturer_specific=True,
        )
        gpio_sel = ZCLAttributeDef(
            id=0xf106, type=TssGPIO_all, access="rw",
            is_manufacturer_specific=True,
        )
        gpio_cf = ZCLAttributeDef(
            id=0xf107, type=TssGPIO_all, access="rw",
            is_manufacturer_specific=True,
        )
        gpio_cf1 = ZCLAttributeDef(
            id=0xf108, type=TssGPIO_all, access="rw",
            is_manufacturer_specific=True,
        )
        gpio_rx = ZCLAttributeDef(
            id=0xf109, type=TssGPIO_rx, access="rw",
            is_manufacturer_specific=True,
        )
        gpio_tx = ZCLAttributeDef(
            id=0xf10a, type=TssGPIO_tx, access="rw",
            is_manufacturer_specific=True,
        )
        gpio_flags = ZCLAttributeDef(
            id=0xf10b, type=TssGPIO_LED_config, access="rw",
            is_manufacturer_specific=True,
        )


class TssOnOffConfiguration(CustomCluster, OnOffConfiguration):

    class AttributeDefs(OnOffConfiguration.AttributeDefs):
        switch_maction = ZCLAttributeDef(
            id=0xF003, type=TssSwitchActions, access="rw",
            is_manufacturer_specific=True,
        )
        switch_decoupled = ZCLAttributeDef(
            id=0xF004, type=TssSwitchDecoupled, access="rw",
            is_manufacturer_specific=True,
        )


class TssElectricalMeasurement(CustomCluster, ElectricalMeasurement):

    class AttributeDefs(ElectricalMeasurement.AttributeDefs):
        alarm_mask = ZCLAttributeDef(
            id=0xF005, type=TssAlarmMask, access="rw",
            is_manufacturer_specific=True,
        )
        alarm_events = ZCLAttributeDef(
            id=0xF006, type=TssAlarmMask, access="rwp",
            is_manufacturer_specific=True,
        )
        calculating_current = ZCLAttributeDef(
            id=0xF007, type=t.uint32_t, access="rw",
            is_manufacturer_specific=True,
        )
        calculating_voltage = ZCLAttributeDef(
            id=0xF008, type=t.uint32_t, access="rw",
            is_manufacturer_specific=True,
        )
        calculating_power = ZCLAttributeDef(
            id=0xF009, type=t.uint32_t, access="rw",
            is_manufacturer_specific=True,
        )
        calculating_freq = ZCLAttributeDef(
            id=0xF00B, type=t.uint32_t, access="rw",
            is_manufacturer_specific=True,
        )
        calibration_current = ZCLAttributeDef(
            id=0xF080, type=t.uint16_t, access="rw",
            is_manufacturer_specific=True,
        )
        calibration_voltage = ZCLAttributeDef(
            id=0xF081, type=t.uint16_t, access="rw",
            is_manufacturer_specific=True,
        )
        calibration_power = ZCLAttributeDef(
            id=0xF082, type=t.uint16_t, access="rw",
            is_manufacturer_specific=True,
        )
        start_calibration = ZCLAttributeDef(
            id=0xF090, type=TssCalibration, access="rwp",
            is_manufacturer_specific=True,
        )
        power_precision = ZCLAttributeDef(
            id=0xF013, type=TssPowerPrecision, access="rw",
            is_manufacturer_specific=True,
        )


# --------------------------------------------------------------------------
# Helper'ы для повторяющихся сущностей
# --------------------------------------------------------------------------

def add_switch_config(b):
    return (
        b.enum(
            TssOnOffConfiguration.AttributeDefs.switch_maction.name,
            TssSwitchActions,
            TssOnOffConfiguration.cluster_id,
            translation_key="switch_maction",
            fallback_name="Ext.Switch actions",
            endpoint_id=ENDPOINT,
        )
        .enum(
            TssOnOffConfiguration.AttributeDefs.switch_decoupled.name,
            TssSwitchDecoupled,
            TssOnOffConfiguration.cluster_id,
            translation_key="switch_decoupled",
            fallback_name="Ext.Switch mode",
            endpoint_id=ENDPOINT,
        )
    )


def add_common_onoff(b):
    return (
        b.switch(
            TssOnOff.AttributeDefs.key_lock.name,
            TssOnOff.cluster_id,
            off_value=0,
            on_value=1,
            translation_key="key_lock",
            fallback_name="Button lock",
            endpoint_id=ENDPOINT,
        )
        .enum(
            TssOnOff.AttributeDefs.led_control.name,
            TssLedControl,
            TssOnOff.cluster_id,
            translation_key="led_control",
            fallback_name="LED Control",
            endpoint_id=ENDPOINT,
        )
        .enum(
            TssOnOff.AttributeDefs.gpio_relay.name,
            TssGPIO_all,
            TssOnOff.cluster_id,
            initially_disabled=True,
            translation_key="gpio_relay",
            fallback_name="GPIO Relay",
            endpoint_id=ENDPOINT,
        )
        .enum(
            TssOnOff.AttributeDefs.gpio_led1.name,
            TssGPIO_all,
            TssOnOff.cluster_id,
            initially_disabled=True,
            translation_key="gpio_led1",
            fallback_name="GPIO LED1",
            endpoint_id=ENDPOINT,
        )
        .enum(
            TssOnOff.AttributeDefs.gpio_led2.name,
            TssGPIO_all,
            TssOnOff.cluster_id,
            initially_disabled=True,
            translation_key="gpio_led2",
            fallback_name="GPIO LED2",
            endpoint_id=ENDPOINT,
        )
        .enum(
            TssOnOff.AttributeDefs.gpio_key.name,
            TssGPIO_all,
            TssOnOff.cluster_id,
            initially_disabled=True,
            translation_key="gpio_key",
            fallback_name="GPIO Key",
            endpoint_id=ENDPOINT,
        )
    )


def add_gpio_switch(b):
    return b.enum(
        TssOnOff.AttributeDefs.gpio_switch.name,
        TssGPIO_all,
        TssOnOff.cluster_id,
        initially_disabled=True,
        translation_key="gpio_switch",
        fallback_name="GPIO Switch",
        endpoint_id=ENDPOINT,
    )


def add_gpio_flags(b):
    return b.enum(
        TssOnOff.AttributeDefs.gpio_flags.name,
        TssGPIO_LED_config,
        TssOnOff.cluster_id,
        initially_disabled=True,
        translation_key="gpio_flags",
        fallback_name="GPIO LED1 and LED2 config",
        endpoint_id=ENDPOINT,
    )


def add_bl0937_gpio(b):
    return (
        b.enum(
            TssOnOff.AttributeDefs.gpio_sel.name,
            TssGPIO_all,
            TssOnOff.cluster_id,
            initially_disabled=True,
            translation_key="gpio_sel",
            fallback_name="GPIO SEL BL0937 (8)",
            endpoint_id=ENDPOINT,
        )
        .enum(
            TssOnOff.AttributeDefs.gpio_cf.name,
            TssGPIO_all,
            TssOnOff.cluster_id,
            initially_disabled=True,
            translation_key="gpio_cf",
            fallback_name="GPIO CF BL0937 (6)",
            endpoint_id=ENDPOINT,
        )
        .enum(
            TssOnOff.AttributeDefs.gpio_cf1.name,
            TssGPIO_all,
            TssOnOff.cluster_id,
            initially_disabled=True,
            translation_key="gpio_cf1",
            fallback_name="GPIO CF1 BL0937 (7)",
            endpoint_id=ENDPOINT,
        )
    )


def add_bl0942_gpio(b, tx_disabled):
    return (
        b.enum(
            TssOnOff.AttributeDefs.gpio_tx.name,
            TssGPIO_tx,
            TssOnOff.cluster_id,
            initially_disabled=tx_disabled,
            translation_key="gpio_tx",
            fallback_name="GPIO TX BL0942 (10)",
            endpoint_id=ENDPOINT,
        )
        .enum(
            TssOnOff.AttributeDefs.gpio_rx.name,
            TssGPIO_rx,
            TssOnOff.cluster_id,
            initially_disabled=True,
            translation_key="gpio_rx",
            fallback_name="GPIO RX BL0942 (9)",
            endpoint_id=ENDPOINT,
        )
    )


def add_electrical(b, include_calculating_freq):
    b = (
        b.number(
            ElectricalMeasurement.AttributeDefs.rms_extreme_over_voltage_period.name,
            ElectricalMeasurement.cluster_id,
            min_value=0, max_value=65535, step=1,
            unit=UnitOfTime.SECONDS,
            translation_key="restart_period_after_protection",
            fallback_name="Restart period after protection",
            mode="box",
            endpoint_id=ENDPOINT,
        )
        .number(
            ElectricalMeasurement.AttributeDefs.rms_extreme_under_voltage_period.name,
            ElectricalMeasurement.cluster_id,
            min_value=0, max_value=65535, step=1,
            unit=UnitOfTime.SECONDS,
            translation_key="poweron_test_period",
            fallback_name="Power-on test period",
            mode="box",
            endpoint_id=ENDPOINT,
        )
        .number(
            ElectricalMeasurement.AttributeDefs.rms_voltage_swell_period.name,
            ElectricalMeasurement.cluster_id,
            min_value=0, max_value=65535, step=1,
            unit=UnitOfTime.SECONDS,
            translation_key="overcurrent_period",
            fallback_name="Overcurrent period before Off",
            mode="box",
            endpoint_id=ENDPOINT,
        )
        .number(
            ElectricalMeasurement.AttributeDefs.rms_extreme_over_voltage.name,
            ElectricalMeasurement.cluster_id,
            min_value=0, max_value=380, step=1, multiplier=0.01,
            unit=UnitOfElectricPotential.VOLT,
            translation_key="rms_extreme_over_voltage",
            fallback_name="Over Voltage",
            mode="box",
            endpoint_id=ENDPOINT,
        )
        .number(
            ElectricalMeasurement.AttributeDefs.rms_extreme_under_voltage.name,
            ElectricalMeasurement.cluster_id,
            min_value=0, max_value=380, step=1, multiplier=0.01,
            unit=UnitOfElectricPotential.VOLT,
            translation_key="rms_extreme_under_voltage",
            fallback_name="Under Voltage",
            mode="box",
            endpoint_id=ENDPOINT,
        )
        .number(
            ElectricalMeasurement.AttributeDefs.rms_voltage_swell.name,
            ElectricalMeasurement.cluster_id,
            min_value=0, max_value=32000, step=1,
            unit=UnitOfElectricCurrent.MILLIAMPERE,
            translation_key="rms_extreme_over_current",
            fallback_name="Over Current",
            mode="box",
            endpoint_id=ENDPOINT,
        )
        .number(
            TssElectricalMeasurement.AttributeDefs.calculating_current.name,
            TssElectricalMeasurement.cluster_id,
            min_value=0, max_value=4294967295, step=1,
            translation_key="calculating_current",
            fallback_name="Calc.Coef. Current",
            mode="box",
            endpoint_id=ENDPOINT,
        )
        .number(
            TssElectricalMeasurement.AttributeDefs.calculating_voltage.name,
            TssElectricalMeasurement.cluster_id,
            min_value=0, max_value=4294967295, step=1,
            translation_key="calculating_voltage",
            fallback_name="Calc.Coef. Voltage",
            mode="box",
            endpoint_id=ENDPOINT,
        )
        .number(
            TssElectricalMeasurement.AttributeDefs.calculating_power.name,
            TssElectricalMeasurement.cluster_id,
            min_value=0, max_value=4294967295, step=1,
            translation_key="calculating_power",
            fallback_name="Calc.Coef. Power",
            mode="box",
            endpoint_id=ENDPOINT,
        )
    )

    if include_calculating_freq:
        b = b.number(
            TssElectricalMeasurement.AttributeDefs.calculating_freq.name,
            TssElectricalMeasurement.cluster_id,
            min_value=0, max_value=4294967295, step=1,
            translation_key="calculating_freq",
            fallback_name="Calc.Coef. Freq",
            mode="box",
            endpoint_id=ENDPOINT,
        )

    return (
        b.number(
            TssElectricalMeasurement.AttributeDefs.calibration_current.name,
            TssElectricalMeasurement.cluster_id,
            unit=UnitOfElectricCurrent.MILLIAMPERE,
            min_value=100, max_value=32000, step=1,
            translation_key="calibration_current",
            fallback_name="Value for Current calibration",
            mode="box",
            endpoint_id=ENDPOINT,
        )
        .number(
            TssElectricalMeasurement.AttributeDefs.calibration_voltage.name,
            TssElectricalMeasurement.cluster_id,
            unit=UnitOfElectricPotential.VOLT,
            min_value=12, max_value=380, step=1, multiplier=0.01,
            translation_key="calibration_voltage",
            fallback_name="Value for Voltage calibration",
            mode="box",
            endpoint_id=ENDPOINT,
        )
        .number(
            TssElectricalMeasurement.AttributeDefs.calibration_power.name,
            TssElectricalMeasurement.cluster_id,
            min_value=1, max_value=12160, step=1, multiplier=0.1,
            unit=UnitOfPower.WATT,
            translation_key="calibration_power",
            fallback_name="Value for Power calibration",
            mode="box",
            endpoint_id=ENDPOINT,
        )
        .enum(
            TssElectricalMeasurement.AttributeDefs.start_calibration.name,
            TssCalibration,
            TssElectricalMeasurement.cluster_id,
            translation_key="start_calibration",
            fallback_name="Start calibration",
            endpoint_id=ENDPOINT,
        )
        .sensor(
            TssElectricalMeasurement.AttributeDefs.start_calibration.name,
            TssElectricalMeasurement.cluster_id,
            attribute_converter=StatusCalibration_converter,
            translation_key="start_calibration",
            fallback_name="Calibration status",
            entity_type=EntityType.DIAGNOSTIC,
            reporting_config=ReportingConfig(
                min_interval=1, max_interval=900, reportable_change=1
            ),
            endpoint_id=ENDPOINT,
        )
        .enum(
            TssElectricalMeasurement.AttributeDefs.power_precision.name,
            TssPowerPrecision,
            TssElectricalMeasurement.cluster_id,
            translation_key="power_precision",
            fallback_name="Power Precision",
            endpoint_id=ENDPOINT,
        )
        .sensor(
            TssElectricalMeasurement.AttributeDefs.alarm_events.name,
            TssElectricalMeasurement.cluster_id,
            entity_type=EntityType.DIAGNOSTIC,
            attribute_converter=Alarm_converter,
            reporting_config=ReportingConfig(
                min_interval=1, max_interval=900, reportable_change=1
            ),
            translation_key="alarm_events",
            fallback_name="Alarm Events",
            endpoint_id=ENDPOINT,
        )
        .number(
            TssElectricalMeasurement.AttributeDefs.alarm_mask.name,
            TssElectricalMeasurement.cluster_id,
            min_value=0, max_value=63, step=1,
            translation_key="alarm_mask",
            fallback_name="Alarm mask",
            mode="box",
            endpoint_id=ENDPOINT,
        )
    )


# --------------------------------------------------------------------------
# Таблица моделей и их возможностей
# --------------------------------------------------------------------------

MODELS = {
    # базовый On/Off, без config-кластера и электрики
    "EM0SW1_z": {
        "config_cluster": False,
        "electrical": False,
        "switch_config_entities": False,
        "gpio_switch": False,
        "bl0937": False,
        "bl0942": None,             # None | "enabled" | "disabled"
        "calculating_freq": False,
    },
    # On/Off + config-кластер, switch-сущности, gpio_switch, без электрики
    "EM0SW2_z": {
        "config_cluster": True,
        "electrical": False,
        "switch_config_entities": True,
        "gpio_switch": True,
        "bl0937": False,
        "bl0942": None,
        "calculating_freq": False,
    },
    # On/Off + config + электрика + BL0942 (TX активен)
    "EM1SW1_z": {
        "config_cluster": True,
        "electrical": True,
        "switch_config_entities": False,
        "gpio_switch": False,
        "bl0937": False,
        "bl0942": "enabled",
        "calculating_freq": True,
    },
    # On/Off + config + электрика + BL0942 (TX скрыт)
    "EM1SW2_z": {
        "config_cluster": True,
        "electrical": True,
        "switch_config_entities": False,
        "gpio_switch": False,
        "bl0937": False,
        "bl0942": "disabled",
        "calculating_freq": True,
    },
    # On/Off + config + электрика + BL0937
    "EM8SW1_z": {
        "config_cluster": True,
        "electrical": True,
        "switch_config_entities": False,
        "gpio_switch": False,
        "bl0937": True,
        "bl0942": None,
        "calculating_freq": False,
    },
    # То же + switch-сущности и gpio_switch
    "EM8SW2_z": {
        "config_cluster": True,
        "electrical": True,
        "switch_config_entities": True,
        "gpio_switch": True,
        "bl0937": True,
        "bl0942": None,
        "calculating_freq": False,
    },
}


def build_quirk(model: str, cfg: dict) -> None:
    b = QuirkBuilder("Telink-pvvx", model)

    # Кластеры
    if cfg["config_cluster"]:
        b = (
            b.removes(TssOnOffConfiguration.cluster_id, endpoint_id=ENDPOINT)
            .adds(TssOnOffConfiguration)
        )

    b = (
        b.removes(TssOnOff.cluster_id, endpoint_id=ENDPOINT)
        .adds(TssOnOff)
    )

    if cfg["electrical"]:
        b = (
            b.removes(TssElectricalMeasurement.cluster_id, endpoint_id=ENDPOINT)
            .adds(TssElectricalMeasurement)
        )

    # Сущности конфигурации внешнего выключателя
    if cfg["switch_config_entities"]:
        b = add_switch_config(b)

    # Общий On/Off набор
    b = add_common_onoff(b)

    # Опциональные GPIO
    if cfg["gpio_switch"]:
        b = add_gpio_switch(b)

    if cfg["bl0937"]:
        b = add_bl0937_gpio(b)

    if cfg["bl0942"] is not None:
        b = add_bl0942_gpio(b, tx_disabled=(cfg["bl0942"] == "disabled"))

    # gpio_flags идёт после всех GPIO-настроек
    b = add_gpio_flags(b)

    # Электрические измерения
    if cfg["electrical"]:
        b = add_electrical(b, include_calculating_freq=cfg["calculating_freq"])

    b.add_to_registry()


for _model, _cfg in MODELS.items():
    build_quirk(_model, _cfg)