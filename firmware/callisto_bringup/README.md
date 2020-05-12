## Bringup Firmware

Firmware to test all hardware capabilities and ADC callibration.

### CLI commands
* `cal` run calibration.
* `save` save calibration to EEPROM.
* `load` reload calibration from EEPROM.
* `res` reset calibration to default.
* `amp` output amplitude from 0.0 to 1.0.
* `freq` output sinwave frequency.
* `size` sampling statistics size for callibration.
* `avr` ADC averaging setting.
* `bit` ADC bithdepth setting.
* `ref` set target voltage for ADC input offset.

### Usage
Use a VT100 compatible terminal, like TeraTerm set to 115200 Baud and CR + LF transmission.

![](https://github.com/ghztomash/callisto-module/blob/master/docs/callisto-bringup.png)