### Template Firmware

Empty project template

#### Callisto Hardware Abstraction Layer

**Class Methods:**
``` c 
void update() 
```
This function updates all of the ADC, Button and LED values. It calls `updateADC()`, `updateButtons()` and `updateLEDs()` under the hood.

``` c
void updateADC()
```
This function updates all of the new ADC values and applies calibration compensation to CV inputs.

``` c
void updateButtons()
```
This function updates the button states and updates the corresponding modes, it also fires off any registered button callbacks.

``` c
void updateLEDs()
```
This function updates the LEDs to their new values. LEDs are either automatically updated from their current mode, or by manually calling `setLED()` or `setTrigLED`.

``` c
int readADCRaw(int)
```

``` c
int readCVRaw(int)
```

``` c
float readCVNorm(int)
```

``` c
float readCVVolt(int)
```

``` c
float readCVPitch()
```

``` c
int readPotRaw(int)
```

``` c
float readPotNorm(int)
```

``` c
float readPotVolt(int)
```

``` c
float readPotPitch()
```

``` c
float readPitch()
```

``` c
void setPitchCVSource(int);
```

``` c
void setPitchPotSource(int);
```

``` c
void setLED(int, bool)
```

``` c
void setTrigLED(int)
```

``` c
bool readButton(int)
```

``` c
bool readTrigger()
```

``` c
int getMode(int)
```

``` c
void setMode(int, int)
```

``` c
void setAutoModeSwitch(bool);
```

``` c
void setModeCallback(int, void (*)(int))
```

``` c
void setButtonCallback(int, void (*)());
```

``` c
void setTriggerCallback(void (*)())
```
