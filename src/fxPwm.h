/*  -----------------------------------------------------------
 *  Andrei Alves Cardoso 16-07-2018
 *  -----------------------------------------------------------
 *  fxPwm.h
 *  Cabeçalho contendo declaração da classe da biblioteca fxPwm,
 *  bem como de todos seus métodos.
 *  -----------------------------------------------------------
 *  Você pode usar livremente esse programa para quaisquer fins,
 *  porém NÃO HÁ GARANTIA para qualquer propósito.
 *  -----------------------------------------------------------
 *  17-07-2018: primeira documentação
 *  21-07-2018: modificações maiores na estrutura da biblioteca. Melhoria de desempenho.
 *  -----------------------------------------------------------
 *  Problemas conhecidos:
 *  22-07-2018: o uso da comunicação Serial pode quebrar a biblioteca.
 *	  Chamar fxPwm.Start() antes e depois de fazer comunicação serial costuma resolver.
 */

#ifndef fxPwm_H
#define fxPwm_H

#include <fxPwmTypes.h>
#include <fxPwm_Port.h>

// ========================================================
// Vários parâmetros configuráveis.
// ========================================================

//Máximo de portas, na inicialização padrão.
#ifndef fxPwm_MaxPorts
#define fxPwm_MaxPorts 32
#endif

//Resolução mínima do timer, em nanossegundos.
//Isto é, o timer vai adotar a maior resolução que garanta um intervalo menor ou igual a este.
//Valores menores melhoram resolução do PWM e da frequência, com penalidade de desempenho.
#ifndef fxPwm_MinTimerResolution
#define fxPwm_MinTimerResolution 1000
#endif

//Fenda mínima entre mudanças de nível de um canal que o timer vai esperar antes de sair, em microssegundos.
//Valores maiores melhoram o jitter, com penalidade de desempenho.
//Valores muito grandes são ineficazes, especialmente quando rodando em altas frequências.
#ifndef fxPwm_MinTimerGap
#define fxPwm_MinTimerGap 100
#endif

//Atraso mínimo entre a saída do callback do timer e a próxima chamada, em microssegundos.
//Valores menores melhoram o jitter com penalidade severa de desempenho.
#ifndef fxPwm_MinTimerDelta
#define fxPwm_MinTimerDelta 15
#endif

//Máxima duração do callback, em microssegundos.
//Valores maiores melhoram o jitter, com penalidade de desempenho.
#ifndef fxPwm_MaxTimerDuration
#define fxPwm_MaxTimerDuration 1000
#endif

//Máximo atraso entre uma chamada do timer e outra.
//Valores menores reduzem a latência ao habilitar ou desabilitar portas, com penalidade de desempenho.
#ifndef fxPwm_MaxTimerPeriod
#define fxPwm_MaxTimerPeriod 10000
#endif

//Máximo valor a ser somado no clock quando agendando um evento.
//Não deve ser maior que 65535
#ifndef fxPwm_MaxTimerClkSum
#define fxPwm_MaxTimerClkSum 60000
#endif

// ========================================================
// Classe principal.
// ========================================================

class fxPwm_T1{
private:
  //Máximo de portas alocáveis.
  UINT8 maxPorts;
  //Ponteiro para vetor contendo ponteiro para os objetos que representam as portas.
  //Ele tem um elemento a mais que maxPorts para que este último sirva de marcação do fim da lista estática.
  fxPwm_Port **ports;
  //Ponteiro para lista de pinos alocados internamente. 0xFF = elemento vazio.
  UINT8 *allocatedPins;

  //Último valor registrado de TCNT1.
  volatile UINT16 lastClock;

  //Contagem dos ciclos de clock do TIMER1.
  volatile TIME_CLOCK clockCount;
  
  //Quantidade de nanossegundos por ciclo de clock do timer.
  static UINT32 nsPerTimerClock;

  //Variáveis contendo dados importantes de temporização.
  static UINT16 minTimerGap;
  static UINT16 minTimerDelta;
  static UINT16 maxTimerDuration;
  static UINT16 maxTimerPeriod;

  //Bits do pré-escalar do TIMER1.
  static UINT8 prescaler;

  //Limpa tudo e calcula dados de temporização.
  void Cleanup();

  //Configura o TIMER1.
  void ConfigureTimer();

  //Começa a contagem do TIMER1.
  void StartTimer();

  //Para (na verdade pausa) a contagem do TIMER1.
  void StopTimer();

  //Seta o próximo evento que acontece, mas apenas se clockCount for anterior ao mais próximo agendado.
  void SetNextFireMin(UINT32 clockCount);

  //Testa se tudo está alocado direito.
  BOOL IsAllocated();
public:

  //Classe amiga, auxiliar.
  friend class fxPwm_Port;
  
  fxPwm_T1();

  //Resolve tudo que der nas saídas.
  void Tick();

  //===============================================================
  //Métodos de inicialização e liberação.
  //===============================================================

  //Inicializa a biblioteca com fxPwm_MaxPorts portas no máximo.
  void Initialize();

  //Inicializa a biblioteca com uma capacidade determinada de portas.
  void Initialize(UINT8 maxPorts);

  //Libera recursos usados pela biblioteca.
  void Free();
  
  //Inicia operação do modulador PWM.
  void Start();
  //Para operação do modulador PWM.
  void Stop();

  //===============================================================
  //Métodos de manipulação de portas e pinos.
  //===============================================================

  //Registra uma porta a partir do ponteiro.
  void RegisterPort(fxPwm_Port *port);
  //Remove uma porta registrada, a partir do ponteiro.
  //Se a porta tiver sido alocada internamente, ela também será desalocada.
  void RemovePort(fxPwm_Port *port);

  //Registra uma porta a partir de um número de pino do Arduino.
  void RegisterPort(UINT8 pin);
  //Remove uma porta a partir de um número de pino do Arduino.
  void RemovePort(UINT8 pin);

  //Atribui o período, em microssegundos, do ciclo PWM de um pino.
  void SetPeriod(UINT8 pin, TIME_US period);
  //Atribui a frequência, em hertz, do ciclo PWM de um pino.
  void SetFrequency(UINT8 pin, FLOAT frequency);
  //Atribui o ciclo de trabalho, de 0.0 (0%) até 1.0 (100%), do ciclo PWM de um pino.
  void SetDuty(UINT8 pin, FLOAT duty);

  //Mapeia o ciclo de trabalho, para adequar os valores de entrada.
  void SetMap(UINT8 pin,FLOAT dutyValue1, FLOAT mappedValue1, FLOAT dutyValue2, FLOAT mappedValue2);

  //Ativa a saída de modulação em um pino. 
  void EnablePin(UINT8 pin);

  //Desativa a saída de modulação em um pino.
  void DisablePin(UINT8 pin);

  //Ativa a saída de modulação em todos pinos registrados.
  void EnableAll();
  
  //Desativa a saída de modulação em todos pinos registrados.
  void DisableAll();

  //===============================================================
  //Métodos de aquisição de informação.
  //===============================================================

  //Retorna o ponteiro para uma porta a partir de um número de pino.
  //Ou retorna NULL se não existir.
  //Esse objeto retornado permite manipulação direta de alguns parâmetros da porta.
  //USE COM CUIDADO!
  fxPwm_Port *GetPort(UINT8 pin);

  //Retorna a quantidade máxima de portas alocáveis.
  UINT8 GetMaxPorts();

  //Retorna a quantidade de portas registradas.
  UINT8 GetNumRegisteredPorts();

  //Retorna um ponteiro para uma porta registrada a partir de um índice, ou NULL se não existir.
  //USE COM CUIDADO!
  fxPwm_Port *GetRegisteredPort(UINT8 index);

  //Retorna o número do pino de uma porta registrada a partir de um índice, ou 0xFF se não existir.
  UINT8 GetRegisteredPortPinNumber(UINT8 index);

  //Retorna o índice de um pino, ou 0xFF se não existir.
  UINT8 GetIndex(UINT8 pin);

  //Retorna o período de oscilação de um pino, em microssegundos.
  TIME_US GetPeriod(UINT8 pin);

  //Retorna a frequência de oscilação de um pino, em hertz.
  FLOAT GetFrequency(UINT8 pin);

  //Retorna o ciclo de trabalho de um pino, com mapeamento.
  FLOAT GetDuty(UINT8 pin);

  //Retorna o próximo evento em um pino.
  //Cuidado, valor muito volátil!
  TIME_US GetNextEvent(UINT8 pin);

  //Retora a contagem de tempo, em microssegundos, do TIMER1. A precisão pode variar.
  TIME_US Micros();
};

extern fxPwm_T1 fxPwm;

#endif


