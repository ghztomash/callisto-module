[![Discord](https://img.shields.io/discord/690660866475032596)](https://discord.gg/vHcUuzN)
[![Open Source](https://badges.frapsoft.com/os/v1/open-source.svg?v=103)](https://opensource.org/)

# callisto-module
Callisto Eurorack Module.

A Teensy 3.2 based eurorack audio synth module.
Although the original idea was a Drum Synthesis module, the hardware is designed in a general way to support any audio synthesis firmware using the Teensy Audio Library.

A Hardware Abstraction Library is provided to simplify reading of the hardware peripherals.

## Features
* 1 x Trigger input with LED.
* 1 x CV 1 V/Oct input.
* 5 x -5 to +5 V CV inputs.
* 1 x 12bit DAC output.
* 6 x Potentiometers.
* 2 x Mode buttons with 3 LEDs each.

## Specs
* 10 HP Width
* 0.068% THD @ 100 Hz (But loads of noise from DSP ;))
* 9.6 Vpp output.
* +12V: 80 mA, -12V: 7 mA, +5V: 0 mA