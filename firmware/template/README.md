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
This function updates the LEDs to their new values. LEDs are either automatically set by the current mode, or by manually calling `setLED()` or `setTrigLED`.

``` c
int readADCRaw(int input)
```
Returns the raw ADC reading for a given input. Inputs are defined as CVA, POTA, CVB, POTB, etc up to CVF and POTF. Or 0 to 11.

``` c
int readCVRaw(int)
```
Returns the raw ADC reading for a CV input.

``` c
float readCVNorm(int)
```
Returns a normalized (from -1.0 to 1.0) ADC reading for a given CV input.

``` c
float readCVVolt(int)
```
Returns the input voltage for a given CV input. -3.0 V to +7.0 V for CV A and -5.0 V to +5.0 V for the rest CV inputs.

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
int getModeA()
```

``` c
int getModeB()
```

``` c
void setModeA(int)
```

``` c
void setModeB(int)
```

``` c
void setModeACallback(void (*)(int))
```

``` c
void setModeBCallback(void (*)(int))
```

``` c
void setTriggerCallback(void (*)())
```
