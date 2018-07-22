/* fxPwm Blink
 * 
 * Fades a LED with an frequency of 1 Hz.
 * 
 * Andrei Alves Cardoso, 18/07/2018
 *
 */

#include <fxPwm.h>

//Frequency of the brightness, Hz.
float frequency=1.0;

void setup() {
  //Initialize fxPwm library.
  fxPwm.Initialize();
  fxPwm.Start();
  
  //Register the LED_BUILTIN pin.
  fxPwm.RegisterPort(LED_BUILTIN);

  //Sets the initial PWM frequency.
  fxPwm.SetFrequency(LED_BUILTIN, 300.0);

  //Maps the duty cycle to go from -1.0 at 0% to +1.0 at 100%, the standard sine function range.
  fxPwm.SetMap(LED_BUILTIN, 0.0, -1.0, 1.0, 1.0);
  
  //Enable LED_BUILTIN pin.
  fxPwm.EnablePin(LED_BUILTIN);
}


void loop() {
  //Get current time.
  float t = ((float)fxPwm.Micros()/1000000.0);
  //Calculate current brightness from a sine function.
  float brightness=sin(6.28*frequency*t);
  //Se duty cycle to change brightness.
  fxPwm.SetDuty(LED_BUILTIN, brightness);
  //Go on forever.
}
