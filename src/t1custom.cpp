/*  -----------------------------------------------------------
 *  Andrei Alves Cardoso 16-07-2018
 *  -----------------------------------------------------------
 *  t1custom.cpp
 *  Cabeçalho contendo definições de funções úteis para
 *  manipulação do timer1 em microcontroladores presentes
 *  em placas Arduino.
 *  Parte da biblioteca FxPwm.
 *  -----------------------------------------------------------
 *  Você pode usar livremente esse programa para quaisquer fins,
 *  porém NÃO HÁ GARANTIA para qualquer propósito.
 *  -----------------------------------------------------------
 *  17-07-2018: primeira documentação
 */

#include "arduino.h"
#include <fxPwmTypes.h>

#include <t1custom.h>

#define t1c_SAVE_SREG() BYTE sreg = SREG

#define t1c_RESTORE_SREG() SREG=sreg

#define t1c_CLEAR_BITS(value,mask)    (value&(~mask)) 
#define t1c_SET_BITS(value,mask)      (value|mask)
#define t1c_INVERT_BITS(value, mask)  (value^mask)

volatile TIME_US t1c_currentMicros = (TIME_US)0;
volatile UINT32 t1c_lastMicros = 0;

volatile BOOL t1c_isPeriodic = FALSE;
volatile UINT16 t1c_period;
volatile UINT8 t1c_clockBits = 0;

volatile TIME_US t1c_nextFire = TIME_US_MAX;


/*Callback.
Vai ser chamada depois do atraso configurado.
A função
*/
void (*t1c_callback)() = NULL;

//Callback para função que retorna a quantidade de microssegundos passada.
TIME_US (*t1c_MicrosCallback)() = t1c_Micros;

inline UINT8 t1c_GetPrescalerData(UINT32 periodClocks, volatile UINT16 *numClocks) __attribute__((always_inline));

/* Callback vazio para o caso do usuário passar NULL para o callback.
 */
void t1c_NullCallback(){
  //Nada...
}

/*
 * Método que retorna o tempo passado desde o início do mundo, em microssegundos.
 * Ela é feita para contar em 64 bits sem estourar depois de 70 minutos.
 */
TIME_US t1c_Micros(){
  #ifdef TIME_US_IS_64
    UINT32 currentMicros = micros();
    t1c_currentMicros+=((TIME_US)(currentMicros-t1c_lastMicros));
    t1c_lastMicros = currentMicros;
    return t1c_currentMicros;
  #else
    return micros();
  #endif
}

ISR(TIMER1_OVF_vect, ISR_NOBLOCK)          
{
  //Gerencia a vida de forma diferente se for periódico ou não.
  if(t1c_isPeriodic==TRUE){
    t1c_nextFire+=
    TCNT1 = -t1c_period;
  }else{
    TCCR1B = 0b00000000;
  }
  t1c_callback();
}

//Retorna os bits do prescalar do temporizador, baseado no valor do período (em Us)
//Coloca em numClocks a quantidade de ciclos do temporizador para disparar esse período.
//Observe que o máximo período suportado é de 67108863 uS. Mais que isso e ele vai falhar.
inline UINT8 t1c_GetPrescalerData(UINT32 period,volatile UINT16 *numClocks){
  if(period<t1c_Resolution){
    *numClocks = period;
    return 0b001;
  }else if(period<(t1c_Resolution<<3)){
    *numClocks = (period>>3);
    return 0b010;
  }else if(period<(t1c_Resolution<<6)){
    *numClocks = (period>>6);
    return 0b011;
  }else if(period<(t1c_Resolution<<8)){
    *numClocks = (period>>8);
    return 0b100;
  }else if(period<(t1c_Resolution<<10)){
    *numClocks = (period>>10);
    return 0b101;
  }else{
    //Ferrou.
    *numClocks = 65535;
    return 0b101;
  }
}

/*Configura miudezas do timer1.
 * 
 * Nesse caso, queremos que o timer chame a interrupção somente quando ocorrer sobrecarga na contagem.
 * Não haverá PWM nem nada dessas coisas chiques.
 * Para que isso aconteça, o modo de operação em que ser 0b000 (NORMAL).
 * Sem captura de entrada, portanto sem cancelamento de ruído.
 * A escolha do clock depende do tempo de atraso. Nessa função isso não vai ser usado.
 * Logo o valor do clock será 0 (isto é, temporizador parado).
 */
void t1c_Configure(void (*callback)(), TIME_US (*microsFunction)()){
  //Salva contexto do SREG.
  t1c_SAVE_SREG();
  //Limpa flag de interrupção.
  cli();

  //Limpa registro de controle 1A. (sem captura, sem clock).
  //Eu uso a máscara 0b11110011 para evitar mexer com bits não documentados.
  TCCR1A = t1c_CLEAR_BITS(TCCR1A, 0b11110011);

  //Sem cancelamento de ruído.
  //Sem captura de borda.
  //Modo normal.
  //Sem fonte de clock. (por enquanto)
  TCCR1B = t1c_CLEAR_BITS(TCCR1B, 0b11011111);

  //Sem frescuras.
  TCCR1C = t1c_CLEAR_BITS(TCCR1C, 0b11000000);

  //Coloca a contagem atual como 1.
  TCNT1 = 0;

  //Mais algumas limpezas para ficar bonito.
  ICR1 = 0;
  OCR1A = 0;
  OCR1B = 0;
  
  //Habilita apenas interrupção por sobrecarga de contagem.
  TIMSK1 = t1c_CLEAR_BITS(TIMSK1, 0b00100111);
  TIMSK1 = t1c_SET_BITS(TIMSK1, 0b00000001);
  
  //Atribui callbacks.
  t1c_callback = (callback==NULL)?(t1c_NullCallback):(callback);
  t1c_MicrosCallback = (microsFunction==NULL)?(t1c_Micros):(microsFunction);

  t1c_clockBits = 0b000;
  t1c_period = 0;
  t1c_isPeriodic = FALSE;

  //Restaura SREG
  t1c_RESTORE_SREG();
  
  return;
}

void t1c_Start(){
  t1c_Stop();
  TCCR1B = t1c_SET_BITS(TCCR1B, t1c_CLEAR_BITS(t1c_clockBits, 0b11111000));
}

void t1c_Stop(){
  TCCR1B = t1c_CLEAR_BITS(TCCR1B, 0b00000111);
}

void t1c_Reset(){
  TCNT1 = 0;
}

//Seta o período; garante um mínimo de 4 us.
void t1c_SetPeriod(TIME_US period, BOOL periodic){
  INT32 overheadTune = ((periodic)?(t1c_PeriodOverheadTune):(t1c_SetPeriodOverheadTune));
  period = (overheadTune<0)?( ((-overheadTune+4)>period)?(4):(period+overheadTune) ):(period+overheadTune);
  t1c_clockBits = t1c_GetPrescalerData(((INT32)period)*t1c_CyclesPerUs, &t1c_period);
  t1c_isPeriodic = periodic;
  TCNT1 = -t1c_period;
  t1c_Start();

  t1c_nextFire = TIME_US_MAX;
}

void t1c_SetNextFire(TIME_US timeMicros){
  TIME_US currentTime = t1c_MicrosCallback();
  TIME_US cycles = 4;
  if(timeMicros>=currentTime+4){
    cycles = (timeMicros-currentTime)*t1c_CyclesPerUs;
  }
  
  t1c_clockBits = t1c_GetPrescalerData(cycles, &t1c_period);
  t1c_isPeriodic = FALSE;
  TCNT1 = -t1c_period;
  t1c_Start();

  t1c_nextFire = currentTime + cycles/t1c_CyclesPerUs;
}


void t1c_SetNextFireMax(TIME_US timeMicros){
  if(t1c_isPeriodic==TRUE){
    t1c_SetNextFire(timeMicros);
  }else{
    if(t1c_nextFire>timeMicros){
      t1c_SetNextFire(timeMicros);
    }
  }
}


