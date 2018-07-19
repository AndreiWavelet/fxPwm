/*  -----------------------------------------------------------
 *  Andrei Alves Cardoso 16-07-2018
 *  -----------------------------------------------------------
 *  t1custom.h
 *  Cabeçalho contendo declarações de funções úteis para
 *  manipulação do timer1 em microcontroladores presentes
 *  em placas Arduino.
 *  Parte da biblioteca FxPwm.
 *  -----------------------------------------------------------
 *  Você pode usar livremente esse programa para quaisquer fins,
 *  porém NÃO HÁ GARANTIA para qualquer propósito.
 *  -----------------------------------------------------------
 *  17-07-2018: primeira documentação
 */

#ifndef T1C_H
#define T1C_H

//Bibliotecas IO para essa biblioteca.
#include <avr/io.h>
#include <avr/interrupt.h>

//Tipos padrões
#include <fxPwmTypes.h>

//Resolução do timer em uso. (16 bits)
#ifndef t1c_Resolution
#define t1c_Resolution 65536
#endif

//Afinção do overhead da função t1c_SetPeriodOverheadTune, em ciclos de clock..
#ifndef t1c_SetPeriodOverheadTune
#define t1c_SetPeriodOverheadTune -20
#endif

#ifndef t1c_PeriodOverheadTune
#define t1c_PeriodOverheadTune -93
#endif

//Afinação do overhead da função t1c_SetNextFireOverheadTune, em microssegundos.
#ifndef t1c_SetNextFireOverheadTune
#define t1c_SetNextFireOverheadTune -30000
#endif

//Ciclos por microssegundo.
#define t1c_CyclesPerUs ((F_CPU)/1000000)

//Máximo período possível para o timer, em microssegundos.
#define t1c_MaxPeriod (t1c_Resolution<<10)/t1c_CyclesPerUs;

//Função padrão de tempo.
//Retorna a quantidade de microssegundos passada desde o início do programa.
//TIME_US tem o mesmo significado que UIN64.
//Depende da função micros().
TIME_US t1c_Micros();

/*Configura os parâmetros do temporizador, e atribui o callback.
 * void *callback: função que será chamada quando o temporizador espirar.
 * TIME_US *microsFunction: função que retorna a quantidade de microssegundos passada desde o começo do mundo (do microcontrolador).
 */
void t1c_Configure(void (*callback)(), TIME_US (*microsFunction)());

//Inicia a contagem de tempo.
void t1c_Start();

//Para a contagem de tempo.
void t1c_Stop();

//Reseta a contagem de tempo.
void t1c_Reset();

/* Agenda o atraso até próximo disparo do callback.
 * TIME_US microseconds: atraso, em microssegundos
 * BOOL periodic: se TRUE, o timer ficará sendo disparado de forma periódica.
 * Se FALSE, o timer será disparado apenas uma vez após o tempo determinado; e deverá ser atribuído novamente se quiser ser chamado novamente.
 * 
 * Obs.: o método tem um overhead que provocará um erro no período.
 * Considere regular t1c_DelayOverheadTune e tlc_FireOverheadTune.
 */
void t1c_SetPeriod(TIME_US microseconds, BOOL periodic);

/* Agenda o momento do próximo disparo do timer.
 * TIME_US timeMicros: momento que o timer vai disparar. Relativo ao valor retornado pelo callback t1c_microsFunction().
 * 
 * Obs.: o método tem um overhead que provocará um erro no período.
 * Considere regular tlc_FireOverheadTune.
 */
void t1c_SetNextFire(TIME_US timeMicros);

/* Agenda o momento máximo do próximo disparo.
 * TIME_US timeMicros: momento que o timer vai disparar, no máximo. Relativo ao valor retornado pelo callback t1c_microsFunction().
 * 
 * Obs.: o método tem um overhead que provocará um erro no período.
 * Só funciona quando em modo não-periódico. Se estiver em modo periódico, vai deixar de ser periódico.
 */
void t1c_SetNextFireMax(TIME_US timeMicros);




#endif

