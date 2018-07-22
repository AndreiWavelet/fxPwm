# fxPwm

Software PWM for Arduino that allows pulse width modulation at any port with full control of duty cycle and frequency.

This software is released into the public domain. See LICENSE for more info.

## Em português

PWM por software para Arduino, que permite modulação por largura de pulso em qualquer porta com total controle de ciclo de trabalho e frequência.
A documentação está disponível em português no diretório "extras".

Me ajude a traduzir se achar por bem, meu inglês não é tão forte.
Se quiser pode contribuir com código também. :)

Esse software está liberado em domínio público. Você pode usar livremente para qualquer propósito, mas SEM QUALQUER GARANTIA. Consulte o documento de licença (LICENSE) para mais informação.

## How to use

The standard recipe to use this library is as follows:

### 1. Initialize the library and start the modulator:

fxPwm.Initialize();
fxPwm.Start();

### 2. Register pin:

fxPwm.RegisterPort(pinNumber);

You must register every pin you use. By default, you can register up to 32 ports.
If you need more pins, use fxPwm.Initialize(maxPins) at init.

### 3. Configure pin frequency:

fxPwm.SetFrequency(pinNumber, frequency);

You also must configure the frequency por every pin. Choose the appropriate frequency for your application. 300 Hz is a good start for LED brightness control.
Lower frequencies have better duty cycle resolution.
Higher frequencies may cause jitter. Using a lot of ports with very high frequency may not be a good idea.

### 4. Enable pin:

fxPwm.EnablePin(pinNumber);
fxPwm.EnableAll();

### 5. Change duty cycle in any way you need.

fxPwm.SetDuty(pinNumber, dutyCycle);

### 6. Disable PWM, remove ports and release.

fxPwm.DisablePin(pinNumber);
fxPwm.DisableAll();
fxPwm.RemovePort(pinNumber);
fxPwm.Stop();
fxPwm.Free();

In most real life cases, these steps are unnecessary, as the microcontroller is expected to be running for long periods.

## Initialization and Release Functions

### fxPwm.Initialize() fxPwm.Initialize(maxPorts)

Sets up the library with the dafault maximum quantity of ports or a defined maxPorts.

### fxPwm.Free()

Releases any resources used by the library.

### fxPwm.Start();

Starts modulation.

### fxPwm.Stop();

Stops modulation.

## Port Manipulation Functions

### fxPwm.RegisterPort(pinNumber);

Registers a pin number to be used as PWM output.

### fxPwm.RemovePort(pinNumber);

Removes a port associated to a pin number.

### fxPwm.SetPeriod(pinNumber, period);

Sets the period of the PWM cycle, in microseconds. Note that the actual period may vary due to timer resolution limitations.
Also note that very low periods (<1000 us) (or many ports enabled at once) may cause jitter.

### fxPwm.SetFrequency(pinNumber, frequency);

Sets the frequency of the PWM cycle, in hertz (cycles per second). Note that the actual frequency may vary due to timer resolution limitations.
Also note that very high frequencies (>1000 hz) (or many ports enabled at once) may cause jitter.

### fxPwm.SetDuty(pinNumber, duty);

Sets the duty cycle of the PWM cycle. By default, this value goes from 0.0 (at 0% duty cycle) to 1.0 (at 100% duty cycle).
If fxPwm.SepMap() had been called before fxPwm.SetDuty, the duty cycle will be mapped to a different function.

### fxPwm.SetMap(pinNumber, duty1, value1, duty2, value2);

Maps the duty cycle so that the range duty1 ~ duty2 becomes value ~ value2.
This is useful if your controlled variable is not in the range 0.0 ~ 1.0.

### fxPwm.EnablePin(pinNumber); fxPwm.DisablePin(pinNumber);

Enables or disables modulation at the specified pinNumber, if it is registered.

### fxPwm.EnableAll(); fxPwm.DisableAll();

Enables or disables modulation at all registered pins.

## Data Acquisition Functions

### UIN8 fxPwm.GetMaxPorts();

Returns the maximum quantity of registrable ports.

### UINT8 fxPwm.GetNumRegisteredPorts();

Returns the number of currently registered ports.

### TIME_US fxPwm.GetPeriod(pinNumber);

Returns the configured period, in microseconds, of the port with the specified pin number. If it doesn't exist, returns 0.

### FLOAT fxPwm.GetFrequency(pinNumber);

Returns the configured frequency, in hertz, of the port with the specified pin number. If it doesn't exist, returns 0.

### FLOAT fxPwm.GetDuty(pinNumber);

Returns the configured mapped duty cycle of the port with the specified pin number. If it doesn't exist, returns 0.

### TIME_US fxPwm.Micros();

Return the number of microseconds passed since fxPwm.Start() first called. It stops counting when fxPwm.Stop() is called, and then resumes every time  fxPwm.Start() is called.
The resolution may vary due to timer resolution limitations.

## Advanced Functions

### RegisterPort(fxPwm_Port *port); RemovePort(fxPwm_Port *port);

Registers or removes a port from a pointer to a user allocated fxPwm_Port object.
Take care when using it as it may break the class structure.

### fxPwm_Port* GetPort(pin);

Returns the pointer to a fxPwm_Port object, enabling direct manipulation of port data.

### UINT8 GetIndex(pin);

Returns the internal index of a pin. It is not recommended to use this function, as the index may change when removing ports.
If the pin isn't registered, returns 0xFF.

### UINT8 GetRegisteredPortPinNumber(index)

Returns the pin number from a internal index. If the index is out of bounds, returns 0xFF.

## fxPwm_Port Functions (advanced)

### fxPwm_Port::SetPinNumber(pinNumber);

Sets the pin number of a port. Take care when using this, as setting the same pin number for 2 or more registered ports may break things.

### TIME_US fxPwm_Port::GetPeriod(); FLOAT fxPwm_Port::GetDuty(); FLOAT fxPwm_Port::GetFrequency();

Returns the configured values. The returned duty is mapped. Period: microseconds. Frequency: hertz.

### FLOAT fxPwm_Port::GetRawDuty();

Returns the actual unmapped duty cycle value. 0.0 ~ 1.0 (0% ~ 100%)

### BOOL fxPwm_Port::GetPinState(); fxPwm_Port::SetPinState(BOOL state);

Gets or sets the pin state (LOW or HIGH) of the associated pin.
This function will force the pin to enter OUTPUT state.

### fxPwm_Port::SetPeriodAndDuty(period, duty); fxPwm_Port::SetFrequencyAndDuty(frequency, duty);

Sets both period (microseconds) and mapped duty cycle OR both frequency (hertz) and mapped duty cycle.

### fxPwm_Port::SetPeriod(period); fxPwm_Port::SetDuty(duty); fxPwm_Port::SetFrequency(frequency); 

Sets period (microseconds), mapped duty cycle or frequency (hertz).

### fxPwm_Port::SetMap(duty1, value1, duty2, value2);

See fxPwm.SepMap(pinNumber, duty1, value1, duty2, value2) above; it works the same way except it doesn't have the pinNumber parameter.

### fxPwm_Port::Enable(); fxPwm_Port::Disable();

Enables or disabled modulation at this port.

## Other functions

The library has other functions of less utility. See the source files for more info.

## Known Issues

### Serial communications may break things

Usually, calling fxPwm.Start() before and after serial communications solves the problem.

### (Theoretical) Clock count overflow

Theoretically, given enough time, the internal clock count will overflow and things will break.
The time needed for this to happen depends on the timer resolution. With a resolution of 500 ns with F_CPU at 16 MHz, this will happen after about 35 minutes and 47 seconds.
Lowering the resolution (fxPwm_MinTimerResolution at fxPwm.h) may increase this.
Defining TIME_IS_64 before including the library will greatly increase this limit with some performance penalty. (at 500 ns, 16 MHz, it will overflow after 292471 years!)


