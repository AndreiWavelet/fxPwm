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
 */

#include "arduino.h"
#include <stdlib.h>
#include <fxPwmTypes.h>

#include <fxPwm.h>
#include <t1custom.h>


//Macros para salvar o contexto do SREG. Muito importante aqui.
#define fxPwm_SAVE_SREG() BYTE sreg = SREG
#define fxPwm_RESTORE_SREG() SREG=sreg

//Indicador de que não há próximo evento (porta desabilitada).
#define fxPwm_NO_NEXT_EVENT TIME_US_MAX

//Estrutura que encapsula dados correntes a respeito de um modulador PWM.
typedef struct{
  //Indica se o canal está ou não habilitado.
  BOOL enabled;
  
  //Ponteiro para o registrador que contém o estado da porta.
  volatile BYTE *port;
  //Ponteiro para o registrador que contém a direção da porta.
  volatile BYTE *DDR;
  //Máscara, para alterar os registradores de direção e estado.
  volatile BYTE mask;

  //Período e ciclo de trabalho atual.
  TIME_US period;
  FLOAT duty;
  
  //Observe que halfPeriod1 + halfPeriod2 = period.
  
  //Primeira metade do período. No primeiro ciclo, representa o tempo em nível ALTO. Fica alternando com halfPeriod2.
  volatile TIME_US halfPeriod1;
  //Segunda metade do período. No primeiro ciclo, representa o tempo em nível BAIXO. Fica alternando com halfPeriod1.
  volatile TIME_US halfPeriod2;
  //Momento do próximo evento de troca de nível em relação ao início do mundo, em microssegundos.
  //Se nextEvent=fxPwm_NO_NEXT_EVENT, significa que o canal está silenciado, mas não necessariamente desabilitado.
  volatile TIME_US nextEvent;
}fxPwm_PortData;

//Portas PWM registradas.
fxPwm_PortData *fxPwm_ports = NULL;
volatile UINT8 fxPwm_numPorts = 0;

//Garantia de haver começado.
volatile BOOL fxPwm_started = FALSE;

//Função micros.
TIME_US (*fxPwm_MicrosCallback)() = t1c_Micros;

/* Callback que efetivamente modula o PWM.
 * 
 * Note que essa função é muito apertada em termos de tempo, e tudo tem que ser feito de forma mais rápida possível.
 * Eu poderia implementar isso em assembly, mas prefiro manter a didaticidade do código.
 * 
 * Para que as coisas sejam rápidas, a função não faz verificações de robustez.
 * Ela assume que tudo está íntegro. Portanto, a integridade deve ser garantida pelas funções que manipulam o que aqui é usado.
 *  
 */
void fxPwm_Callback(){
  TIME_US currentTime = fxPwm_MicrosCallback();
  TIME_US nextEvent, aux;
  fxPwm_PortData *currentPort, *last;

  //Calcular deadline, isto é, até quando essa função pode ficar executando sem penalidade.
  TIME_US deadline = currentTime+fxPwm_MAX_TIMER_CALLBACK_DURATION;
  //Calcular próximo evento no pior caso. Internamente, essa valor vai ser mudado.
  nextEvent = currentTime + fxPwm_MIN_TIMER_PERIOD;

  last = &fxPwm_ports[fxPwm_numPorts+1];

  do{
    currentTime = fxPwm_MicrosCallback();
    currentPort = fxPwm_ports;

    //Laço que vai percorrer as portas.
    do{
      if(currentTime>=currentPort->nextEvent){
        *currentPort->port ^= currentPort->mask;

        //Calcular o próximo evento.
        currentPort->nextEvent += currentPort->halfPeriod1;

        //Permutar os valores de halfPeriod1 e halfPeriod2.
        //É possível usar XOR para fazer isso, se quiser.
        //Prefiro o método convencional, entretanto.
        aux = currentPort->halfPeriod1;
        currentPort->halfPeriod1 = currentPort->halfPeriod2;
        currentPort->halfPeriod2 = aux;
      }
      //Selecionar o evento mais próximo enquanto percorre.
      if(currentPort->nextEvent<nextEvent){
        nextEvent = currentPort->nextEvent;
      }
      currentPort++;
    }while(currentPort<last);
  }while(currentTime>nextEvent-fxPwm_TIMER_CALLBACK_SAFE_OVERHEAD && currentTime<deadline);

  t1c_SetNextFire(nextEvent);
}

// Limpa toda a memória referente a uma porta, e inicializa valores padrões.
void fxPwm_ClearPort(UINT8 port){
  //Garantir robustez.
  if(port>=fxPwm_numPorts || fxPwm_ports==NULL){
    return;
  }

  fxPwm_PortData *current = &fxPwm_ports[port];

  fxPwm_SAVE_SREG();
  cli();

  current->enabled = FALSE;

  current->port = NULL;
  current->DDR = NULL;
  current->mask = 0;

  current->period = (TIME_US)0;
  current->duty = fxPwm_START_DUTY;
  
  current->halfPeriod1 = (TIME_US)0;
  current->halfPeriod2 = (TIME_US)0;

  current->nextEvent = (TIME_US)fxPwm_NO_NEXT_EVENT;

  fxPwm_RESTORE_SREG();
}

void fxPwm_ClearAllPorts(){
  UINT8 t;

  for(t=0;t<UINT8_MAX;t++){
    fxPwm_ClearPort(t);
  }

  return;
}

void fxPwm_Initialize(UINT8 numPorts, TIME_US (*microsFunction)()){
  if(numPorts==0){
    //Equivale à liberação da biblioteca.
    fxPwm_Free();
    return;
  }

  fxPwm_SAVE_SREG();
  cli();

  fxPwm_started = FALSE;
  
  //Liberar memória caso haja algo de antes...
  if(fxPwm_ports!=NULL && fxPwm_numPorts!=0){
    fxPwm_Free();
  }

  fxPwm_ports = (fxPwm_PortData*)malloc(sizeof(fxPwm_PortData)*numPorts);

  //Verificar se está ok e prosseguir.
  if(fxPwm_ports==NULL){
    //Não alocou.
    fxPwm_numPorts = 0;
    fxPwm_MicrosCallback = t1c_Micros;
    return;
  }
  
  fxPwm_numPorts = numPorts;

  if(microsFunction!=NULL){
    fxPwm_MicrosCallback = microsFunction;
  }else{
    fxPwm_MicrosCallback = t1c_Micros;
  }

  //Inicializar temporizador.
  t1c_Configure(fxPwm_Callback, fxPwm_MicrosCallback);

  //Limpa todas memórias e inicializa com valores padrões.
  fxPwm_ClearAllPorts();
  
  fxPwm_RESTORE_SREG();
  
  return;
}

//Simplesmente atribui os valores adequados na estrutura.
void fxPwm_ConfigurePort(UINT8 port, volatile BYTE* portPointer, volatile BYTE* ddrPointer, BYTE mask){
  if(port>=fxPwm_numPorts || fxPwm_ports==NULL || portPointer==NULL || ddrPointer==NULL){
    return;
  }

  fxPwm_PortData *current = &fxPwm_ports[port];

  fxPwm_SAVE_SREG();
  cli();

  current->port = portPointer;
  current->DDR = ddrPointer;
  current->mask = mask;

  fxPwm_DigitalWrite(port, LOW);

  fxPwm_RESTORE_SREG();
}

//Configura a porta a partir de um número de pino.
void fxPwm_ConfigurePort(UINT8 port, UINT8 pinNumber){
  volatile UINT8 *portAddr;
  volatile UINT8 *ddrAddr;
  UINT8 mask, portN;

  //Obtém o número da porta e verifica se é um pino válido.
  portN = digitalPinToPort(pinNumber);
  if(portN==NOT_A_PIN){
    return;
  }

  //Obtém endereços da porta e do ddr.
  portAddr = portOutputRegister(portN);
  ddrAddr = portModeRegister(portN);
  mask = digitalPinToBitMask(pinNumber);

  //Configura normalmente.
  fxPwm_ConfigurePort(port, portAddr, ddrAddr, mask);

  return;
}

void fxPwm_DigitalWrite(UINT8 port, INT16 value){
  if(port>=fxPwm_numPorts || fxPwm_ports==NULL){
    return;
  }
	
  //Garantir modo de saída.
  *fxPwm_ports[port].DDR |= fxPwm_ports[port].mask;
  if(value==LOW){
	*fxPwm_ports[port].port &= ~fxPwm_ports[port].mask;
  }else{
	*fxPwm_ports[port].port |= fxPwm_ports[port].mask;
  }
  return;
}

INT16 fxPwm_DigitalRead(UINT8 port){
  if(port>=fxPwm_numPorts || fxPwm_ports==NULL){
    return LOW;
  }
  
  return (*fxPwm_ports[port].port&fxPwm_ports[port].mask)?(HIGH):(LOW);
}

//Funções que retornam parâmetros.

inline FLOAT fxPwm_GetDuty(UINT8 port){
  return (port>=fxPwm_numPorts || fxPwm_ports==NULL)?(0.0):(fxPwm_ports[port].duty);
}

inline TIME_US fxPwm_GetPeriod(UINT8 port){
  return (port>=fxPwm_numPorts || fxPwm_ports==NULL)?(0):((TIME_US)fxPwm_ports[port].period);
}

inline FLOAT fxPwm_GetFrequency(UINT8 port){
  return (port>=fxPwm_numPorts || fxPwm_ports==NULL)?(0.0):(fxPwm_ports[port].period);
}

//Recalcula os períodos de ciclo alto e baixo, e coloca a saída em nível BAIXO.
//Não agenda de forma nenhuma o próximo evento.
void fxPwm_ResetDuty(UINT8 port){
  if(port>=fxPwm_numPorts || fxPwm_ports == NULL) return;

  fxPwm_PortData *currentPort = &fxPwm_ports[port];

  //Salvar contexto.
  fxPwm_SAVE_SREG();
  cli();
  
  //Calcula os períodos.
  TIME_US highPeriod = (TIME_US)(((FLOAT)fxPwm_GetPeriod(port)*fxPwm_GetDuty(port)));
  TIME_US lowPeriod = fxPwm_GetPeriod(port) - highPeriod;
  
  currentPort->halfPeriod1 = highPeriod;
  currentPort->halfPeriod2 = lowPeriod;
  fxPwm_DigitalWrite(port, LOW);

  fxPwm_RESTORE_SREG();
}

//Atribui valor de período e ciclo de trabalho.
//Se o período = 0, é desligado.
void fxPwm_SetPeriodAndDuty(UINT8 port, TIME_US period, FLOAT duty){
  if(port>=fxPwm_numPorts || fxPwm_ports == NULL) return;
  
  fxPwm_PortData *currentPort = &fxPwm_ports[port];
  
  duty = (duty<0.0)?(0.0):((duty>1.0)?(1.0):(duty));
  
  //Verifica se está efetivamente fazendo uma modificação nos valores chaves.
  if(period==currentPort->period && currentPort->duty==duty){
    return;
  }
  
  //Calcula os períodos.
  TIME_US highPeriod = (TIME_US)((FLOAT)period*duty);
  TIME_US lowPeriod = period - highPeriod;
  
  TIME_US currentTime = fxPwm_MicrosCallback();
  
  //Salvar contexto.
  fxPwm_SAVE_SREG();
  cli();

  /* A mudança de período não deve influenciar o ciclo atual.
   * Para isso, a função olha para o ciclo de trabalho e para os meios períodos atuais para determinar se está com a porta positiva ou negativa.
   * halfPeriod1 e halfPeriod2 ficam trocando de valores; inicialmente halfPeriod1 representa o tempo de nível ALTO.
   * É preciso combinar os valores de nivel alto e baixo sem quebrar a sequência atual.
   */
   if(fxPwm_DigitalRead(port)==LOW){
		currentPort->halfPeriod1 = highPeriod;
		currentPort->halfPeriod2 = lowPeriod;
   }else{
		currentPort->halfPeriod2 = highPeriod;
		currentPort->halfPeriod1 = lowPeriod;
   }

  //Guardar informações úteis.
  currentPort->duty = duty;
  currentPort->period = highPeriod+lowPeriod;
  
  if(currentPort->period==0){
    //Não há período. Não agendar nada.
    currentPort->nextEvent = (TIME_US)fxPwm_NO_NEXT_EVENT;
    fxPwm_DigitalWrite(port, LOW);
  }else{
    if(highPeriod==0){
      //Nível alto nulo, nível baixo constante.
      currentPort->nextEvent = (TIME_US)fxPwm_NO_NEXT_EVENT;
      fxPwm_DigitalWrite(port, LOW);
    }else if(lowPeriod==0){
      //Nível alto constante, nível baixo nulo.
	  currentPort->nextEvent = (TIME_US)fxPwm_NO_NEXT_EVENT;
      fxPwm_DigitalWrite(port, HIGH);
	}else{
      //Há ambos período alto e baixo.
      //Verificar se a porta habilitada. Se estiver, agendar próximo evento.
      if(currentPort->enabled == TRUE && period*2 + currentTime < currentPort->nextEvent){
        currentPort->nextEvent = period*2 + currentTime;
        t1c_SetNextFireMax(currentPort->nextEvent);
      }
    }
  }
  
  

  fxPwm_RESTORE_SREG();
}

//Atribui o período usando a função fxPwm_SetPeriodAndDuty.
void fxPwm_SetPeriod(UINT8 port, TIME_US period){
  if(port>=fxPwm_numPorts || fxPwm_ports == NULL) return;
  fxPwm_SetPeriodAndDuty(port, period,fxPwm_ports[port].duty);
}

//Atribui o ciclo de trabalho usando fxPwm_SetPeriodAndDuty.
void fxPwm_SetDuty(UINT8 port, FLOAT duty){
  if(port>=fxPwm_numPorts || fxPwm_ports == NULL) return;
  fxPwm_SetPeriodAndDuty(port, fxPwm_ports[port].period,duty);
}

//Atribui a frequência (inverso do período) e o ciclo de trabalho usando fxPwm_SetPeriodAndDuty.
void fxPwm_SetFrequencyAndDuty(UINT8 port, FLOAT frequency, FLOAT duty){
  if(frequency<=0.0){
    fxPwm_SetPeriodAndDuty(port, 0, duty);
  }else{
	fxPwm_SetPeriodAndDuty(port, (TIME_US)(1000000.0/frequency), duty);
  }
}

//Atribui a frequência (inverso do período) usando fxPwm_SetPeriod.
void fxPwm_SetFrequency(UINT8 port, FLOAT frequency){
  if(frequency<=0.0){
    fxPwm_SetPeriod(port, 0);
  }else{
	fxPwm_SetPeriod(port, (TIME_US)(1000000.0/frequency));
  }
}

//Desativa (silencia) a porta.
void fxPwm_Disable(UINT8 port){
  if(port>=fxPwm_numPorts || fxPwm_ports  == NULL) return;
  
  fxPwm_PortData *currentPort = &fxPwm_ports[port];

  //Não desativar se já estiver desativada.
  if(currentPort->enabled == FALSE)
    return;
  
  fxPwm_SAVE_SREG();
  cli();

  //Inibir o próximo evento e desativar.
  currentPort->nextEvent = (TIME_US)fxPwm_NO_NEXT_EVENT;
  currentPort->enabled = FALSE;

  fxPwm_DigitalWrite(port, LOW);
  
  fxPwm_RESTORE_SREG();
}

void fxPwm_DisableAll(){
  UINT8 t;
  for(t=0;t<fxPwm_numPorts;t++){
    fxPwm_Disable(t);
  }
}

//Ativa a porta e permite soar.
void fxPwm_Enable(UINT8 port){
  if(port>=fxPwm_numPorts || fxPwm_ports  == NULL) return;
  
  fxPwm_PortData *currentPort = &fxPwm_ports[port];

  //Não ativar se já estiver ativada.
  if(currentPort->enabled != FALSE)
    return;
  
  fxPwm_SAVE_SREG();
  cli();

  //Verificar se tem algo para ser tocado.
  if(currentPort->period!=0){
    //Permitir que esse algo toque. Habilitar disparo.
    currentPort->nextEvent = fxPwm_MicrosCallback();
    //Recalcular valores do ciclo de trabalho, para garantir que inicie pelo ciclo positivo.
    fxPwm_ResetDuty(port);

    //Verificar se a biblioteca foi inicializada, e já agendar a próxima chamada.
    if(fxPwm_started!=FALSE){
      t1c_SetNextFire(fxPwm_MicrosCallback());
    }
  }else{
    //Habilitar mas não agendar coisa alguma.
    currentPort->nextEvent = (TIME_US)fxPwm_NO_NEXT_EVENT;
  }
  currentPort->enabled = TRUE;

  fxPwm_RESTORE_SREG();
}

void fxPwm_EnableAll(){
  UINT8 t;
  //Salvar contexto para impedir que o callback interrompa essa função até o fim.
  fxPwm_SAVE_SREG();
  cli();
  for(t=0;t<fxPwm_numPorts;t++){
    fxPwm_Enable(t);
  }
  fxPwm_RESTORE_SREG();
}

void fxPwm_Start(){
  fxPwm_started = TRUE;
  
  t1c_SetNextFire(fxPwm_MicrosCallback());
}

void fxPwm_Stop(){
  t1c_Stop();
  fxPwm_started = FALSE;
}

//Limpa e libera tudo.
void fxPwm_Free(){
  fxPwm_DisableAll();
  fxPwm_Stop();
  fxPwm_ClearAllPorts();

  if(fxPwm_numPorts==0 || fxPwm_ports  == NULL){
    fxPwm_numPorts = 0;
    fxPwm_ports = NULL;
    return;
  }

  free(fxPwm_ports);
  
  fxPwm_numPorts = 0;
  fxPwm_ports = NULL;
  return;
}


