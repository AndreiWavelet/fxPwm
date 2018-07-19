/* fxPwm Chord Progression
 * 
 * Plays a chord progression using 4 PWM ports.
 * The chord progression is: Cm Fm Cm G G/7 Cm
 * 
 * You must connect a buzzer into each port (2,3,4,5) for this to work;
 * 
 * Andrei Alves Cardoso, 18/07/2018
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
  fxPwm_Initialize(4,NULL);

  //Configures the ports to pins 2, 3, 4 and 5.
  fxPwm_ConfigurePort(0, 2);
  fxPwm_ConfigurePort(1, 3);
  fxPwm_ConfigurePort(2, 4);
  fxPwm_ConfigurePort(3, 5);

  //Starts the library.
  fxPwm_Start();
}

void loop() {
  //500 ms silent
  fxPwm_DisableAll();
  delay(500);
  fxPwm_EnableAll();

  //C minor
  fxPwm_SetFrequency(0, FREQUENCY_C4);
  fxPwm_SetFrequency(1, FREQUENCY_Ds4);
  fxPwm_SetFrequency(2, FREQUENCY_G4);
  delay(1000);

  //F minor
  fxPwm_SetFrequency(0, FREQUENCY_C4);
  fxPwm_SetFrequency(1, FREQUENCY_F4);
  fxPwm_SetFrequency(2, FREQUENCY_Gs4);
  delay(1000);

  //C minor
  fxPwm_SetFrequency(0, FREQUENCY_C4);
  fxPwm_SetFrequency(1, FREQUENCY_Ds4);
  fxPwm_SetFrequency(2, FREQUENCY_G4);
  delay(1000);

  //G
  fxPwm_SetFrequency(0, FREQUENCY_B3);
  fxPwm_SetFrequency(1, FREQUENCY_D4);
  fxPwm_SetFrequency(2, FREQUENCY_G4);
  delay(1000);

  //G/7
  fxPwm_SetFrequency(0, FREQUENCY_B3);
  fxPwm_SetFrequency(1, FREQUENCY_D4);
  fxPwm_SetFrequency(2, FREQUENCY_F4);
  fxPwm_SetFrequency(3, FREQUENCY_G4);
  delay(1000);

  //C minor
  fxPwm_SetFrequency(0, FREQUENCY_C4);
  fxPwm_SetFrequency(1, FREQUENCY_Ds4);
  fxPwm_SetFrequency(2, FREQUENCY_G4);
  fxPwm_SetFrequency(3, 0.0);
  delay(1500);
  
}
