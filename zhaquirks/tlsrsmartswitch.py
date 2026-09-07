"""Alternative firmwares for TlsrSmartSwitch on the TLSR825x/TLSR8656 chipset.
see https://github.com/pvvx/TlsrSmartSwitch
"""
from zigpy.quirks.v2 import QuirkBuilder, ReportingConfig, EntityType
from zigpy.quirks.v2.homeassistant import UnitOfTemperature, UnitOfTime, UnitOfElectricPotential, UnitOfElectricCurrent, UnitOfPower
from zhaquirks import CustomCluster
from zigpy.zcl.clusters.homeautomation import ElectricalMeasurement
from zigpy.zcl.clusters.general import OnOffConfiguration,  MultistateInput, OnOff
from zigpy.zcl.foundation import ZCLAttributeDef
import zigpy.types as t

class TssSwitchActions(t.enum8):
	Toggle = 0
	Momentary = 1
	Multifunction = 2
	#Thermostat = 4

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

class TssCalibration(t.enum8):
	Cal_ok = 0
	Cal_U = 1
	Cal_I = 2
	Cal_UI = 3
	Cal_P = 4
	Cal_UIP = 7
	ReCal_P = 8
	ErrCal_U = 129
	ErrCal_I = 130
	ErrCal_UI = 131
	ErrCal_P = 132
	ErrCal_UP = 133
	ErrCal_UIP = 135


class TssOnOff(CustomCluster, OnOff):

	class AttributeDefs(OnOff.AttributeDefs):
		relay_state = ZCLAttributeDef(
			id=0xf000,
			type=t.Bool,
			access="r",
			is_manufacturer_specific=True,
		)
		key_lock = ZCLAttributeDef(
			id=0xf001,
			type=t.Bool,
			access="rw",
			is_manufacturer_specific=True,
		)
		led_control = ZCLAttributeDef(
			id=0xf002,
			type=TssLedControl,
			access="rw",
			is_manufacturer_specific=True,
		)

class TssOnOffConfiguration(CustomCluster, OnOffConfiguration):

	class AttributeDefs(OnOffConfiguration.AttributeDefs):

		switch_maction = ZCLAttributeDef(
			id=0xF003,
			type=TssSwitchActions,
			access="rw",
			is_manufacturer_specific=True,
		)

		switch_decoupled = ZCLAttributeDef(
			id=0xF004,
			type=TssSwitchDecoupled,
			access="rw",
			is_manufacturer_specific=True,
		)

class TssElectricalMeasurement(CustomCluster, ElectricalMeasurement):

	class AttributeDefs(ElectricalMeasurement.AttributeDefs):

		alarm_mask =  ZCLAttributeDef(
			id=0xF005,
			type=TssAlarmMask,
			access="rw",
			is_manufacturer_specific=True,
		)

		alarm_events =  ZCLAttributeDef(
			id=0xF006,
			type=TssAlarmMask,
			access="rwp",
			is_manufacturer_specific=True,
		)

		calculating_current = ZCLAttributeDef(
			id=0xF007,
			type=t.uint32_t,
			access="rw",
			is_manufacturer_specific=True,
		)
		calculating_voltage = ZCLAttributeDef(
			id=0xF008,
			type=t.uint32_t,
			access="rw",
			is_manufacturer_specific=True,
		)
		calculating_power = ZCLAttributeDef(
			id=0xF009,
			type=t.uint32_t,
			access="rw",
			is_manufacturer_specific=True,
		)
		calculating_freq = ZCLAttributeDef(
			id=0xF00B,
			type=t.uint32_t,
			access="rw",
			is_manufacturer_specific=True,
		)
		calibration_current = ZCLAttributeDef(
			id=0xF080,
			type=t.uint16_t,
			access="rw",
			is_manufacturer_specific=True,
		)
		calibration_voltage = ZCLAttributeDef(
			id=0xF081,
			type=t.uint16_t,
			access="rw",
			is_manufacturer_specific=True,
		)
		calibration_power = ZCLAttributeDef(
			id=0xF082,
			type=t.uint16_t,
			access="rw",
			is_manufacturer_specific=True,
		)
		start_calibration = ZCLAttributeDef(
			id=0xF090,
			type=TssCalibration,
			access="rwp",
			is_manufacturer_specific=True,
		)
		power_precision = ZCLAttributeDef(
			id=0xF013,
			type=TssPowerPrecision,
			access="rw",
			is_manufacturer_specific=True,
		)
		
(
	QuirkBuilder("Telink-pvvx", "EM0SW1_z")
	.removes(TssOnOff.cluster_id, endpoint_id=1)
	.adds(TssOnOff)
	.switch(
		TssOnOff.AttributeDefs.key_lock.name,
		TssOnOff.cluster_id,
		off_value=0,
		on_value=1,
		translation_key="key_lock",
		fallback_name="Button lock",
	)
	.enum(
		TssOnOff.AttributeDefs.led_control.name,
		TssLedControl,
		TssOnOff.cluster_id,
		translation_key="led_control",
		fallback_name="LED Control",
		endpoint_id=1,
	)
	.add_to_registry()
)

(
	QuirkBuilder("Telink-pvvx", "EM0SW2_z")
	.removes(TssOnOffConfiguration.cluster_id, endpoint_id=1)
	.adds(TssOnOffConfiguration)
	.removes(TssOnOff.cluster_id, endpoint_id=1)
	.adds(TssOnOff)
	.enum(
		TssOnOffConfiguration.AttributeDefs.switch_maction.name,
		TssSwitchActions,
		TssOnOffConfiguration.cluster_id,
		translation_key="switch_maction",
		fallback_name="Ext.Switch actions",
		endpoint_id=1,
		#reporting_config=ReportingConfig(min_interval=0, max_interval=3600, reportable_change=1),
	)
	.enum(
		TssOnOffConfiguration.AttributeDefs.switch_decoupled.name,
		TssSwitchDecoupled,
		TssOnOffConfiguration.cluster_id,
		translation_key="switch_decoupled",
		fallback_name="Ext.Switch mode",
		endpoint_id=1,
	)
	.switch(
		TssOnOff.AttributeDefs.key_lock.name,
		TssOnOff.cluster_id,
		off_value=0,
		on_value=1,
		translation_key="key_lock",
		fallback_name="Button lock",
	)
	.enum(
		TssOnOff.AttributeDefs.led_control.name,
		TssLedControl,
		TssOnOff.cluster_id,
		translation_key="led_control",
		fallback_name="LED Control",
		endpoint_id=1,
	)
	.add_to_registry()
)

(
	QuirkBuilder("Telink-pvvx", "EM8SW1_z")
	.applies_to("Telink-pvvx", "EM1SW1_z")
	.removes(TssOnOffConfiguration.cluster_id, endpoint_id=1)
	.adds(TssOnOffConfiguration)
	.removes(TssOnOff.cluster_id, endpoint_id=1)
	.adds(TssOnOff)
	.removes(TssElectricalMeasurement.cluster_id, endpoint_id=1)
	.adds(TssElectricalMeasurement)
	.switch(
		TssOnOff.AttributeDefs.key_lock.name,
		TssOnOff.cluster_id,
		off_value=0,
		on_value=1,
		translation_key="key_lock",
		fallback_name="Button lock",
		endpoint_id=1,
	)
	.enum(
		TssOnOff.AttributeDefs.led_control.name,
		TssLedControl,
		TssOnOff.cluster_id,
		translation_key="led_control",
		fallback_name="LED Control",
		endpoint_id=1,
	)
	.number(
		ElectricalMeasurement.AttributeDefs.rms_extreme_over_voltage_period.name,
		ElectricalMeasurement.cluster_id,
		min_value=0,
		max_value=65535,
		step=1,
		unit=UnitOfTime.SECONDS,
		translation_key="restart_period_after_protection",
		fallback_name="Restart period after protection",
		mode="box",
		endpoint_id=1,
	)
	.number(
		ElectricalMeasurement.AttributeDefs.rms_extreme_under_voltage_period.name,
		ElectricalMeasurement.cluster_id,
		min_value=0,
		max_value=65535,
		step=1,
		unit=UnitOfTime.SECONDS,
		translation_key="poweron_test_period",
		fallback_name="Power-on test period",
		mode="box",
		endpoint_id=1,
	)
	.number(
		ElectricalMeasurement.AttributeDefs.rms_voltage_swell_period.name,
		ElectricalMeasurement.cluster_id,
		min_value=0,
		max_value=65535,
		step=1,
		unit=UnitOfTime.SECONDS,
		translation_key="overcurrent_period",
		fallback_name="Overcurrent period before Off",
		mode="box",
		endpoint_id=1,
	)
	.number(
		ElectricalMeasurement.AttributeDefs.rms_extreme_over_voltage.name,
		ElectricalMeasurement.cluster_id,
		min_value=0,
		max_value=380,
		step=1,
		multiplier=0.01,
		unit=UnitOfElectricPotential.VOLT,
		translation_key="rms_extreme_over_voltage",
		fallback_name="Over voltage",
		mode="box",
		endpoint_id=1,
	)
	.number(
		ElectricalMeasurement.AttributeDefs.rms_extreme_under_voltage.name,
		ElectricalMeasurement.cluster_id,
		min_value=0,
		max_value=380,
		step=1,
		multiplier=0.01,
		unit=UnitOfElectricPotential.VOLT,
		translation_key="rms_extreme_under_voltage",
		fallback_name="Under voltage",
		mode="box",
		endpoint_id=1,
	)
	.number(
		ElectricalMeasurement.AttributeDefs.rms_voltage_swell.name,
		ElectricalMeasurement.cluster_id,
		min_value=0,
		max_value=32000,
		step=1,
		# if Amper: multiplier=0.001,
		unit=UnitOfElectricCurrent.MILLIAMPERE,
		translation_key="rms_extreme_over_current",
		fallback_name="Over current",
		mode="box",
		endpoint_id=1,
	)
	.number(
		TssElectricalMeasurement.AttributeDefs.calculating_current.name,
		TssElectricalMeasurement.cluster_id,
		min_value=0,
		max_value=4294967295,
		step=1,
		translation_key="calculating_current",
		fallback_name="Calc.Coef. current",
		mode="box",
		endpoint_id=1,
	)
	.number(
		TssElectricalMeasurement.AttributeDefs.calculating_voltage.name,
		TssElectricalMeasurement.cluster_id,
		min_value=0,
		max_value=4294967295,
		step=1,
		translation_key="calculating_voltage",
		fallback_name="Calc.Coef. voltage",
		mode="box",
		endpoint_id=1,
	)
	.number(
		TssElectricalMeasurement.AttributeDefs.calculating_power.name,
		TssElectricalMeasurement.cluster_id,
		min_value=0,
		max_value=4294967295,
		step=1,
		translation_key="calculating_power",
		fallback_name="Calc.Coef. power",
		mode="box",
		endpoint_id=1,
	)
	.number(
		TssElectricalMeasurement.AttributeDefs.calculating_freq.name,
		TssElectricalMeasurement.cluster_id,
        initially_disabled=True,
		min_value=0,
		max_value=4294967295,
		step=1,
		translation_key="calculating_freq",
		fallback_name="Calc.Coef. freq",
		mode="box",
		endpoint_id=1,
	)
	.number(
		TssElectricalMeasurement.AttributeDefs.calibration_current.name,
		TssElectricalMeasurement.cluster_id,
        initially_disabled=True,
		unit=UnitOfElectricCurrent.MILLIAMPERE,
		min_value=100,
		max_value=32000,
		step=1,
		translation_key="calibration_current",
		fallback_name="Value for current calibration",
		mode="box",
		endpoint_id=1,
	)
	.number(
		TssElectricalMeasurement.AttributeDefs.calibration_voltage.name,
		TssElectricalMeasurement.cluster_id,
        initially_disabled=True,
		unit=UnitOfElectricPotential.VOLT,
		min_value=12,
		max_value=380,
		step=1,
		multiplier=0.01,
		translation_key="calibration_voltage",
		fallback_name="Value for voltage calibration",
		mode="box",
		endpoint_id=1,
	)
	.number(
		TssElectricalMeasurement.AttributeDefs.calibration_power.name,
		TssElectricalMeasurement.cluster_id,
        initially_disabled=True,
		min_value=1,
		max_value=12160,
		step=1,
		multiplier=0.1,
		unit=UnitOfPower.WATT,
		translation_key="calibration_power",
		fallback_name="Value for power calibration",
		mode="box",
		endpoint_id=1,
	)
	.enum(
		TssElectricalMeasurement.AttributeDefs.start_calibration.name,
		TssCalibration,
		TssElectricalMeasurement.cluster_id,
        initially_disabled=True,
		translation_key="start_calibration",
		fallback_name="Start and Status calibration",
		reporting_config=ReportingConfig(
			min_interval=0, max_interval=900, reportable_change=1
		),
		endpoint_id=1,
	)
	.enum(
		TssElectricalMeasurement.AttributeDefs.power_precision.name,
		TssPowerPrecision,
		TssElectricalMeasurement.cluster_id,
		translation_key="power_precision",
		fallback_name="Power Precision",
		endpoint_id=1,
	)
	.sensor(
		TssElectricalMeasurement.AttributeDefs.alarm_events.name,
		TssElectricalMeasurement.cluster_id,
		entity_type=EntityType.DIAGNOSTIC,
		reporting_config=ReportingConfig(
			min_interval=0, max_interval=900, reportable_change=1
		),
		translation_key="alarm_events",
		fallback_name="Alarm Events",
		endpoint_id=1,
	)
	.number(
		TssElectricalMeasurement.AttributeDefs.alarm_mask.name,
		TssElectricalMeasurement.cluster_id,
		min_value=0,
		max_value=63,
		step=1,
		fallback_name="alarm_mask",
		translation_key="Alarm mask",
		mode="box",
		endpoint_id=1,
	)
	.add_to_registry()
)

