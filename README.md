# fxPwm

Software PWM for Arduino that allows pulse width modulation at any port with full control of duty cycle and frequency.

## Functions

### fxPwm_Initialize(numPorts, microsFunction)

Sets up the library with numPorts ports and a microsFunction.
microsFunction can be set NULL.

### fxPwm_ConfigurePort(portIndex, pinNumber);

Configures a port to output at the pinNumber pin.
0 <= portIndex < numPorts

### fxPwm_Start() fxPwm_Stop()

To start or stop the library. 

### fxPwm_SetFrequencyAndDuty(portIndex, frequency, duty);

Sets frequency and duty of a port.

### fxPwm_SetFrequency(portIndex, frequency) fxPwm_SetDuty(portIndex);

Sets frequency or duty of a port.

### fxPwm_Enable(portIndex) fxPwm_EnableAll()

Enables PWM at a single port or at all of them.

### fxPwm_Disable(portIndex) fxPwm_DisableAll()

Disables PWM at a single port or at all of them.
