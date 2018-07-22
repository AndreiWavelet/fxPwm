/* fxPwm Chord Progression
 * 
 * Plays a chord progression using 4 PWM ports.
 * The chord progression is: Cm Fm Cm G G/7 Cm
 * 
 * To test this example you must build one of the following circuits:
 *
 * The easiest way is to connect a buzzer into each used pin. (2, 3, 4 and 5).
 * 
 * If you have a small speaker, you can connect a 220 ohm resistor into each port.
 * Then you connect the other side of the resistors together at a breadboard.
 * Connect a capacitor (10 uF?) in series with the resistors, and the other side of the cap to the speaker.
 * The other side of the speaker go to ground.
 * 
 * 
 * Andrei Alves Cardoso, 18/07/2018
 * Updated 22/07/2018
 *
 */

#include <fxPwm.h>

//Frequencies of musical notes
//Formula: f = 440.0 * (2^(n/12))
//Where n is the relative half-step from A4 at 440.0 Hz.
#define FREQUENCY_B3  246.9
#define FREQUENCY_C4  261.6
#define FREQUENCY_D4  293.7
#define FREQUENCY_Ds4 311.1
#define FREQUENCY_F4  349.2
#define FREQUENCY_G4  392.0
#define FREQUENCY_Gs4 415.3

void setup() {
  //Initialize fxPwm library with four output.
  fxPwm.Initialize();
  fxPwm.Start();

  //Configures the ports to pins 2, 3, 4 and 5.
  fxPwm.RegisterPort(2);
  fxPwm.RegisterPort(3);
  fxPwm.RegisterPort(4);
  fxPwm.RegisterPort(5);
}

void loop() {
  //500 ms silent
  fxPwm.DisableAll();
  delay(500);
  fxPwm.EnableAll();

  //C minor
  fxPwm.SetFrequency(2, FREQUENCY_C4);
  fxPwm.SetFrequency(3, FREQUENCY_Ds4);
  fxPwm.SetFrequency(4, FREQUENCY_G4);
  delay(1000);

  //F minor
  fxPwm.SetFrequency(2, FREQUENCY_C4);
  fxPwm.SetFrequency(3, FREQUENCY_F4);
  fxPwm.SetFrequency(4, FREQUENCY_Gs4);
  delay(1000);

  //C minor
  fxPwm.SetFrequency(2, FREQUENCY_C4);
  fxPwm.SetFrequency(3, FREQUENCY_Ds4);
  fxPwm.SetFrequency(4, FREQUENCY_G4);
  delay(1000);

  //G
  fxPwm.SetFrequency(2, FREQUENCY_B3);
  fxPwm.SetFrequency(3, FREQUENCY_D4);
  fxPwm.SetFrequency(4, FREQUENCY_G4);
  delay(1000);

  //G/7
  fxPwm.SetFrequency(2, FREQUENCY_B3);
  fxPwm.SetFrequency(3, FREQUENCY_D4);
  fxPwm.SetFrequency(4, FREQUENCY_F4);
  fxPwm.SetFrequency(5, FREQUENCY_G4);
  delay(1000);

  //C minor
  fxPwm.SetFrequency(2, FREQUENCY_C4);
  fxPwm.SetFrequency(3, FREQUENCY_Ds4);
  fxPwm.SetFrequency(4, FREQUENCY_G4);
  fxPwm.SetFrequency(5, 0.0);
  delay(1500);
  
}
