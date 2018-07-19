/*  -----------------------------------------------------------
 *  Andrei Alves Cardoso 16-07-2018
 *  -----------------------------------------------------------
 *  fxPwm.h
 *  Cabeçalho contendo declarações de funções da biblioteca
 *  fxPwm.
 *  -----------------------------------------------------------
 *  Você pode usar livremente esse programa para quaisquer fins,
 *  porém NÃO HÁ GARANTIA para qualquer propósito.
 *  -----------------------------------------------------------
 *  17-07-2018: primeira documentação
 */

/*
COMO USAR:

O roteiro básico de uso dessa biblioteca é bem simples e pode ser resumido pela seguinte receita de bolo:
1. Inicialize a biblioteca: ........................... fxPwm_Initialize
2. Configure suas portas: ............................. fxPwm_ConfigurePort
3. Ligue a biblioteca: ................................ fxPwm_Start
4. Estabeleça as condições iniciais de cada porta: .... fxPwm_SetFrequencyAndDuty e afins
5. Habilite suas portas: .............................. fxPwm_Enable
6. Manipule suas portas: .............................. fxPwm_SetFrequencyAndDuty e afins
7. Desligue suas portas: .............................. fxPwm_Disable
8. Desligue a biblioteca: ............................. fxPwm_Stop
9. Libere a biblioteca: ............................... fxPwm_Free

Na verdade, nem todas etapas são obrigatórias.
Na maioria das aplicações de vida real com microcontroladores, não vai ser necessário parar ou liberar a biblioteca.
Consulte a documentação de cada função para saber mais.

Andrei Alves Cardoso, 17/07/2018 23:39 GMT-3, enquanto ouvia "Love will keep us together" de "Captain & Tennille".
 */

#ifndef fxPwm_H

#include <fxPwmTypes.h>

//Tempo máximo que o callback do timer vai gastar, em microssegundos.
#ifndef fxPwm_MAX_TIMER_CALLBACK_DURATION
#define fxPwm_MAX_TIMER_CALLBACK_DURATION 1000
#endif

//Folga, em microssegundos, que o callback vai esperar ter antes de sair.
#ifndef fxPwm_TIMER_CALLBACK_SAFE_OVERHEAD
#define fxPwm_TIMER_CALLBACK_SAFE_OVERHEAD 50
#endif

#ifndef fxPwm_MIN_TIMER_PERIOD
#define fxPwm_MIN_TIMER_PERIOD 10000
#endif

//Valor inicial do ciclo de trabalho.
#ifndef fxPwm_START_DUTY
#define fxPwm_START_DUTY 0.5
#endif

/* Aloca recursos necessários para a operação suave da biblioteca, e atribui função de tempo do usuário.
 * Apesar de alocar os recursos, ela não configura as portas. Para isso, é preciso usar fxPwm_ConfigurePort.
 * 
 * UINT8 numPorts: quantidade de portas.
 * TIME_US (*microsFunction)(): ponteiro para função que retorna o tempo em microssegundos desde o começo do mundo. Pode ser NULL.
 * 
 * Depois que essa função for chamada, as portas podem ser acessadas pelos índices 0-numPorts-1 no resto das funções.
 */
void fxPwm_Initialize(UINT8 numPorts, TIME_US (*microsFunction)());

/* Configura uma porta para que possa funcionar como saída PWM. É preciso que fxPwm_Initialize tenha sido chamado antes.
 * 
 * UINT8 port: índice da porta a ser chamada.
 * BYTE* portPointer: ponteiro para o registrador que contém o bit que representa o estado da saída (BAIXO ou ALTO).
 *    Geralmente está no formato PORTx, onde x é uma letra.
 *    Esse parâmetro pode variar entre placas Arduino. É preciso consultar a página de manipulação de portas para descobrir qual usar.
 * BYTE* ddrPointer: ponteiro para o registrador que contém o bit que representa a direção da saída (ENTRADA ou SAIDA).
 *    Geralmente está no formato DDRx, onde x é uma letra.
 *    A letra no PORTx E DDRx costumam ser as mesmas.
 * BYTE mask: máscara que será usada para alterar o estado dos registradores de porta e direção (PORTx e DDRx).
 * 
 * Consulte a documentação do seu microcontrolador para saber qual porta vai para onde.
 * Ou use a outra sobrecarga da função. :)
 * 
 * Obs.: pode ser chamada mais de uma vez para, por exemplo, trocar o pino de uma porta.
 */
void fxPwm_ConfigurePort(UINT8 port, BYTE* portPointer, BYTE* ddrPointer, BYTE mask);

/* Configura uma porta para que possa funcionar como saída PWM. É preciso que fxPwm_Initialize tenha sido chamado antes.
 *  
 * UINT8 port: índice da porta a ser chamada.
 * UINT8 pinNumber: número do pino na sua placa Arduino. :)
 * 
 * Obs.: pode ser chamada mais de uma vez para, por exemplo, trocar o pino de uma porta.
 */
void fxPwm_ConfigurePort(UINT8 port, UINT8 pinNumber);

/* Escreve de forma digital na porta configurada. Nada demais.
 * 
 * UINT8 id: índice da porta.
 * INT16 value: valor. 0 = BAIXO. De outra forma, = ALTO.
 * 
 * Obs: essa função FORÇA a porta a entrar em modo de SAÍDA.
 */
void fxPwm_DigitalWrite(UINT8 port, INT16 value);

//Retorna o ciclo de trabalho atual de uma porta. O ciclo de trabalho vai sempre estar entre 0.0 e 1.0.
inline FLOAT fxPwm_GetDuty(UINT8 port);

//Retorna o período atual de oscilação de uma porta, em microssegundos.
inline TIME_US fxPwm_GetPeriod(UINT8 port);

//Retorna a frequência atual de oscilação da porta, em hertz.
inline FLOAT fxPwm_GetFrequency(UINT8 port);

/* Atribui o período de oscilação e o ciclo de trabalho da porta.
 * 
 * UINT8 port: índice da porta.
 * TIME_US period: período, em microssegundos.
 * FLOAT duty: ciclo de trabalho. De 0.0 a 1.0. (Isto é, de 0% a 100%)
 */
void fxPwm_SetPeriodAndDuty(UINT8 port, TIME_US period, FLOAT duty);

/* Atribui o período de oscilação da porta. O ciclo de trabalho é conservado.
 * 
 * UINT8 port: índice da porta.
 * TIME_US period: período, em microssegundos.
 */
void fxPwm_SetPeriod(UINT8 port, TIME_US period);

/* Atribui o ciclo de trabalho da porta. A frequência é conservada.
 * 
 * UINT8 port: índice da porta.
 * FLOAT duty: ciclo de trabalho. De 0.0 a 1.0. (Isto é, de 0% a 100%)
 */
void fxPwm_SetDuty(UINT8 port, FLOAT duty);


/* Atribui a frequência de oscilação e o ciclo de trabalho da porta.
 * 
 * UINT8 port: índice da porta.
 * FLOAT frequency: frequência, em hertz.
 * FLOAT duty: ciclo de trabalho. De 0.0 a 1.0. (Isto é, de 0% a 100%)
 */
void fxPwm_SetFrequencyAndDuty(UINT8 port, FLOAT frequency, FLOAT duty);

/* Atribui a frequência de oscilação da porta. O ciclo de trabalho é conservado.
 * 
 * UINT8 port: índice da porta.
 * FLOAT frequency: frequência, em hertz.
 */
void fxPwm_SetFrequency(UINT8 port, FLOAT frequency);

// Desabilita uma porta; isto é, inibe a saída de PWM dela e joga para nível BAIXO.
void fxPwm_Disable(UINT8 port);

// Desabilita todas portas.
void fxPwm_DisableAll();

// Habilita uma porta, permitindo que ocorra PWM nela.
// Se ela estiver sido previamente configurada com um período (ou frequência) e um ciclo de trabalho, 
// ela vai começar a modulá-los imediatamente.
void fxPwm_Enable(UINT8 port);

//Habilita todas portas.
void fxPwm_EnableAll();

// Inicia o funcionamento da biblioteca. É uma condição necessária para tudo funcionar.
// Se alguma porta estiver previamente configurada e habilitada, ela vai voltar a funcionar.
void fxPwm_Start();

// Desliga a biblioteca. Tudo para.
void fxPwm_Stop();

// Libera todos recursos usados pela biblioteca.
void fxPwm_Free();

// Boa noite.

#endif

