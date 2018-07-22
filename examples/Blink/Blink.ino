/* fxPwm Blink
 * 
 * Blinks an LED with an frequency of 1 Hz. (0.5 second on, 0.5 second off, repeating)
 * 
 * Andrei Alves Cardoso, 18/07/2018
 * Updated 22/07/2018
 *
 */

#include <fxPwm.h>

void setup() {
  //Initialize fxPwm library.
  fxPwm.Initialize(1);
  fxPwm.Start();

  //Register the LED_BUILTIN pon.
  fxPwm.RegisterPort(LED_BUILTIN);

  //Sets the initial PWM frequency (1.0 Hz).
  fxPwm.SetFrequency(LED_BUILTIN, 1.0);
  //Sets the initial PWM duty cycle (50%).
  fxPwm.SetDuty(LED_BUILTIN, 0.5);


  //Enable LED_BUILTIN pin.
  fxPwm.EnablePin(LED_BUILTIN);
}

void loop() {
  //Do nothing. It will blink automatically.
}