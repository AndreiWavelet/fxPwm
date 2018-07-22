/*  -----------------------------------------------------------
 *  Andrei Alves Cardoso 21-07-2018
 *  -----------------------------------------------------------
 *  fxPwm_Port.h
 *  Cabeçalho contendo declaração da classe fxPwm_Port, que en-
 *  capsula as configurações de uma porta onde será modulado PWM.
 *  -----------------------------------------------------------
 *  Você pode usar livremente esse programa para quaisquer fins,
 *  porém NÃO HÁ GARANTIA para qualquer propósito.
 *  -----------------------------------------------------------
 *  21-07-2018: primeira documentação.
 */

#ifndef fxPwm_Port_H
#define fxPwm_Port_H

#include <fxPwmTypes.h>

//Marcação de que não há um próximo evento no canal atual.
#define fxPwm_NO_NEXT_EVENT TIME_US_MAX

//Classe que encapsula dados de uma porta,
//e métodos para manipulação dela.
class fxPwm_Port{
private:
  //Indica se o canal está habilitado para modulação.
  volatile BOOL enabled;
  //Guarda número do pino associado, ou 0xFF se nada tiver associado.
  UINT8 pinNumber;

  //Valores de referência para mapeamento do ciclo de trabalho.
  //O valor mapeado do ciclo de trabalho é dado pela equação:
  //dutyMap = dutyMapMulti*duty + dutyMapDelta.
  FLOAT dutyMapMulti;
  FLOAT dutyMapDelta;

  //Ponteiro para o registrador da porta.
  volatile BYTE *port;
  //Ponteiro para o registrador de direção da forta.
  volatile BYTE *ddr;
  //Valor da máscara correspondente.
  volatile BYTE mask;
  //Dica para o valor da saída.
  volatile BYTE outHint;

  //Período da classe, em microssegundos.
  TIME_US period;
  //Ciclo de trabalho.
  FLOAT duty;

  //Próximo evento agendado.
  volatile TIME_US next;

  //Período ALTO, em ciclos do timer.
  volatile TIME_US highPeriod;
  //Período BAIXO, em ciclos do timer.
  volatile TIME_US lowPeriod;

  //Realiza limpeza.
  void Cleanup();
  //Recalcula parâmetros de fase da classe, e agenda próximo evento.
  void ResetPhase();
public:
  friend class fxPwm_T1;

  //Atribui um pino.
  void SetPinNumber(UINT8 pinNumber);

  //Inicializa a porta com ou sem um pino.
  fxPwm_Port();
  fxPwm_Port(UINT8 pinNumber);

  //Retorna o período configurado.
  TIME_US GetPeriod();
  //Retorna o ciclo de trabalho configurado, com mapeamento. Inicialmente 0.5.
  FLOAT GetDuty();
  //Retorna o ciclo de trabalho sem mapeamento.
  FLOAT GetRawDuty();
  //Retorna a frequência configurada.
  FLOAT GetFrequency();

  //Retorna o estado lógico do pino (LOW, HIGH)
  BOOL GetPinState();

  //Atribui o estado lógico do pino. (LOW, HIGH)
  void SetPinState(BOOL state);

  //Atribui o valor do período e o ciclo de trabalho.
  void SetPeriodAndDuty(TIME_US period, FLOAT duty);

  //Atribui várias variáveis.
  void SetPeriod(TIME_US period);
  void SetDuty(FLOAT duty);
  void SetFrequencyAndDuty(FLOAT frequency, FLOAT duty);
  void SetFrequency(FLOAT frequency);

  //Mapeia o ciclo de trabalho.
  void SetMap(FLOAT dutyValue1, FLOAT mappedValue1, FLOAT dutyValue2, FLOAT mappedValue2);

  //Habilita a modulação PWM.
  void Enable();
  //Desabilita a modulação PWM.
  void Disable();
};

#endif

