# TlsrSmartSwitch
Alternative firmwares for Smart Switch on the TLSR925x chipset.

## The repository is under development!

Current FW Version: 0.0.0.3 (for test)

Supported device models:

* [Tuya ZigBee "Mini Zigbee Switch"](https://pvvx.github.io/TS0001_TZ3000_kqvb5akv/) (Module ZTU, Sensor: BL0942), FW name: MZSW01
* [Tuya ZigBee "Mini Smart Switch"](https://pvvx.github.io/TS000F_TZ3000_xkap8wtb/) (Module ZT2S, Sensor: BL0937), FW name: MZSW02

---

## Story:

The original FW structure for "Zigbee Smart Switch" was designed by [slacky1965](https://github.com/slacky1965).

The current versions of the published alternative firmware retain compatibility with [tuya_mini_relay_zrd](https://github.com/slacky1965/tuya_mini_relay_zrd) "quirks."

Optimizations have been made:

* Compatibility with ZHA and Zigbee 3.0 standards has been added.
* Firmware size for compatibility with chips with 512K flash memory.
* Chip hardware usage.
* Firmware loader for compatibility with OTA BLE and Zigbee variants.
* Added a variant with the BL0937 sensor.
* Custom settings of coefficients for the sensor.
* Disabling the relay based on voltage or current threshold values.
* Setting the relay restart interval after tripping based on thresholds.
* Setting the starting interval for threshold checking before the relay is turned on when power is restored.
* The start and restart intervals are counted using threshold values. If any threshold is triggered, the countdown begins again.

Further development:

* Adding a BLE beacon with the BTHome format.

---

[Описание устройств с датчиком BL0937](https://github.com/pvvx/TlsrSmartSwitch/blob/master/README_BL0937.md).

[Описание устройств с датчиком BL0942](https://github.com/pvvx/TlsrSmartSwitch/blob/master/README_BL0942.md).

[Telegram](https://t.me/pvvx_developments/39)
