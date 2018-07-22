/*  -----------------------------------------------------------
 *  Andrei Alves Cardoso 21-07-2018
 *  -----------------------------------------------------------
 *  fxPwm_Port.cpp
 *  Cabeçalho contendo definição dos métodos da classe fxPwm_Port.
 *  -----------------------------------------------------------
 *  Você pode usar livremente esse programa para quaisquer fins,
 *  porém NÃO HÁ GARANTIA para qualquer propósito.
 *  -----------------------------------------------------------
 *  21-07-2018: primeira documentação.
 */

#include <fxPwmTypes.h>
#include <fxPwm.h>
#include <fxPwm_Port.h>

#ifndef fxPwm_SaveSREG
#define fxPwm_SaveSREG() BYTE sreg_saved = SREG
#endif

#ifndef fxPwm_RestoreSREG
#define fxPwm_RestoreSREG() SREG = sreg_saved
#endif

//Limpa todos itens da classe, e atribui valores padrões onde precisar.
void fxPwm_Port::Cleanup(){
  fxPwm_SaveSREG();cli();
  
  this->enabled = FALSE;
  this->pinNumber = 0xFF;

  this->dutyMapMulti = 1.0;
  this->dutyMapDelta = 0.0;

  this->port = NULL;
  this->ddr = NULL;
  this->mask = 0x00;
  this->outHint = 0x00;

  this->period = 0;
  this->duty = 0.5;

  this->next = fxPwm_NO_NEXT_EVENT;
  this->highPeriod = 0;
  this->lowPeriod = 0;

  fxPwm_RestoreSREG();
  
  return;
}

//Agenda um evento para esse objeto, se estiver habilitado.
void fxPwm_Port::ResetPhase(){
  if(this->enabled==TRUE && this->period!=0){
    //Verificar se vale a pena agendar.
    TIME_CLOCK minNext = fxPwm.clockCount + this->highPeriod + this->lowPeriod;
    this->next = (this->next>minNext)?(minNext):(this->next);
    fxPwm.SetNextFireMin(this->next);
  }

  return;
}

//Atribui um número de pino À classe.
//Para isso ele consulta se o pino é válido.
//Se for, busca os ponteiros associados aos registradores do pino.
void fxPwm_Port::SetPinNumber(UINT8 pinNumber){
  UINT8 portN = digitalPinToPort(pinNumber);

  fxPwm_SaveSREG();cli();
  if(portN==NOT_A_PIN){
    //Pino inválido.
    this->port = NULL;
    this->ddr = NULL;
    this->mask = 0x00;
    this->pinNumber = 0xFF;
    this->next = fxPwm_NO_NEXT_EVENT;
    fxPwm_RestoreSREG();
    return;
  }

  //Adquirir ponteiros.
  this->port = portOutputRegister(portN);
  this->ddr = portModeRegister(portN);
  this->mask = digitalPinToBitMask(pinNumber);
  this->pinNumber = pinNumber;

  fxPwm_RestoreSREG();
  
  return;
}

//Construtor padrão. Somente limpa.
fxPwm_Port::fxPwm_Port(){
  this->Cleanup();

  return;
}

//Construtor. Atribui um número de pino.
fxPwm_Port::fxPwm_Port(UINT8 pinNumber){
  this->Cleanup();
  this->SetPinNumber(pinNumber);

  return;
}

//Retorna o período diretamente, em microssegundos.
TIME_US fxPwm_Port::GetPeriod(){
  return this->period;
}

//Retorna o ciclo de trabalho mapeado.
FLOAT fxPwm_Port::GetDuty(){
  return (this->GetRawDuty() - this->dutyMapDelta)/this->dutyMapMulti;
}

//Retorna o ciclo de trabalho diretamente.
FLOAT fxPwm_Port::GetRawDuty(){
  return this->duty;
}

//Retorna a frequência a partir do inverso do período.
//Mas antes, verifica se o período não é 0 (inválido).
FLOAT fxPwm_Port::GetFrequency(){
  TIME_US period = this->GetPeriod();
  return (period==0)?(0.0):(1000000.0/(FLOAT)period);
}

//Retorna o estado lógico do pino (HIGH ou LOW).
BOOL fxPwm_Port::GetPinState(){
  fxPwm_SaveSREG();cli();
  BYTE pinState;
  if(this->pinNumber==0xFF){
    //Não há pino registrado.
    fxPwm_RestoreSREG();
    return LOW;
  }
  //Obtém estado do pino.
  pinState = *this->port & this->mask;
  fxPwm_RestoreSREG();
  //Transforma em HIGH ou LOW usando comparação ternária.
  return (pinState)?(HIGH):(LOW);
}

//Coloca a porta em modo saída e escreve valor.
void fxPwm_Port::SetPinState(BOOL state){
  if(this->pinNumber==0xFF){
    //Não há pino registrado.
    return;
  }

  //Coloca porta em modo de saída.
  *this->ddr |= this->mask;
  if(state==LOW){
    //Limpa bits.
    *this->port &= ~this->mask;
  }else{
    //Seta bits.
    *this->port |= this->mask;
  }
}

//Atribui o período e o ciclo de trabalho.
//Todas as outras funções que atribuem ciclo de trabalho, frequência e duty chamam essa.
//Escrever período 0 faz a modulação parar. O valor que fica na porta é BAIXO se duty<=0.5, e ALTO se duty>0.5.
void fxPwm_Port::SetPeriodAndDuty(TIME_US period, FLOAT duty){
  //Obtém valor mapeado do duty.
  duty = duty * this->dutyMapMulti + this->dutyMapDelta;
  
  //Acerta o duty.
  duty = (duty<0.0)?(0.0):((duty>1.0)?(1.0):(duty));

  //Verifica se o período é válido.
  if(period==0){
    fxPwm_SaveSREG();cli();
    //Sem período. Zerar variáveis de período mas impedir agendamento.
    this->period = 0;
    this->duty = duty;
    this->next = fxPwm_NO_NEXT_EVENT;
    this->highPeriod = 0;
    this->lowPeriod = 0;

    //Atribui valor na porta de acordo com o duty.
    if(this->port!=NULL && this->enabled!=FALSE){
      if(duty>0.5){
        *this->port |= this->mask;
      }else{
        *this->port &= ~this->mask;
      }
    }
    
    fxPwm_RestoreSREG();
    return;
  }
  
  //Calcular períodos.
  
  //Soma 500 para melhorar arredondamento.
  UINT32 periodClk = (UINT32)((UINT64)period * 1000 + 500)/((UINT64)fxPwm_T1::nsPerTimerClock);

  //Período em nível ALTO e BAIXO.
  UINT32 highPeriod = (UINT32)((FLOAT)periodClk * duty);
  UINT32 lowPeriod = periodClk - highPeriod;

  //Calcula previsão do próximo evento.
  TIME_CLOCK minNext = periodClk + fxPwm.clockCount;

  //Atribui todos valores.
  fxPwm_SaveSREG();cli();
  
  this->period = period;
  this->duty = duty;
  
  if(this->port!=NULL && this->ddr!=NULL && this->enabled!=FALSE){
    //Somente agendar próximo evento se estiver tudo certo.
    this->next = (this->next>minNext)?(minNext):(this->next);
    fxPwm.SetNextFireMin(this->next);
  }else{
    this->next = fxPwm_NO_NEXT_EVENT;
  }
  
  this->highPeriod = highPeriod;
  this->lowPeriod = lowPeriod;

  fxPwm_RestoreSREG();

  return;
}

void fxPwm_Port::SetPeriod(TIME_US period){
  this->SetPeriodAndDuty(period, this->GetDuty());
}

void fxPwm_Port::SetDuty(FLOAT duty){
  this->SetPeriodAndDuty(this->GetPeriod(), duty);
}

//Atribui frequência. Se frequency<=0.0, atribui período 0.
void fxPwm_Port::SetFrequencyAndDuty(FLOAT frequency, FLOAT duty){
  this->SetPeriodAndDuty((frequency<=0.0)?(0):((TIME_US)(1000000.0/(FLOAT)frequency+0.5)), duty);
}

void fxPwm_Port::SetFrequency(FLOAT frequency){
  this->SetFrequencyAndDuty(frequency, this->GetDuty());
}

//Calcula os parâmetros de mapeamento do ciclo de trabalho.
void fxPwm_Port::SetMap(FLOAT dutyValue1, FLOAT mappedValue1, FLOAT dutyValue2, FLOAT mappedValue2){
  if(mappedValue1 == mappedValue2){
    //Não há diferença. Não mexer.
    return;
  }
  this->dutyMapMulti = (dutyValue2 - dutyValue1) / (mappedValue2 - mappedValue1);
  this->dutyMapDelta = dutyValue1 - mappedValue1*dutyMapMulti;

  return;
}

//Habilita a modulação PWM na porta.
//Coloca a porta em estado de saída e em nível BAIXO.
void fxPwm_Port::Enable(){
  //Não habilitar se já estiver habilitado OU se algo estiver errado.
  if(this->enabled!=FALSE || this->pinNumber==0xFF){
    return;
  }
  fxPwm_SaveSREG();cli();

  //Configurar modo de saída e colocar em nível BAIXO.
  *this->ddr |= this->mask;
  *this->port &= ~this->mask;
  this->outHint = 0x00;

  //Se não tiver período, não agendar evento.
  if(this->period==0){
    this->next = fxPwm_NO_NEXT_EVENT;
  }else{
    this->next = fxPwm.clockCount;
    fxPwm.SetNextFireMin(this->next);
  }
  this->enabled = TRUE;
  fxPwm_RestoreSREG();
}

//Desabilita a porta, inibindo modulação.
//Coloca a saída em nível BAIXO.
void fxPwm_Port::Disable(){
  fxPwm_SaveSREG();cli();
  if(this->port!=NULL){
    //Escrever nível BAIXO na saída.
    *this->port &= ~this->mask;
    this->outHint = 0x00;
  }
  this->enabled = FALSE;
  this->next = fxPwm_NO_NEXT_EVENT;
  fxPwm_RestoreSREG();
}


