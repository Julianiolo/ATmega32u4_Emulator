#ifndef __A32U4TYPES_H__
#define __A32U4TYPES_H__

#include <stdint.h>

typedef uint8_t regind_t;
typedef uint8_t reg_t;
typedef uint16_t addrmcu_t;
typedef uint16_t pc_t;
typedef uint16_t sizemcu_t;

#ifdef _MSC_VER

#define MCU_INLINE

#else

#define MCU_INLINE

#endif

#endif