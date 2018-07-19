/* fxPwm Blink
 * 
 * Blinks an LED with an frequency of 1 Hz. (0.5 second on, 0.5 second off, repeating)
 * 
 * Andrei Alves Cardoso, 18/07/2018
 *
 */

#include <fxPwm.h>

void setup() {
  //Initialize fxPwm library with one output.
  fxPwm_Initialize(1,NULL);

  //Configures the port index 0 with the LED_BUILTIN pin.
  fxPwm_ConfigurePort(0, LED_BUILTIN);

  //Sets the initial PWM frequency (1.0 Hz) and duty cycle (50%).
  fxPwm_SetFrequencyAndDuty(0, 1.0, 0.5);

  //Starts the library.
  fxPwm_Start();

  //Enable port index 0.
  fxPwm_Enable(0);
}

void loop() {
  //Do nothing.
}
