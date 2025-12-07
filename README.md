# TlsrSmartSwitch
Alternative firmwares for Smart Switch on the TLSR925x chipset.

Current FW Version: 0.0.0.6 (beta)

## Supported device models:

![MiniZigbeeSwitchWitchPowerMonitor.jpg](https://raw.githubusercontent.com/pvvx/TlsrSmartSwitch/refs/heads/master/img/MiniZigbeeSwitchWitchPowerMonitor.jpg)

Default:
* [Tuya ZigBee "Mini Zigbee Switch"](https://pvvx.github.io/TS0001_TZ3000_kqvb5akv/) (Sensor: **BL0942**), FW name: ***EM1SW**<**1**/**2**>[**T**/**TS**]*
* [Tuya ZigBee "Mini Smart Switch"](https://pvvx.github.io/TS000F_TZ3000_xkap8wtb/) (Sensor: **BL0937**), FW name: ***EM8SW**<**1/2**>[**T**/**TS**]*

And others, with sensors BL0942 or BL0937 (GPIOs are assigned by a separate [utility](https://pvvx.github.io/TlsrSmartSwitch/zigbeeswconfig.html), the all coefficients are set online)

---

## *Story:*

The original FW structure for "Zigbee Smart Switch" was designed by [slacky1965-tuya_mini_relay_zrd](https://github.com/slacky1965/tuya_mini_relay_zrd).

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
* Added thermometer and thermostat with MY18B20 sensor.
* The flash memory's lifespan has been extended to maintain energy metering. Energy is recorded every minute. For 512 KB flash memory, the minimum write life is 777 years (100K Program-Erase cycles), and for 1 MB flash memory, it's over 19,813.5 years.
* Added GPIOs modification.
* [GPIOs modification utility for ready-made firmware](https://pvvx.github.io/TlsrSmartSwitch/zigbeeswconfig.html).

---

## *Обозначение типа прошивки:*

Все устройства имеют датчик мониторинга электросети BL0937/BL0942.  

Начальные символы в названии “**EM1**” (Energy Metering)   указывают, что это прошивка  для варианта устройства с датчиком BL0942. “**EM8**” – для устройства с датчиком BL0937. Число дополнительно указывает на минимальный шаг в секундах периода измерения датчиком.

Следующие символы “**SW1**” (Switch)  - у устройства всего одна кнопка. “**SW2**” – есть дополнительная кнопка или клеммы.

Далее могут быть символы “**T**” или “**TS**”:

“**T**” обозначает, что используется датчик MY18B20 и устройство  имеет кластер для передачи текущей  температуры с контролем по аварийным границам.

“**TS**” обозначает, что используется датчик MY18B20 и устройство  имеет кластер для передачи текущей  температуры с контролем по аварийным границам и кластер термостата.


Пример: “**EM8SW1TS**” – устройство с кнопкой и к дополнительным клеммам “*S1*” и “*S2*” подключен датчик MY18B20 для передачи температуры с отслеживанием аварийных границ  и возможности работы  в качестве термостата.


## *Краткое описание имеющихся кластеров и атрибутов:*

[Описание устройств с датчиком BL0937](https://github.com/pvvx/TlsrSmartSwitch/blob/master/README_BL0937.md).

[Описание устройств с датчиком BL0942](https://github.com/pvvx/TlsrSmartSwitch/blob/master/README_BL0942.md).

[Описание дополнительных (нестандартных) атрибутов для максимального варианта прошивки](https://github.com/pvvx/TlsrSmartSwitch/blob/master/README_ExtAttr.md).


## Thermostat

![thermostat1.gif](https://raw.githubusercontent.com/pvvx/TlsrSmartSwitch/refs/heads/master/img/thermostat1.gif)

Прошивка FW, содержащая термостат, при включении любого рабочего режима для термостата:
1.	Отключает обработку кнопок и команд On/Off. При отключении режима термостата в режим “выключен” обработка кнопок и команд On/Off производится в обычном режиме.
2.	Управление реле подчиняется условиям, установленным для термостата и остальным заданным граничным условиям.
3.	Скорость переключения термостата зависит от скорости изменения температуры и заданного гистерезиса в [my18b20_hysteresis](https://github.com/pvvx/TlsrSmartSwitch/blob/master/README_ExtAttr.md#cluster-0x0402-temperature-measurement). Проверка температуры производится каждую секунду.

Пример работы термостата на нагрев: 
Датчик MY18B20 находится у баллона  лампы накаливания на 40 Вт.

![thermostat2.gif](https://raw.githubusercontent.com/pvvx/TlsrSmartSwitch/refs/heads/master/img/thermostat2.gif)

В ZHA по умолчанию задаются слишком большие интервалы для репорта, и возможность изменить значения в репортах у пользователя отсутствуют. В итоге, если температура меняется быстрее назначенных в репорте значений,  ZHA может пропускать переключения состояний термостата работающего в устройстве.

![thermostat3.gif](https://raw.githubusercontent.com/pvvx/TlsrSmartSwitch/refs/heads/master/img/thermostat3.gif)

## Программа конфигурации GPIO.

![config.gif](https://raw.githubusercontent.com/pvvx/TlsrSmartSwitch/refs/heads/master/img/config.gif)

[Программа конфигурации GPIO](https://pvvx.github.io/TlsrSmartSwitch/zigbeeswconfig.html) служит для конфигурации собственных  вариантов прошивок.
 
Назначенная в FW конфигурация GPIO  при первом запуске сохраняется во Flash, в специальную область, не перезаписываемую при сбросе устройства или обновлению через Zigbee OTA.
Далее это сохранение является приоритетным - изменить GPIO возможно только с помощью изменения значений в [кластере On/Off](https://github.com/pvvx/TlsrSmartSwitch/blob/master/README_ExtAttr.md#cluster-0x0006-onoff).
Применение измененных значений GPIO произойдет после пересброса устроства по питанию.

Большинство "Умных розеток" практически неразборные. Требуется корежить корпус очень аккуратно, чтобы открыть - он клееный.
И ошибка с выбором GPIO кнопки может потребовать такой разборки для очистки Flash и перепрошивки программатором.
Установки GPIO необходимо создать для первой версии обновления оригинальной прошивки через ZigbeeOTA правильно, без ошибок для GPIO кнопки.
Иначе, если GPIO кнопки не совпадет, тогда могут возникнуть проблемы со сбросом устройства для новой регистрации в сети Zigbee.

В дальнейших обновлениях через ZigbeeOTA можно использовать любую новую версию алтернативной прошивки, не смотря на то, какие в ней назначены GPIO.
После Zigbee OTA с новой версией FW будут приняты ранее сохраненные в этом устройстве значения для GPIO.


## Подключение датчика MY18B20


![MY18B20.jpg](https://raw.githubusercontent.com/pvvx/TlsrSmartSwitch/refs/heads/master/img/MY18B20.jpg) ![Wire.jpg](https://raw.githubusercontent.com/pvvx/TlsrSmartSwitch/refs/heads/master/img/Wire.jpg)

---

## ...

* Требуется помощь в проверке всех вариантов прошивок и комбинаций установок, а так-же в создании скриптов "причуд" для ZHA и Z2M.

[Telegram](https://t.me/pvvx_developments/39)

