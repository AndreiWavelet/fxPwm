/* fxPwm Blink
 * 
 * Fades a LED with an frequency of 1 Hz.
 * 
 * Andrei Alves Cardoso, 18/07/2018
 *
 */

#include <fxPwm.h>

//Current brigthness, from 0.0 to 1.0.
float brightness = 0.0;

//Frequency of the brightness, Hz.
float frequency=1.0;

void setup() {
  //Initialize fxPwm library with one output.
  fxPwm_Initialize(1,NULL);

  //Configures the port index 0 with the LED_BUILTIN pin.
  fxPwm_ConfigurePort(0, LED_BUILTIN);

  //Sets the initial PWM frequency (1000.0 Hz) with standart duty cycle.
  fxPwm_SetFrequency(0, 1000.0);

  //Starts the library.
  fxPwm_Start();

  //Enable port index 0.
  fxPwm_Enable(0);
}


void loop() {
  //Calculate current brightness from a sine function.
  brightness=sin(6.283*frequency*((float)micros()/1000000.0))*0.5+0.5;
  //Se duty cycle to change brightness.
  fxPwm_SetDuty(0, brightness);
}
