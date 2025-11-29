# Описание дополнительных (нестандартных) атрибутов для максимального варианта прошивки (типа EMxSW2TS):

#### Настройки и дополнительные атрибуты:

* Наличие некоторых кластеров или атрибутов зависит от варианта прошивки. 

### Cluster 0x0006: _OnOff_ 

* Attribute ***0xF000:*** _key_lock_ - текущее состояние реле (On/Off), read only
* Attribute ***0xF001:*** _led_control_ - 0: LED OFF, 1: LED ON, 2: LED ON/OFF
* Attribute ***0xF002:*** _Switch_Type_ - 0: TOGGLE, 1: MOMENTARY, 2: MULTIFUNCTION, 3: TERMOSTAT

* Attribute ***0xF100***: _gpio_realy_ - номер gpio для подключения реле
* Attribute ***0xF101***: _gpio_led1_ - номер gpio для подключения светодиода состояния
* Attribute ***0xF102***: _gpio_led2_ - номер gpio для подключения светодиода On/Off (=0 - не использется)
* Attribute ***0xF103***: _gpio_key_ - номер gpio для подключения кнопки
* Attribute ***0xF104***: _gpio_switch_ - номер gpio для подключения внешней кнопки (может совпадать с _gpio_swire_)
* Attribute ***0xF105***: _gpio_swire_ - номер gpio для подключения MY18B20 (может совпадать с _gpio_switch_)
* Attribute ***0xF106***: _gpio_sel_ - номер gpio для подключения BL0937 вывода SEL (8)
* Attribute ***0xF107***: _gpio_cf_ - номер gpio для подключения BL0937 вывода CF (6)
* Attribute ***0xF108***: _gpio_cf1_ - номер gpio для подключения BL0937 вывода CF1 (7)
* Attribute ***0xF109***: _gpio_rx_ - номер gpio для подключения BL0942 вывода TX (10), вариатны: [PA0,PB0,PB7,PC3,PC5,PD6]
* Attribute ***0xF10A***: _gpio_tx_ - номер gpio для подключения BL0942 вывода RX (9), вариатны: [PA2,PB1,PC2,PD0,PD3,PD7]

(!) Если номер GPIO не назначен, то устанавливливается номер GPIO по умолчанию (установленный в прошивке).

(!) Назначенные номер GPIO не должны совпадать. Выбор GPIO производится по имеющимся у модуля или чипа выводам.

(!) Обработка новых настроек GPIO обратывается только при перезагрузке устройства.
После перенастройки номеров GPIO следует перезапустить питание устройства. 

Номера GPIO:

| *GPIO* | NN | *GPIO* | NN | *GPIO* | NN | *GPIO* | NN | *GPIO* | NN |
| - | - | - | - | - | - | - | - | - | - |
| ***PA0*** | 0x0001 | ***PB0*** | 0x0101 | ***PC0*** | 0x0201 | ***PD0*** | 0x0301 | ***PE0*** | 0x0401 |
| ***PA1*** | 0x0002 | ***PB1*** | 0x0102 | ***PC1*** | 0x0202 | ***PD1*** | 0x0302 | ***PE1*** | 0x0402 |
| ***PA2*** | 0x0004 | ***PB2*** | 0x0104 | ***PC2*** | 0x0204 | ***PD2*** | 0x0304 | ***PE2*** | 0x0404 |
| ***PA3*** | 0x0008 | ***PB3*** | 0x0108 | ***PC3*** | 0x0208 | ***PD3*** | 0x0308 | ***PE3*** | 0x0408 |
| ***PA4*** | 0x0010 | ***PB4*** | 0x0110 | ***PC4*** | 0x0210 | ***PD4*** | 0x0310 |
| ***PA5*** | 0x0020 | ***PB5*** | 0x0120 | ***PC5*** | 0x0220 | ***PD5*** | 0x0320 |
| ***PA6*** | 0x0040 | ***PB6*** | 0x0140 | ***PC6*** | 0x0240 | ***PD6*** | 0x0340 |
| ***PA7*** | 0x0080 | ***PB7*** | 0x0180 | ***PC7*** | 0x0280 | ***PD7*** | 0x0380 |


### Cluster 0x0007: _On/Off Switch Configuration_ 

* Attribute ***0xF003***: _Switch_Decoupled_ - 0: Off, 1: On
* Attribute ***0xF004***: _Emergency_off_mask_ - задать отключение побитно: bit0: Max voltage, bit1: Min voltage, bit2: Max current, bit3: Max temperature, bit4: Min temperature
* Attribute ***0xF005***: _Alarm_EmOff_ - произошло отключение по _Emergency_off_mask_, reportable, write bitX=0 -> reset event


### Cluster 0x0b04: _Electrical Measurement_:

* Attribute ***0xF007***: Coefficient for calculating _current_ – значение коэффициента множителя для счетчика тока. См. описание для конкретного варианта датчика: BL0937/BL0942
* Attribute ***0xF008***: Coefficient for calculating _voltage_ – значение коэффициента множителя для счетчика напряжения. См. описание для конкретного варианта датчика: BL0937/BL0942
* Attribute ***0xF009***: Coefficient for calculating _power_ – значение коэффициента множителя для счетчика мощности. См. описание для конкретного варианта датчика: BL0937/BL0942
* Attribute ***0xF00A***: Coefficient for calculating _energy_ – значение коэффициента множителя для счетчика энергии. См. описание для варианта с датчиком BL0942.
* Attribute ***0xF00B***: Coefficient for calculating _freq_ – значение коэффициента для счетчика частоты. См. описание для варианта с датчиком BL0942.

### Cluster 0x0402: _Temperature Measurement_

* Attribute ***0xF00C***: _my18b20_id_ - серийный номер датчика MY18B20, 32 bit, read only
* Attribute ***0xF00D***: _my18b20_err_ - ошибки при работе с датчиком MY18B20: bit0: сбои при работе, bit1: не инициализирован, read only
* Attribute ***0xF00E***: _my18b20_coef_ - коэффициент множителя для преобразования значения от датчика MY18B20 в температуру в градусах C, по умолчанию равен 409600.
* Attribute ***0xF00F***: _my18b20_zero_ - смещение нуля для датчика MY18B20 в 0.01C, по умолчанию равен 0.
* Attribute ***0xF010***: _my18b20_hysteresis_ - гистерезис для датчика MY18B20 в 0.01C для уставки термостата, по умолчанию равен 0.15C.
* Attribute ***0xF011***: _emergency_max_temp_ - максимальная температура для аварийного отключения в 0.01C (см. _Emergency_off_mask_).
* Attribute ***0xF012***: _emergency_min_temp_ - минимальная температура для аварийного отключения в 0.01C (см. _Emergency_off_mask_).
            
Остальные описания имеющихся у устройства кластеров и атрибутов смотрите в документации Zigbee 3.0.

Полный список имеющихся у устройства кластеров и атрибутов, и их начальных значений, возможно получить путем сканирования устройства. 

К примеру, для HA с ZHA в zha_toolkit.scan_device - [Scan_EM8SW2TS.json](https://github.com/pvvx/TlsrSmartSwitch/blob/master/Scan_EM8SW2TS.yaml)...

