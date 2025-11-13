# TlsrSmartSwitch
Alternative firmwares for Smart Switch on the TLSR925x chipset.

## The repository is under development!

---

### Story:

The original FW structure for "Zigbee Smart Switch" was designed by [slacky1965](https://github.com/slacky1965).

The current versions of the published alternative firmware retain compatibility with [slacky1965's](https://github.com/slacky1965/tuya_mini_relay_zrd) "quirks."

Optimizations have been made:

* Compatibility with ZHA and Zigbee 3.0 standards has been added.
* Firmware size for compatibility with chips with 512K flash memory.
* Chip hardware usage.
* Firmware loader for compatibility with OTA BLE and Zigbee variants.
* Added a variant with the BL0937 sensor.

Further development

* Custom settings of coefficients for the sensor.
* Adding a BLE beacon with the BTHome format.
  
