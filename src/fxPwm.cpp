/*  -----------------------------------------------------------
 *  Andrei Alves Cardoso 16-07-2018
 *  -----------------------------------------------------------
 *  fxPwm.cpp
 *  Arquivo contendo implementação das funções da biblioteca
 *  fxPwm.
 *  -----------------------------------------------------------
 *  Você pode usar livremente esse programa para quaisquer fins,
 *  porém NÃO HÁ GARANTIA para qualquer propósito.
 *  -----------------------------------------------------------
 *  17-07-2018: primeira documentação
 *  21-07-2018: modificações maiores na estrutura da biblioteca. Melhoria de desempenho.
 */

#include <fxPwmTypes.h>
#include <fxPwm.h>
#include <fxPwm_Port.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include "arduino.h"


#ifndef fxPwm_SaveSREG
#define fxPwm_SaveSREG() BYTE sreg_saved = SREG
#endif

#ifndef fxPwm_RestoreSREG
#define fxPwm_RestoreSREG() SREG = sreg_saved
#endif

#define fxPwm_CLEAR_BITS(value,mask)    (value&(~mask)) 
#define fxPwm_SET_BITS(value,mask)      (value|mask)
#define fxPwm_INVERT_BITS(value, mask)  (value^mask)

//Instância.
fxPwm_T1 fxPwm;

UINT32  fxPwm_T1::nsPerTimerClock   = 1; //Cuidado com divisão por zero
UINT16  fxPwm_T1::minTimerGap       = 0;
UINT16  fxPwm_T1::minTimerDelta     = 0;
UINT16  fxPwm_T1::maxTimerDuration  = 0;
UINT16  fxPwm_T1::maxTimerPeriod    = 0;
UINT8   fxPwm_T1::prescaler         = 0;

//===============================================================
//Métodos internos.
//===============================================================

//Interrupt
ISR(TIMER1_COMPB_vect , ISR_NOBLOCK){
  fxPwm.Tick();
}

//Limpa todos itens e pré-calcula alguns valores específicos.
void fxPwm_T1::Cleanup(){
  this->maxPorts = 0;
  this->ports = NULL;
  this->allocatedPins = NULL;

  this->lastClock = 0;
  this->clockCount = 0;
  
  //Escolher pré-escalar de acordo com a frequência de clock,
  //de modo que o período do timer seja o menor valor possível maior que 1 us.
  if(((UINT32)1000000000)/(F_CPU)>=fxPwm_MinTimerResolution){
    //Precisa usar a resolução máxima.
    prescaler = 0b001;
    nsPerTimerClock = 1000000000/F_CPU;
  }else if(((UINT32)1000000000)/(F_CPU>>3)>=fxPwm_MinTimerResolution){
    //Escolher pré-escalar anterior.
    //Pré-escalar: clk/1
    prescaler = 0b001;
    nsPerTimerClock = 1000000000/F_CPU;
  }else if(((UINT32)1000000000)/(F_CPU>>6)>=fxPwm_MinTimerResolution){
    //Escolher pré-escalar anterior.
    //Pré-escalar: clk/8
    prescaler = 0b010;
    nsPerTimerClock = 1000000000/(F_CPU>>3);
  }else if(((UINT32)1000000000)/(F_CPU>>8)>=fxPwm_MinTimerResolution){
    //Escolher pré-escalar anterior.
    //Pré-escalar: clk/64
    prescaler = 0b011;
    nsPerTimerClock = 1000000000/(F_CPU>>6);
  }else if(((UINT32)1000000000)/(F_CPU>>10)>=fxPwm_MinTimerResolution){
    //Escolher pré-escalar anterior.
    //Pré-escalar: clk/256
    prescaler = 0b100;
    nsPerTimerClock = 1000000000/(F_CPU>>8);
  }else{
    //Escolher pré-escalar máximo.
    //Pré-escalar: clk/1024
    prescaler = 0b101;
    nsPerTimerClock = 1000000000/(F_CPU>>10);
  
  }

  //Calcular parâmetros.
  //As constantes estão em microssegundos.
  //Converter para ciclos de clock do timer.
  
  TIME_CLOCK temp = (TIME_CLOCK)(((UINT64)fxPwm_MinTimerGap*1000)/nsPerTimerClock);
  minTimerGap = (UINT16)((temp>fxPwm_MaxTimerClkSum)?(fxPwm_MaxTimerClkSum):(temp));

  temp = (TIME_CLOCK)((UINT64)fxPwm_MaxTimerDuration*1000)/nsPerTimerClock;
  maxTimerDuration = (UINT16)((temp>fxPwm_MaxTimerClkSum)?(fxPwm_MaxTimerClkSum):(temp));

  temp = (TIME_CLOCK)((UINT64)fxPwm_MaxTimerPeriod*1000)/nsPerTimerClock;
  maxTimerPeriod = (UINT16)((temp>fxPwm_MaxTimerClkSum)?(fxPwm_MaxTimerClkSum):(temp));

  temp = (TIME_CLOCK)((UINT64)fxPwm_MinTimerDelta*1000)/nsPerTimerClock;
  minTimerDelta = (UINT16)((temp>fxPwm_MaxTimerClkSum)?(fxPwm_MaxTimerClkSum):(temp));

  return;
}

//Configura o timer escrevendo em seus registros.
//Também calcula pré-escalar adequado e resolução.
void fxPwm_T1::ConfigureTimer(){
  fxPwm_SaveSREG();cli();

  //Escrever todos registros.

  //TCCR1A:
  //Não mexer com OC1A e OC1B:  0b0000xxxx
  //Modo normal:                0bxxxxxx00
  TCCR1A = fxPwm_CLEAR_BITS(TCCR1A, 0b11110011);

  //TCCR1B:
  //Sem cancelamento de ruído:  0b0xxxxxxx
  //Sem detecção de borda:      0bx0xxxxxx
  //Modo normal:                0bxxx00xxx
  //Clock desligado:            0bxxxxx000
  TCCR1B = fxPwm_CLEAR_BITS(TCCR1B, 0b11011111);

  //TCCR1C:
  TCCR1B = fxPwm_CLEAR_BITS(TCCR1B, 0b11000000);

  //TCNT1: contagem atual
  TCNT1 = 1;

  //ICR1: não usado.
  ICR1 = 0;

  //OCR1A: comparado constantemente com TCNT1 para disparar interrupção.
  OCR1A = 0;

  //OCR1B: o mesmo que OCR1A. Não usado.
  OCR1B = 0;

  //TIMSK1: 
  //Habilitar OCIEA:  0bxxxxxx1x (interrupção quando OCR1A=TCNT1)
  //Limpar ICIE, OCIEB E TOIE: 0bxx0xx0x0
  TIMSK1 = fxPwm_CLEAR_BITS(TIMSK1, 0b00100111);
  TIMSK1 = fxPwm_SET_BITS(TIMSK1, 0b00000100);

  //TIFR1: flags de interrupção. Limpar todas: 0bxx0xx000
  TIFR1 = fxPwm_CLEAR_BITS(TIMSK1, 0b00100111);
  
  //Timer configurado.
  fxPwm_RestoreSREG();
  
  return;
}

//Inicia o temporizador escrevendo os bits do pré-escalar no TCCR1B.
void fxPwm_T1::StartTimer(){
  fxPwm_SaveSREG();cli();
  
  //Escrever em TCCR1B os bits do pré-escalar calculado.
  TCCR1B = fxPwm_CLEAR_BITS(TCCR1B, 0b00000111);
  TCCR1B = fxPwm_SET_BITS(TCCR1B, fxPwm_CLEAR_BITS(prescaler, 0b11111000));

  fxPwm_RestoreSREG();

  return;
}

//Para o temporizador limpando os bits do pré-escalar no TCCR1B.
void fxPwm_T1::StopTimer(){
  fxPwm_SaveSREG();cli();
  
  //Escrever em TCCR1B bits 0 no pré-escalar.
  TCCR1B = fxPwm_CLEAR_BITS(TCCR1B, 0b00000111);

  fxPwm_RestoreSREG();

  return;
}

//Agenda a próxima chamada do callback do temporizaodr.
//Se clockCount estiver mais longe que a próxima chamada, a próxima chamada continua agendada.
//Se clockCount vier primeiro, clockCount é agendado.
void fxPwm_T1::SetNextFireMin(TIME_CLOCK clockCount){
  fxPwm_SaveSREG();cli();
  if(clockCount<=this->clockCount){
    //Muito em cima da hora.
    OCR1B = TCNT1 + 1;
  }else{
    //Calcular diferença atual e a próxima e decidir pela menor.
    TIME_CLOCK currentDif = (OCR1B==TCNT1)?(65536):((TIME_CLOCK)(OCR1B - TCNT1));
    TIME_CLOCK newDif = clockCount - this->clockCount;
    if(newDif<currentDif){
      OCR1B = TCNT1 + (UINT16)((newDif==0)?(1):(newDif));
    }
  }
  fxPwm_RestoreSREG();

  return;
}

//Limpar na instância.
fxPwm_T1::fxPwm_T1(){
  Cleanup();

  return;
}

BOOL fxPwm_T1::IsAllocated(){
  return (this->maxPorts>0 && this->ports!=NULL && this->allocatedPins)?(TRUE):(FALSE);
}

//===============================================================
//Um método muito importante.
//===============================================================

//Realiza o processamento da modulação PWM.
//Essa função precisa executar tão rápida quanto possível.
void fxPwm_T1::Tick(){
  //Adquirir rapidamente condições inicais.
  UINT16 lastTCNT1 = TCNT1;
  this->clockCount += (TIME_CLOCK)(lastTCNT1 - lastClock);
  this->lastClock = lastTCNT1;

  //Deadline de execução dessa função. Para evitar que se perca eternamente aqui.
  TIME_CLOCK deadline = this->clockCount + maxTimerDuration;
  //Próximo evento previsto.
  TIME_CLOCK next;

  //Alguns ponteiros.
  fxPwm_Port* currentPort;
  fxPwm_Port** portIndex;
  
  do{
    //Adquirir novos valores de tempo.
    lastTCNT1 = TCNT1;
    this->clockCount += (TIME_CLOCK)(lastTCNT1 - lastClock);
    this->lastClock = lastTCNT1;
    next=this->clockCount+maxTimerPeriod;
    //Restaura ponteiro para início da lista de portas.
    portIndex = ports;

    //Percorre a lista de portas e processa eventos agendados em cada uma.
    //Para quando encontrar um elemento NULL na lista.
    while((currentPort = *portIndex++)!=NULL){
      if(currentPort->enabled==FALSE){
        //Pula elemento se não estiver habilitado.
        continue;
      }
      //Verifica se está na hora do próximo evento.
      if(this->clockCount>=currentPort->next){
        //Verifica se está em nível ALTO (para trocar para BAIXO), e se o período BAIXO é >0.
        if(currentPort->outHint && currentPort->lowPeriod>0){
          //Está em nível ALTO. Trocar para nível BAIXO.
          *currentPort->port &= ~currentPort->mask;
          //Calcular próxima chamada.
          currentPort->next+=currentPort->lowPeriod;
          currentPort->outHint = 0x00;
        }else if(currentPort->highPeriod>0){
          //Está em nível BAIXO. Trocar para ALTO, se o período ALTO é >0.
          *currentPort->port |= currentPort->mask;
          currentPort->next+=currentPort->highPeriod;
          currentPort->outHint = 0xFF;
        }
        //A motivação dessa estrutura é permitir ciclos de trabalhos 0% verdadeiro e 100% verdadeiro.
      }
      //Obtém próximo evento.
      next = (currentPort->next<next)?(currentPort->next):(next);
    }

    //Sai do laço em duas condições:
    //Se a fenda até o próximo evento por grande o suficiente OU
    //Se der o deadline.
  }while(clockCount>next-minTimerGap && clockCount<deadline);

  //Agendar próxima chamada.

  //Calcular tempo pela última vez.
  lastTCNT1 = TCNT1;
  this->clockCount += (TIME_CLOCK)(lastTCNT1 - lastClock);
  this->lastClock = lastTCNT1;

  //Garantir que a próxima chamada ocorra não antes que minTimerDelta do tempo atual.
  //Se isso não for feito, coisas estranhas acontecem... (estouro de pilha?)
  if(next<this->clockCount+minTimerDelta){
    OCR1B = lastTCNT1 + minTimerDelta;
  }else{
    OCR1B = lastTCNT1 + (next - this->clockCount);
  }

  return;
}

//===============================================================
//Métodos de inicialização e liberação.
//===============================================================

//Inicializa usando a definição de fxPwm_MaxPorts.
void fxPwm_T1::Initialize(){
  this->Initialize(fxPwm_MaxPorts);

  return;
}

//Inicializa a classe definindo um número máximo de portas alocáveis.
void fxPwm_T1::Initialize(UINT8 maxPorts){
  fxPwm_SaveSREG();cli();

  //Verificar se existe algo de antes.
  //Se tiver, limpar.
  if(this->IsAllocated()!=FALSE){
    this->Free();
  }else{
    //Realizar limpeza preventiva de qualquer jeito.
    Cleanup();
  }

  //Tentar alocar lista de portas.
  ports = (fxPwm_Port**)malloc(sizeof(fxPwm_Port*)*(maxPorts+1));
  if(ports==NULL){
    //Falhou.
    fxPwm_RestoreSREG();
    return;
  }

  //Tentar alocar lista de pinos.
  allocatedPins = (UINT8*)malloc(sizeof(UINT8)*(maxPorts));
  if(allocatedPins==NULL){
    free(ports);
    fxPwm_RestoreSREG();
    return;
  }

  //Salvar máximo de portas.
  this->maxPorts = maxPorts;

  //Passar limpando tudo.
  UINT8 t;
  for(t=0;t<this->maxPorts+1;t++){
    ports[t] = NULL;
    if(t<this->maxPorts){
      //Evitar que o último elemento seja limpo.
      allocatedPins[t] = 0xFF;
    }
  }

  //Configurar o timer.
  this->ConfigureTimer();

  //Restaurar SREG e terminar.
  fxPwm_RestoreSREG();
  
  return;
}

//Libera todos recursos.
void fxPwm_T1::Free(){
  if(this->IsAllocated()==FALSE){
    //Não há portas alocadas. Apenas fazer uma limpeza.
    Cleanup();
    return;
  }

  fxPwm_SaveSREG();cli();

  //Liberar todas portas.
  UINT8 t;
  for(t=0;t<this->maxPorts==0;t++){
    this->RemovePort(this->ports[0]);
  }

  //Liberar memórias.
  free(ports);
  free(allocatedPins);

  //Limpeza.
  Cleanup();

  fxPwm_RestoreSREG();
  return;
}

//Inicia o TIMER1 e recalcula a fase de todos osciladores.
void fxPwm_T1::Start(){
  //Verifica realidade.
  if(this->IsAllocated()==FALSE){
    return;
  }
  
  fxPwm_SaveSREG();cli();
  
  this->StartTimer();
  
  UINT8 t;
  for(t=0;t<this->maxPorts;t++){
    ports[t]->ResetPhase();
  }

  fxPwm_RestoreSREG();

  return;
}

//Para o TIMER1.
void fxPwm_T1::Stop(){
  this->StopTimer();
}

//===============================================================
//Funções de manipulação de portas e pinos.
//===============================================================

//Registra uma porta, colocando seu ponteiro no fim de uma lista estática.
void fxPwm_T1::RegisterPort(fxPwm_Port *port){
  //Verifica realidade.
  if(this->IsAllocated()==FALSE || port==NULL){
    return;
  }

  UINT8 t;

  //Verificar se o pino já está registrado. Recusar se estiver.
  for(t=0;t<this->maxPorts;t++){
    if(this->ports[t]->pinNumber == port->pinNumber){
      //Não aceitar alocar duas portas com mesmo valor de pino.
      return;
    }
    if(this->ports[t]==NULL){
      //Acabou a lista.
      break;
    }
  }

  //Verificar se chegou ao fim da lista. Se tiver chegado, recusar adição.
  if(t!=this->maxPorts){
    fxPwm_SaveSREG();cli();
    this->ports[t] = port;
    //Não precisa registrar o pino em allocatedPins, já que não há garantia que esse ponteiro foi alocado internamente.
    fxPwm_RestoreSREG();
  }

  return;
}


//Registra uma porta a partir de um número de pino.
//Para isso, ela aloca um objeto fxPwm_Port.
void fxPwm_T1::RegisterPort(UINT8 pin){
  //Verifica realidade.
  if(this->IsAllocated()==FALSE || pin==0xFF){
    return;
  }
  
  //Buscar se porta já foi registrada e recusar se tiver sido.
  UINT8 t;
  for(t=0;t<this->maxPorts;t++){
    if(this->ports[t]==NULL){
      break;
    }
    if(this->ports[t]->pinNumber==pin){
      //Já foi registado.
      return;
    }
  }

  //Verificar se porta é válida.
  if(t==this->maxPorts || digitalPinToPort(pin)==NOT_A_PIN){
    return;
  }

  //Tentar alocar memória para a porta.
  fxPwm_Port *newPort = (fxPwm_Port*)malloc(sizeof(fxPwm_Port));
  if(newPort==NULL){
    //Falha ao alocar.
    return;
  }

  //Realizar limpeza da memória da porta e atribuir pino.
  newPort->Cleanup();
  newPort->SetPinNumber(pin);

  //Registrar porta.
  this->RegisterPort(newPort);

  //Colocar número do pino na lista de alocados internamente.
  for(t=0;t<this->maxPorts;t++){
    if(this->allocatedPins[t]==0xFF){
      this->allocatedPins[t] = pin;
      break;
    }
  }

  return;
}

//Dado o ponteiro para uma porta, tenta removê-la.
//A remoção e feita procurando o ponteiro na lista de portas.
//Se encontrar, desloca todos elementos da frente para trás, e coloca NULL no último.
//Se o ponteiro representar um item alocado internamente, este é desalocado.
void fxPwm_T1::RemovePort(fxPwm_Port *port){
  //Verificação de realidade.
  if(this->IsAllocated()==FALSE || port==NULL){
    return;
  }

  fxPwm_SaveSREG();cli();

  //Percorrer lista estática.
  UINT8 t,u, portN;
  for(t=0;t<this->maxPorts;t++){
    if(this->ports[t]==NULL){
      //Chegou no último. Terminar.
      break;
    }
    if(this->ports[t]==port){
      //Encontrou item certo. Deslocar todos depois dele para trás.
      for(u=t;u<this->maxPorts-1;u++){
        this->ports[u] = this->ports[u+1];
      }
      //Colocar NULL no último e sair.
      this->ports[this->maxPorts-1] = NULL;
      break;
    }
  }

  //Pesquisar se o número do pino está na lista de itens alocados internamente.
  for(t=0;t<this->maxPorts;t++){
    if(this->allocatedPins[t]==port->pinNumber){
      //Porta foi alocada internamente. Desalocar e marcar.
      free(port);
      this->allocatedPins[t] = 0xFF;
      break;
    }
  }
  
  fxPwm_RestoreSREG();
  return;
}

//Remove a porta a partir de um número de pino.
void fxPwm_T1::RemovePort(UINT8 pin){
  this->RemovePort(this->GetPort(pin));
 
  return;
}


//Atribui período a um dos pinos.
void fxPwm_T1::SetPeriod(UINT8 pin, TIME_US period){
  fxPwm_Port *port = this->GetPort(pin);

  if(port!=NULL){
    port->SetPeriod(period);
  }
  
  return;
}

//Atribui frequência a um dos pinos.
void fxPwm_T1::SetFrequency(UINT8 pin, FLOAT frequency){
  fxPwm_Port *port = this->GetPort(pin);

  if(port!=NULL){
    port->SetFrequency(frequency);
  }

  return;
}

//Atribui ciclo de trabalho a um dos pinos.
void fxPwm_T1::SetDuty(UINT8 pin, FLOAT duty){
  fxPwm_Port *port = this->GetPort(pin);

  if(port!=NULL){
    port->SetDuty(duty);
  }

  return;
}

//Atribui o mapeamento na porta.
void fxPwm_T1::SetMap(UINT8 pin, FLOAT dutyValue1, FLOAT mappedValue1, FLOAT dutyValue2, FLOAT mappedValue2){
  fxPwm_Port *port = this->GetPort(pin);

  if(port!=NULL){
    port->SetMap(dutyValue1, mappedValue1, dutyValue2, mappedValue2);
  }

  return;
}

//Habilita modulação em um pino.
void fxPwm_T1::EnablePin(UINT8 pin){
  fxPwm_Port *port = this->GetPort(pin);

  if(port!=NULL){
    port->Enable();
  }

  return;
}

//Desabilita modulação em um pino.
void fxPwm_T1::DisablePin(UINT8 pin){
  fxPwm_Port *port = this->GetPort(pin);

  if(port!=NULL){
    port->Disable();
  }

  return;
}

//Habilita todos os pinos alocados.
void fxPwm_T1::EnableAll(){
  UINT8 t;
  for(t=0;t<this->GetNumRegisteredPorts();t++){
    this->EnablePin(this->ports[t]->pinNumber);
  }

  return;
}

void fxPwm_T1::DisableAll(){
  UINT8 t;
  for(t=0;t<this->GetNumRegisteredPorts();t++){
    this->DisablePin(this->ports[t]->pinNumber);
  }

  return;
}


//===============================================================
//Métodos de aquisição de informação.
//===============================================================

//Pesquisa pela porta com certo número de pino e retorna o ponteiro.
//USE COM CUIDADO!!!
fxPwm_Port *fxPwm_T1::GetPort(UINT8 pin){
  if(this->ports==NULL || this->maxPorts==0 || pin==0xFF){
    return NULL;
  }

  UINT8 t;
  for(t=0;t<this->maxPorts;t++){
    if(this->ports[t]==NULL){
      break;
    }
    if(this->ports[t]->pinNumber == pin){
      return this->ports[t];
    }
  }

  return NULL;
}

//Retorna a quantidade máxima de portas alocáveis.
UINT8 fxPwm_T1::GetMaxPorts(){
  return this->maxPorts;
}

//Conta a quantidade de portas registradas.
UINT8 fxPwm_T1::GetNumRegisteredPorts(){
  if(this->IsAllocated()==FALSE || this->maxPorts==0xFF){
    return 0;
  }

  UINT8 t;
  for(t=0;t<this->maxPorts;t++){
    if(this->ports[t]==NULL){
      return t;
    }
  }

  return t;
}

//Retorna um ponteiro para uma porta registrada a partir de um índice, ou NULL se não existir.
fxPwm_Port *fxPwm_T1::GetRegisteredPort(UINT8 index){
  if(this->IsAllocated()==FALSE || index>=this->maxPorts){
    return NULL;
  }
  return this->ports[index];
}

//Retorna o número do pino de uma porta registrada a partir de um índice, ou 0xFF se não existir.
UINT8 fxPwm_T1::GetRegisteredPortPinNumber(UINT8 index){
  fxPwm_Port *port = GetRegisteredPort(index);
  if(port!=NULL){
    return port->pinNumber;
  }
  return 0xFF;
}

//Retorna o índice de um pino, ou 0xFF se não existir.
UINT8 fxPwm_T1::GetIndex(UINT8 pin){
  if(this->IsAllocated() || pin == 0xFF){
    return 0xFF;
  }

  UINT8 t;
  for(t=0;t<this->maxPorts;t++){
    if(this->ports[t]==NULL){
      break;
    }
    if(this->ports[t]->pinNumber == pin){
      return t;
    }
  }

  return 0xFF;
}

TIME_US fxPwm_T1::GetPeriod(UINT8 pin){
  fxPwm_Port *port = GetPort(pin);
  if(port!=NULL){
    return port->GetPeriod();
  }
  return 0;
}

FLOAT fxPwm_T1::GetFrequency(UINT8 pin){
  fxPwm_Port *port = GetPort(pin);
  if(port!=NULL){
    return port->GetFrequency();
  }
  return 0.0;
}

FLOAT fxPwm_T1::GetDuty(UINT8 pin){
  fxPwm_Port *port = GetPort(pin);
  if(port!=NULL){
    return port->GetDuty();
  }
  return 0.0;
}

TIME_US fxPwm_T1::GetNextEvent(UINT8 pin){
  fxPwm_Port *port = GetPort(pin);
  if(port!=NULL){
    return (TIME_US)(((UINT64)port->next*(UINT64)nsPerTimerClock)/(UINT64)1000);
  }
  return 0;
}

  
TIME_US fxPwm_T1::Micros(){
  return (TIME_US)(((UINT64)this->clockCount*(UINT64)nsPerTimerClock)/(UINT64)1000);
}


