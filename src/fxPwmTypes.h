/*  -----------------------------------------------------------
 *  Andrei Alves Cardoso 16-07-2018
 *  -----------------------------------------------------------
 *  fxPwmTypes.h
 *  Cabeçalho contendo definições de tipos padrões em todos
 *  meus programas.
 *  Parte da biblioteca FxPwm.
 *  Nesse caso, as definições estão de acordo com os tamanhos
 *  de bits do Arduino.
 *  Pode ser necessário mudar.
 *  -----------------------------------------------------------
 *  Você pode usar livremente esse programa para quaisquer fins,
 *  porém NÃO HÁ GARANTIA para qualquer propósito.
 *  -----------------------------------------------------------
 *  17-07-2018: primeira documentação
 */

#ifndef FXPWMTYPES_H
#define FXPWMTYPES_H

typedef unsigned char BYTE;

typedef unsigned char UINT8;
typedef signed char INT8;

#ifndef UINT8_MAX
#define UINT8_MAX 255
#endif
#ifndef UINT8_MIN
#define UINT8_MIN 0
#endif

#ifndef INT8_MAX
#define INT8_MAX 127
#endif
#ifndef INT8_MIN
#define INT8_MIN -128
#endif

typedef unsigned int UINT16;
typedef signed int INT16;

#ifndef UINT16_MAX
#define UINT16_MAX 65535
#endif
#ifndef UINT16_MIN
#define UINT16_MIN 0
#endif

#ifndef INT16_MAX
#define INT16_MAX 32767
#endif
#ifndef INT16_MIN
#define INT16_MIN -32768
#endif

typedef unsigned long UINT32;
typedef signed long INT32;

#ifndef UINT32_MAX
#define UINT32_MAX 4294967295
#endif
#ifndef UINT32_MIN
#define UINT32_MIN 0
#endif

#ifndef INT32_MAX
#define INT32_MAX 2147483647
#endif
#ifndef INT32_MIN
#define INT32_MIN -2147483648
#endif

typedef float FLOAT;


typedef UINT32 TIME_US;

#ifndef TIME_US_MAX
#define TIME_US_MAX UINT32_MAX
#endif
#ifndef TIME_US_MIN
#define TIME_US_MIN UINT32_MIN
#endif

typedef bool BOOL;

#ifndef TRUE
#define TRUE true
#endif

#ifndef FALSE
#define FALSE false
#endif

#endif
