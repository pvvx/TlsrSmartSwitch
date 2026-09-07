"""Alternative firmwares for TlsrSmartSwitch on the TLSR825x/TLSR8656 chipset.
see https://github.com/pvvx/TlsrSmartSwitch
"""
from zigpy.quirks.v2 import QuirkBuilder, ReportingConfig, SensorDeviceClass, EntityType
#from typing import Final
from zhaquirks import CustomCluster
#from zigpy.zcl import ClusterType
from zigpy.zcl.clusters.general import OnOffConfiguration, SwitchType, MultistateInput, OnOff, Basic
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
	LedOff = 0
	LedOn = 1
	LedOnOff = 2


class TssOnOffCluster(CustomCluster, OnOff):

	class AttributeDefs(OnOff.AttributeDefs):
		"""relay_state = ZCLAttributeDef(
			id=0xf000,
			type=TssRelayIndicatorMode,
			access="r",
			is_manufacturer_specific=True,
		)"""
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

class TssOnOffConfigurationCluster(CustomCluster, OnOffConfiguration):

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

(
    QuirkBuilder("Telink-pvvx", "EM0SW1_z")
	.removes(TssOnOffCluster.cluster_id, endpoint_id=1)
	.adds(TssOnOffCluster)
	.switch(
		TssOnOffCluster.AttributeDefs.key_lock.name,
		TssOnOffCluster.cluster_id,
		off_value=0,
		on_value=1,
		translation_key="key_lock",
		fallback_name="Button lock",
	)
	.enum(
		TssOnOffCluster.AttributeDefs.led_control.name,
		TssLedControl,
		TssOnOffCluster.cluster_id,
		translation_key="led_control",
		fallback_name="LED Control",
		endpoint_id=1,
	)
    .add_to_registry()
)

(
    QuirkBuilder("Telink-pvvx", "EM0SW2_z")
	.removes(TssOnOffConfigurationCluster.cluster_id, endpoint_id=1)
	.adds(TssOnOffConfigurationCluster)
	.removes(TssOnOffCluster.cluster_id, endpoint_id=1)
	.adds(TssOnOffCluster)
	.enum(
		TssOnOffConfigurationCluster.AttributeDefs.switch_maction.name,
		TssSwitchActions,
		TssOnOffConfigurationCluster.cluster_id,
		translation_key="switch_maction",
		fallback_name="Ext.Switch actions",
		endpoint_id=1,
		#reporting_config=ReportingConfig(min_interval=0, max_interval=3600, reportable_change=1),
	)
	.enum(
		TssOnOffConfigurationCluster.AttributeDefs.switch_decoupled.name,
		TssSwitchDecoupled,
		TssOnOffConfigurationCluster.cluster_id,
		translation_key="switch_decoupled",
		fallback_name="Ext.Switch mode",
		endpoint_id=1,
	)
	.switch(
		TssOnOffCluster.AttributeDefs.key_lock.name,
		TssOnOffCluster.cluster_id,
		off_value=0,
		on_value=1,
		translation_key="key_lock",
		fallback_name="Button lock",
	)
	.enum(
		TssOnOffCluster.AttributeDefs.led_control.name,
		TssLedControl,
		TssOnOffCluster.cluster_id,
		translation_key="led_control",
		fallback_name="LED Control",
		endpoint_id=1,
	)
    .add_to_registry()
)

(
    QuirkBuilder("Telink-pvvx", "EM1SW1_z")
	.removes(TssOnOffConfigurationCluster.cluster_id, endpoint_id=1)
	.adds(TssOnOffConfigurationCluster)
	.removes(TssOnOffCluster.cluster_id, endpoint_id=1)
	.adds(TssOnOffCluster)
	.enum(
		TssOnOffConfigurationCluster.AttributeDefs.switch_maction.name,
		TssSwitchActions,
		TssOnOffConfigurationCluster.cluster_id,
		translation_key="switch_maction",
		fallback_name="Ext.Switch actions",
		endpoint_id=1,
		#reporting_config=ReportingConfig(min_interval=0, max_interval=3600, reportable_change=1),
	)
	.enum(
		TssOnOffConfigurationCluster.AttributeDefs.switch_decoupled.name,
		TssSwitchDecoupled,
		TssOnOffConfigurationCluster.cluster_id,
		translation_key="switch_decoupled",
		fallback_name="Ext.Switch mode",
		endpoint_id=1,
	)
	.switch(
		TssOnOffCluster.AttributeDefs.key_lock.name,
		TssOnOffCluster.cluster_id,
		off_value=0,
		on_value=1,
		translation_key="key_lock",
		fallback_name="Button lock",
	)
	.enum(
		TssOnOffCluster.AttributeDefs.led_control.name,
		TssLedControl,
		TssOnOffCluster.cluster_id,
		translation_key="led_control",
		fallback_name="LED Control",
		endpoint_id=1,
	)
    .add_to_registry()
)
